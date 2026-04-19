#ifndef SERIAL_H
#define SERIAL_H

#include "stm32f303xc.h"
#include <stdint.h>

#define SERIAL_START_BYTE   0x02
#define SERIAL_STOP_BYTE    0x03
#define SERIAL_MAX_PAYLOAD  32
#define SERIAL_BUFFER_SIZE  64
#define SERIAL_READY_SLOTS  2

typedef struct
{
    uint16_t sensorValue;
    uint8_t status;
    uint8_t reserved;
} TestMessage;

typedef void (*SerialRxCallback)(uint8_t msgType, uint8_t *payload, uint16_t length);

/* debug values so you can watch them in expressions */
extern volatile uint8_t dbg_irqRxCount;
extern volatile uint8_t dbg_lastByte;
extern volatile uint8_t dbg_rxState;
extern volatile uint16_t dbg_expectedLength;
extern volatile uint16_t dbg_payloadIndex;
extern volatile uint8_t dbg_currentMsgType;
extern volatile uint8_t dbg_checksumRunning;
extern volatile uint8_t dbg_checksumReceived;
extern volatile uint8_t dbg_packetReady;
extern volatile uint8_t dbg_packetValid;
extern volatile uint8_t dbg_packetStored;
extern volatile uint8_t dbg_rxOverflow;
extern volatile uint8_t dbg_txOverflow;

void Serial_Init(void);
void Serial_SetRxCallback(SerialRxCallback callback);

/* UART4 packet path */
void Serial_SendChar(uint8_t ch);
void Serial_SendMsg(uint8_t msgType, const void *data, uint16_t size);
void Serial_Task(void);

/* USART1 debug path to Mac screen */
void Serial_SendString(const char *str);

#endif
