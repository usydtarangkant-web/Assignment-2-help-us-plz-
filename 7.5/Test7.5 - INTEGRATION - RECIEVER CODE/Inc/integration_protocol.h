#ifndef INTEGRATION_PROTOCOL_H
#define INTEGRATION_PROTOCOL_H

#include <stdint.h>

#define MSG_TYPE_COMPASS_DATA   1U

#define DISPLAY_MODE_SERVO_ONLY   0U
#define DISPLAY_MODE_LED_HEADING  1U

typedef struct
{
    int16_t  mag_x;
    int16_t  mag_y;
    int16_t  mag_z;
    uint16_t heading_deg;
    uint32_t timestamp_ms;
    uint8_t  display_mode;
    uint8_t  reserved[3];
} IntegrationMessage_t;

#endif
