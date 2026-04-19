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

typedef struct
{
    uint8_t inUse;
    uint8_t msgType;
    uint16_t length;
    uint8_t payload[SERIAL_MAX_PAYLOAD];
} SerialReadyPacket;

static SerialRxCallback serialRxCallback = 0;

/* debug watch values */
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
volatile uint8_t dbg_packetStored = 0;
volatile uint8_t dbg_rxOverflow = 0;
volatile uint8_t dbg_txOverflow = 0;

/* working receive state */
static volatile uint16_t rxExpectedLength = 0;
static volatile uint16_t rxPayloadIndex = 0;
static volatile uint8_t rxCurrentMsgType = 0;
static volatile uint8_t rxChecksumRunning = 0;
static volatile uint8_t rxChecksumReceived = 0;
static volatile uint8_t rxWorkingPayload[SERIAL_MAX_PAYLOAD];

/* ready packets waiting for main code */
static volatile SerialReadyPacket rxReady[SERIAL_READY_SLOTS];

/* tx ring buffer for UART4 interrupt transmit */
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

static void Serial_ClearReadySlots(void)
{
    uint8_t slot;
    uint16_t i;

    for (slot = 0; slot < SERIAL_READY_SLOTS; slot++)
    {
        rxReady[slot].inUse = 0;
        rxReady[slot].msgType = 0;
        rxReady[slot].length = 0;

        for (i = 0; i < SERIAL_MAX_PAYLOAD; i++)
        {
            rxReady[slot].payload[i] = 0;
        }
    }
}

static uint8_t Serial_StorePacket(void)
{
    uint8_t slot;
    uint16_t i;

    for (slot = 0; slot < SERIAL_READY_SLOTS; slot++)
    {
        if (rxReady[slot].inUse == 0)
        {
            rxReady[slot].inUse = 1;
            rxReady[slot].msgType = rxCurrentMsgType;
            rxReady[slot].length = rxExpectedLength;

            for (i = 0; i < rxExpectedLength; i++)
            {
                rxReady[slot].payload[i] = rxWorkingPayload[i];
            }

            dbg_packetStored = 1;
            dbg_packetReady = 1;
            dbg_packetValid = 1;
            return 1;
        }
    }

    dbg_rxOverflow = 1;
    dbg_packetReady = 1;
    dbg_packetValid = 0;
    return 0;
}

static void Serial_DebugSendChar(uint8_t ch)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0)
    {
    }

    USART1->TDR = ch;
}

static void Serial_HandleUart4Irq(void)
{
    uint8_t byteIn;

    /* transmit side */
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

    /* overrun means bytes came in faster than we handled them */
    if (UART4->ISR & USART_ISR_ORE)
    {
        dbg_rxOverflow = 1;
        UART4->ICR = USART_ICR_ORECF;
        dbg_packetReady = 1;
        dbg_packetValid = 0;
        Serial_ResetRxState();
        return;
    }

    /* receive side */
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
    /* clocks */
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* UART4 on PC10 TX and PC11 RX */
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

    /* AF5 for UART4 */
    GPIOC->AFR[1] &= ~(0xFU << 8);
    GPIOC->AFR[1] &= ~(0xFU << 12);
    GPIOC->AFR[1] |=  (5U << 8);
    GPIOC->AFR[1] |=  (5U << 12);

    /* USART1 debug on PC4 TX and PC5 RX */
    GPIOC->MODER &= ~(3U << (4 * 2));
    GPIOC->MODER &= ~(3U << (5 * 2));
    GPIOC->MODER |=  (2U << (4 * 2));
    GPIOC->MODER |=  (2U << (5 * 2));

    GPIOC->OTYPER &= ~(1U << 4);
    GPIOC->OTYPER &= ~(1U << 5);

    GPIOC->OSPEEDR |= (3U << (4 * 2));
    GPIOC->OSPEEDR |= (3U << (5 * 2));

    GPIOC->PUPDR &= ~(3U << (4 * 2));
    GPIOC->PUPDR &= ~(3U << (5 * 2));

    /* AF7 for USART1 */
    GPIOC->AFR[0] &= ~(0xFU << (4 * 4));
    GPIOC->AFR[0] &= ~(0xFU << (5 * 4));
    GPIOC->AFR[0] |=  (7U << (4 * 4));
    GPIOC->AFR[0] |=  (7U << (5 * 4));

    /* 115200 baud using same simple setup you were already using */
    UART4->BRR = 69U;
    USART1->BRR = 69U;

    /* enable UART4 with RX interrupt */
    UART4->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    /* enable USART1 for debug strings */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    dbg_irqRxCount = 0;
    dbg_lastByte = 0;
    dbg_packetReady = 0;
    dbg_packetValid = 0;
    dbg_packetStored = 0;
    dbg_rxOverflow = 0;
    dbg_txOverflow = 0;

    txHead = 0;
    txTail = 0;

    Serial_ResetRxState();
    Serial_ClearReadySlots();

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

    if (nextHead == txTail)
    {
        dbg_txOverflow = 1;
        return;
    }

    txBuffer[txHead] = ch;
    txHead = nextHead;

    UART4->CR1 |= USART_CR1_TXEIE;
}

void Serial_SendString(const char *str)
{
    while (*str != '\0')
    {
        Serial_DebugSendChar((uint8_t)*str);
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
    uint8_t slot;
    uint8_t anyInUse = 0;

    for (slot = 0; slot < SERIAL_READY_SLOTS; slot++)
    {
        if (rxReady[slot].inUse)
        {
            anyInUse = 1;

            if (serialRxCallback != 0)
            {
                serialRxCallback(rxReady[slot].msgType,
                                 (uint8_t *)rxReady[slot].payload,
                                 rxReady[slot].length);
            }

            rxReady[slot].inUse = 0;
        }
    }

    if (!anyInUse)
    {
        dbg_packetReady = 0;
        dbg_packetValid = 0;
        dbg_packetStored = 0;
    }
}

void UART4_IRQHandler(void)
{
    Serial_HandleUart4Irq();
}

void UART4_EXTI34_IRQHandler(void)
{
    Serial_HandleUart4Irq();
}
