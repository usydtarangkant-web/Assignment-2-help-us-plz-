#include "serial.h"

enum
{
    RX_WAIT_START = 0,
    RX_LEN_LOW,
    RX_LEN_HIGH,
    RX_MSG_TYPE,
    RX_PAYLOAD,
    RX_CHECKSUM,
    RX_STOP
};

static SerialRxCallback serialRxCallback = 0;

volatile uint8_t dbg_irqRxCount = 0;
volatile uint8_t dbg_lastByte = 0;
volatile uint8_t dbg_rxState = RX_WAIT_START;
volatile uint16_t dbg_expectedLength = 0;
volatile uint16_t dbg_payloadIndex = 0;
volatile uint8_t dbg_currentMsgType = 0;
volatile uint8_t dbg_checksumRunning = 0;
volatile uint8_t dbg_checksumReceived = 0;
volatile uint8_t dbg_packetReady = 0;
volatile uint8_t dbg_packetValid = 0;
volatile uint8_t dbg_rxOverflow = 0;
volatile uint8_t dbg_txOverflow = 0;

static volatile uint16_t rxExpectedLength = 0;
static volatile uint16_t rxPayloadIndex = 0;
static volatile uint8_t rxCurrentMsgType = 0;
static volatile uint8_t rxChecksumRunning = 0;
static volatile uint8_t rxChecksumReceived = 0;
static volatile uint8_t rxWorkingPayload[SERIAL_MAX_PAYLOAD];

static volatile uint8_t packetReady = 0;
static volatile uint8_t packetValid = 0;
static volatile uint8_t readyMsgType = 0;
static volatile uint16_t readyLength = 0;
static volatile uint8_t readyPayload[SERIAL_MAX_PAYLOAD];

static volatile uint8_t txBuffer[SERIAL_BUFFER_SIZE];
static volatile uint16_t txHead = 0;
static volatile uint16_t txTail = 0;

static void Serial_ResetRxState(void)
{
    uint16_t i;

    rxExpectedLength = 0;
    rxPayloadIndex = 0;
    rxCurrentMsgType = 0;
    rxChecksumRunning = 0;
    rxChecksumReceived = 0;

    dbg_rxState = RX_WAIT_START;
    dbg_expectedLength = 0;
    dbg_payloadIndex = 0;
    dbg_currentMsgType = 0;
    dbg_checksumRunning = 0;
    dbg_checksumReceived = 0;

    for (i = 0; i < SERIAL_MAX_PAYLOAD; i++)
    {
        rxWorkingPayload[i] = 0;
    }
}

static void Serial_StorePacket(void)
{
    uint16_t i;

    readyMsgType = rxCurrentMsgType;
    readyLength = rxExpectedLength;

    for (i = 0; i < rxExpectedLength; i++)
    {
        readyPayload[i] = rxWorkingPayload[i];
    }

    packetValid = 1;
    packetReady = 1;
    dbg_packetValid = 1;
    dbg_packetReady = 1;
}

static void Serial_HandleUart4Irq(void)
{
    uint8_t byteIn;

    if (UART4->ISR & USART_ISR_TXE)
    {
        if (txTail != txHead)
        {
            UART4->TDR = txBuffer[txTail];
            txTail++;
            if (txTail >= SERIAL_BUFFER_SIZE)
            {
                txTail = 0;
            }
        }
        else
        {
            UART4->CR1 &= ~USART_CR1_TXEIE;
        }
    }

    if (UART4->ISR & USART_ISR_ORE)
    {
        dbg_rxOverflow = 1;
        UART4->ICR = USART_ICR_ORECF;
        packetReady = 1;
        packetValid = 0;
        dbg_packetReady = 1;
        dbg_packetValid = 0;
        Serial_ResetRxState();
        return;
    }

    if (UART4->ISR & USART_ISR_RXNE)
    {
        dbg_irqRxCount++;

        byteIn = (uint8_t)UART4->RDR;
        dbg_lastByte = byteIn;

        switch (dbg_rxState)
        {
            case RX_WAIT_START:
                if (byteIn == SERIAL_START_BYTE)
                {
                    Serial_ResetRxState();
                    dbg_rxState = RX_LEN_LOW;
                }
                break;

            case RX_LEN_LOW:
                rxExpectedLength = byteIn;
                rxChecksumRunning ^= byteIn;
                dbg_expectedLength = rxExpectedLength;
                dbg_checksumRunning = rxChecksumRunning;
                dbg_rxState = RX_LEN_HIGH;
                break;

            case RX_LEN_HIGH:
                rxExpectedLength |= ((uint16_t)byteIn << 8);
                rxChecksumRunning ^= byteIn;
                dbg_expectedLength = rxExpectedLength;
                dbg_checksumRunning = rxChecksumRunning;
                if (rxExpectedLength > SERIAL_MAX_PAYLOAD)
                {
                    dbg_rxOverflow = 1;
                    packetReady = 1;
                    packetValid = 0;
                    dbg_packetReady = 1;
                    dbg_packetValid = 0;
                    dbg_rxState = RX_WAIT_START;
                }
                else
                {
                    dbg_rxState = RX_MSG_TYPE;
                }
                break;

            case RX_MSG_TYPE:
                rxCurrentMsgType = byteIn;
                rxChecksumRunning ^= byteIn;
                dbg_currentMsgType = rxCurrentMsgType;
                dbg_checksumRunning = rxChecksumRunning;
                if (rxExpectedLength == 0)
                {
                    dbg_rxState = RX_CHECKSUM;
                }
                else
                {
                    dbg_rxState = RX_PAYLOAD;
                }
                break;

            case RX_PAYLOAD:
                if (rxPayloadIndex < SERIAL_MAX_PAYLOAD)
                {
                    rxWorkingPayload[rxPayloadIndex] = byteIn;
                    rxPayloadIndex++;
                    rxChecksumRunning ^= byteIn;
                    dbg_payloadIndex = rxPayloadIndex;
                    dbg_checksumRunning = rxChecksumRunning;
                    if (rxPayloadIndex >= rxExpectedLength)
                    {
                        dbg_rxState = RX_CHECKSUM;
                    }
                }
                else
                {
                    dbg_rxOverflow = 1;
                    packetReady = 1;
                    packetValid = 0;
                    dbg_packetReady = 1;
                    dbg_packetValid = 0;
                    dbg_rxState = RX_WAIT_START;
                }
                break;

            case RX_CHECKSUM:
                rxChecksumReceived = byteIn;
                dbg_checksumReceived = rxChecksumReceived;
                dbg_rxState = RX_STOP;
                break;

            case RX_STOP:
                if ((byteIn == SERIAL_STOP_BYTE) && (rxChecksumRunning == rxChecksumReceived))
                {
                    Serial_StorePacket();
                }
                else
                {
                    packetReady = 1;
                    packetValid = 0;
                    dbg_packetReady = 1;
                    dbg_packetValid = 0;
                    dbg_rxOverflow = 1;
                }
                dbg_rxState = RX_WAIT_START;
                break;

            default:
                dbg_rxState = RX_WAIT_START;
                break;
        }
    }
}

void Serial_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

    GPIOC->MODER &= ~(3U << (10 * 2));
    GPIOC->MODER &= ~(3U << (11 * 2));
    GPIOC->MODER |=  (2U << (10 * 2));
    GPIOC->MODER |=  (2U << (11 * 2));

    GPIOC->OTYPER &= ~(1U << 10);
    GPIOC->OTYPER &= ~(1U << 11);

    GPIOC->OSPEEDR |= (3U << (10 * 2));
    GPIOC->OSPEEDR |= (3U << (11 * 2));

    GPIOC->PUPDR &= ~(3U << (10 * 2));
    GPIOC->PUPDR &= ~(3U << (11 * 2));

    /* AF5 for UART4 on PC10 and PC11 */
    GPIOC->AFR[1] &= ~(0xFU << 8);
    GPIOC->AFR[1] &= ~(0xFU << 12);
    GPIOC->AFR[1] |=  (5U << 8);
    GPIOC->AFR[1] |=  (5U << 12);

    UART4->BRR = 69U;

    UART4->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    dbg_irqRxCount = 0;
    dbg_lastByte = 0;
    dbg_packetReady = 0;
    dbg_packetValid = 0;
    dbg_rxOverflow = 0;
    dbg_txOverflow = 0;

    packetReady = 0;
    packetValid = 0;
    txHead = 0;
    txTail = 0;

    Serial_ResetRxState();

    NVIC_EnableIRQ(UART4_IRQn);
}

void Serial_SetRxCallback(SerialRxCallback callback)
{
    serialRxCallback = callback;
}

void Serial_SendChar(uint8_t ch)
{
    uint16_t nextHead;

    nextHead = txHead + 1U;
    if (nextHead >= SERIAL_BUFFER_SIZE)
    {
        nextHead = 0U;
    }

    while (nextHead == txTail)
    {
        dbg_txOverflow = 1;
    }

    txBuffer[txHead] = ch;
    txHead = nextHead;

    UART4->CR1 |= USART_CR1_TXEIE;
}

void Serial_SendString(const char *str)
{
    while (*str != '\0')
    {
        Serial_SendChar((uint8_t)*str);
        str++;
    }
}

void Serial_SendMsg(uint8_t msgType, const void *data, uint16_t size)
{
    const uint8_t *payload = (const uint8_t *)data;
    uint8_t checksum = 0;
    uint16_t i;

    if (size > SERIAL_MAX_PAYLOAD)
    {
        return;
    }

    Serial_SendChar(SERIAL_START_BYTE);
    Serial_SendChar((uint8_t)(size & 0xFF));
    checksum ^= (uint8_t)(size & 0xFF);
    Serial_SendChar((uint8_t)((size >> 8) & 0xFF));
    checksum ^= (uint8_t)((size >> 8) & 0xFF);
    Serial_SendChar(msgType);
    checksum ^= msgType;

    for (i = 0; i < size; i++)
    {
        Serial_SendChar(payload[i]);
        checksum ^= payload[i];
    }

    Serial_SendChar(checksum);
    Serial_SendChar(SERIAL_STOP_BYTE);
}

void Serial_Task(void)
{
    if (packetReady == 0)
    {
        return;
    }

    if (packetValid && (serialRxCallback != 0))
    {
        serialRxCallback(readyMsgType, (uint8_t *)readyPayload, readyLength);
    }

    packetReady = 0;
    packetValid = 0;
    dbg_packetReady = 0;
    dbg_packetValid = 0;
}

void UART4_IRQHandler(void)
{
    Serial_HandleUart4Irq();
}

void UART4_EXTI34_IRQHandler(void)
{
    Serial_HandleUart4Irq();
}
