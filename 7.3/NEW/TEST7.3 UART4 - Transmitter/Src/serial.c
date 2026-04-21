#include "serial.h"

/* RX packet states */
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

/* One completed packet waiting for main code */
typedef struct
{
    uint8_t inUse;
    uint8_t msgType;
    uint16_t length;
    uint8_t payload[SERIAL_MAX_PAYLOAD];
} SerialReadyPacket;

/* Saved callback */
static SerialRxCallback serialRxCallback = 0;

/* RX working values */
static volatile uint16_t rxExpectedLength = 0;
static volatile uint16_t rxPayloadIndex = 0;
static volatile uint8_t rxCurrentMsgType = 0;
static volatile uint8_t rxChecksumRunning = 0;
static volatile uint8_t rxChecksumReceived = 0;
static volatile uint8_t rxWorkingPayload[SERIAL_MAX_PAYLOAD];

/* Completed packet slots */
static volatile SerialReadyPacket rxReady[SERIAL_READY_SLOTS];

/* UART4 TX ring buffer */
static volatile uint8_t txBuffer[SERIAL_TX_BUFFER_SIZE];
static volatile uint16_t txHead = 0;
static volatile uint16_t txTail = 0;

/* Debug values */
volatile uint8_t dbg_lastByte = 0;
volatile uint8_t dbg_rxState = RX_WAIT_START;
volatile uint16_t dbg_expectedLength = 0;
volatile uint16_t dbg_payloadIndex = 0;
volatile uint8_t dbg_currentMsgType = 0;
volatile uint8_t dbg_checksumRunning = 0;
volatile uint8_t dbg_checksumReceived = 0;
volatile uint8_t dbg_packetStored = 0;
volatile uint8_t dbg_packetValid = 0;
volatile uint8_t dbg_rxOverflow = 0;
volatile uint8_t dbg_txOverflow = 0;
volatile uint16_t dbg_txHead = 0;
volatile uint16_t dbg_txTail = 0;

/* Reset receive state so a new packet can start cleanly */
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

/* Clear ready packet slots */
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

/* Store a full valid packet in a free slot */
static void Serial_StorePacket(void)
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
            dbg_packetValid = 1;

            /* BREAKPOINT: full valid packet stored */
            return;
        }
    }

    /* No free slot means overflow */
    dbg_rxOverflow = 1;
    dbg_packetValid = 0;
}

/* Send one readable character to PC debug UART
   If your current screen output already works using a different UART,
   keep that part of your old code here instead. */
static void Serial_DebugSendChar(uint8_t ch)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0)
    {
    }

    USART1->TDR = ch;
}

void Serial_Init(void)
{
    /* If your current screen output already works,
       keep your current pin/UART setup here. */

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* UART4 packet link
       PC10 = TX
       PC11 = RX */
    GPIOC->MODER &= ~(3U << (10 * 2));
    GPIOC->MODER &= ~(3U << (11 * 2));
    GPIOC->MODER |=  (2U << (10 * 2));
    GPIOC->MODER |=  (2U << (11 * 2));

    GPIOC->AFR[1] &= ~(0xFU << 8);
    GPIOC->AFR[1] &= ~(0xFU << 12);
    GPIOC->AFR[1] |=  (5U << 8);
    GPIOC->AFR[1] |=  (5U << 12);

    /* USART1 debug text
       PC4 = TX
       PC5 = RX */
    GPIOC->MODER &= ~(3U << (4 * 2));
    GPIOC->MODER &= ~(3U << (5 * 2));
    GPIOC->MODER |=  (2U << (4 * 2));
    GPIOC->MODER |=  (2U << (5 * 2));

    GPIOC->AFR[0] &= ~(0xFU << (4 * 4));
    GPIOC->AFR[0] &= ~(0xFU << (5 * 4));
    GPIOC->AFR[0] |=  (7U << (4 * 4));
    GPIOC->AFR[0] |=  (7U << (5 * 4));

    /* 115200 in this setup */
    UART4->BRR = 69U;
    USART1->BRR = 69U;

    /* UART4 uses RX interrupt and TX interrupt when needed */
    UART4->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    /* USART1 is just readable debug text */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    Serial_ResetRxState();
    Serial_ClearReadySlots();

    txHead = 0;
    txTail = 0;
    dbg_txHead = 0;
    dbg_txTail = 0;
    dbg_rxOverflow = 0;
    dbg_txOverflow = 0;
    dbg_packetStored = 0;
    dbg_packetValid = 0;

    NVIC_EnableIRQ(UART4_IRQn);
}

void Serial_SetRxCallback(SerialRxCallback callback)
{
    serialRxCallback = callback;
}

/* Queue one byte into UART4 TX buffer */
void Serial_SendChar(uint8_t ch)
{
    uint16_t nextHead;

    nextHead = txHead + 1U;
    if (nextHead >= SERIAL_TX_BUFFER_SIZE)
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

    dbg_txHead = txHead;
    dbg_txTail = txTail;

    /* Start TX interrupt */
    UART4->CR1 |= USART_CR1_TXEIE;

    /* BREAKPOINT: byte queued for TX */
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

    /* Packet:
       start | size low | size high | type | payload | checksum | stop */

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

    /* BREAKPOINT: full packet queued */
}

void Serial_SendString(const char *str)
{
    while (*str != '\0')
    {
        Serial_DebugSendChar((uint8_t)*str);
        str++;
    }
}

/* Call from main loop so callback happens outside interrupt */
void Serial_Task(void)
{
    uint8_t slot;

    for (slot = 0; slot < SERIAL_READY_SLOTS; slot++)
    {
        if (rxReady[slot].inUse != 0)
        {
            if (serialRxCallback != 0)
            {
                serialRxCallback(rxReady[slot].msgType,
                                 (uint8_t *)rxReady[slot].payload,
                                 rxReady[slot].length);
            }

            rxReady[slot].inUse = 0;
        }
    }
}

void UART4_IRQHandler(void)
{
    uint8_t byteIn;

    /* TX interrupt */
    if (UART4->ISR & USART_ISR_TXE)
    {
        if (txTail != txHead)
        {
            UART4->TDR = txBuffer[txTail];
            txTail++;

            if (txTail >= SERIAL_TX_BUFFER_SIZE)
            {
                txTail = 0U;
            }

            dbg_txHead = txHead;
            dbg_txTail = txTail;

            /* BREAKPOINT: interrupt sent next byte */
        }
        else
        {
            UART4->CR1 &= ~USART_CR1_TXEIE;
        }
    }

    /* RX overrun */
    if (UART4->ISR & USART_ISR_ORE)
    {
        dbg_rxOverflow = 1;
        UART4->ICR = USART_ICR_ORECF;
        Serial_ResetRxState();
        return;
    }

    /* RX interrupt */
    if (UART4->ISR & USART_ISR_RXNE)
    {
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
                    Serial_ResetRxState();
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
                    Serial_ResetRxState();
                }
                break;

            case RX_CHECKSUM:
                rxChecksumReceived = byteIn;
                dbg_checksumReceived = rxChecksumReceived;
                dbg_rxState = RX_STOP;
                break;

            case RX_STOP:
                if ((byteIn == SERIAL_STOP_BYTE) &&
                    (rxChecksumRunning == rxChecksumReceived))
                {
                    Serial_StorePacket();
                }

                Serial_ResetRxState();
                break;

            default:
                Serial_ResetRxState();
                break;
        }
    }
}

/* Some projects use this interrupt name instead */
void UART4_EXTI34_IRQHandler(void)
{
    UART4_IRQHandler();
}
