#ifndef SERIAL_H
#define SERIAL_H

#include "stm32f303xc.h"
#include <stdint.h>

#define SERIAL_START_BYTE   0x02
#define SERIAL_STOP_BYTE    0x03
#define SERIAL_MAX_PAYLOAD  32

typedef struct
{
    uint16_t sensorValue;
    uint8_t status;
    uint8_t reserved;
} TestMessage;

typedef void (*SerialRxCallback)(uint8_t msgType, uint8_t *payload, uint16_t length);

void Serial_Init(void);
void Serial_SetRxCallback(SerialRxCallback callback);
void Serial_SendChar(uint8_t ch);
void Serial_SendString(const char *str);
void Serial_SendMsg(uint8_t msgType, const void *data, uint16_t size);
void Serial_Task(void);

#endif
