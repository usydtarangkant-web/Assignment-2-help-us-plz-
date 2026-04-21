#ifndef SERIAL_H
#define SERIAL_H

#include "stm32f303xc.h"
#include <stdint.h>
#include <stdbool.h>

/* Packet markers */
#define SERIAL_START_BYTE   0x02
#define SERIAL_STOP_BYTE    0x03

/* Payload limit for one packet */
#define SERIAL_MAX_PAYLOAD  32

/* TX ring buffer size */
#define SERIAL_TX_BUFFER_SIZE 64

/* Ready packet slots for simple double buffering */
#define SERIAL_READY_SLOTS  2

/* Simple message for demo
   Keep this the same on both boards */
typedef struct
{
    uint16_t sensorValue;
    uint8_t status;
    uint8_t reserved;
} TestMessage;

/* Callback for a full valid packet */
typedef void (*SerialRxCallback)(uint8_t msgType, uint8_t *payload, uint16_t length);

/* Debug values for Expressions */
extern volatile uint8_t dbg_lastByte;
extern volatile uint8_t dbg_rxState;
extern volatile uint16_t dbg_expectedLength;
extern volatile uint16_t dbg_payloadIndex;
extern volatile uint8_t dbg_currentMsgType;
extern volatile uint8_t dbg_checksumRunning;
extern volatile uint8_t dbg_checksumReceived;
extern volatile uint8_t dbg_packetStored;
extern volatile uint8_t dbg_packetValid;
extern volatile uint8_t dbg_rxOverflow;
extern volatile uint8_t dbg_txOverflow;
extern volatile uint16_t dbg_txHead;
extern volatile uint16_t dbg_txTail;

/* Set up packet UART and debug UART */
void Serial_Init(void);

/* Set the callback used when a full packet arrives */
void Serial_SetRxCallback(SerialRxCallback callback);

/* Queue one packet byte for UART TX interrupt */
void Serial_SendChar(uint8_t ch);

/* Build and send one full packet */
void Serial_SendMsg(uint8_t msgType, const void *data, uint16_t size);

/* Send readable text to the PC */
void Serial_SendString(const char *str);

/* Main loop calls this to hand completed packets to callback */
void Serial_Task(void);

#endif
