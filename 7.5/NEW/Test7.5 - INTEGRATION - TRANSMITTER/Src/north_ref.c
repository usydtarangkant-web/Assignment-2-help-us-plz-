#include "north_ref.h"

#define NORTH_REF_DOUBLE_PRESS_MS  2000U

static uint32_t s_last_press_ms = 0U;
static uint8_t  s_waiting_second_press = 0U;
static uint16_t s_north_offset_deg = 0U;

void NorthRef_Init(void)
{
    s_last_press_ms = 0U;
    s_waiting_second_press = 0U;
    s_north_offset_deg = 0U;
}

uint8_t NorthRef_RegisterButtonPress(uint32_t now_ms)
{
    if ((s_waiting_second_press != 0U) &&
        ((now_ms - s_last_press_ms) <= NORTH_REF_DOUBLE_PRESS_MS))
    {
        s_waiting_second_press = 0U;
        return 1U;
    }

    s_waiting_second_press = 1U;
    s_last_press_ms = now_ms;
    return 0U;
}

void NorthRef_SetCurrentAsNorth(uint16_t current_heading_deg)
{
    s_north_offset_deg = (uint16_t)(current_heading_deg % 360U);
}

uint16_t NorthRef_Apply(uint16_t raw_heading_deg)
{
    uint16_t raw;
    uint16_t offset;
    uint16_t adjusted;

    raw = (uint16_t)(raw_heading_deg % 360U);
    offset = (uint16_t)(s_north_offset_deg % 360U);

    if (raw >= offset)
    {
        adjusted = raw - offset;
    }
    else
    {
        adjusted = (uint16_t)(360U + raw - offset);
    }

    return adjusted;
}
