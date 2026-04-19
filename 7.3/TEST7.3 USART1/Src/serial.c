#include "serial.h"

static SerialRxCallback serialRxCallback = 0;

void Serial_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PC4 = USART1_TX, PC5 = USART1_RX */
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

    /* AF7 for USART1 on PC4 and PC5 */
    GPIOC->AFR[0] &= ~(0xFU << (4 * 4));
    GPIOC->AFR[0] &= ~(0xFU << (5 * 4));
    GPIOC->AFR[0] |=  (7U << (4 * 4));
    GPIOC->AFR[0] |=  (7U << (5 * 4));

    /* 115200 baud when USART clock is 8 MHz */
    USART1->BRR = 69U;

    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void Serial_SetRxCallback(SerialRxCallback callback)
{
    serialRxCallback = callback;
    (void)serialRxCallback;
}

void Serial_SendChar(uint8_t ch)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0)
    {
    }

    USART1->TDR = ch;
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
    /* not needed yet for the one-board USART1 debug version */
}
