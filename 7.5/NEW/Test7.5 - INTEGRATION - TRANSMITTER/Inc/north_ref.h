#ifndef NORTH_REF_H
#define NORTH_REF_H

#include <stdint.h>

void NorthRef_Init(void);
uint8_t NorthRef_RegisterButtonPress(uint32_t now_ms);
void NorthRef_SetCurrentAsNorth(uint16_t current_heading_deg);
uint16_t NorthRef_Apply(uint16_t raw_heading_deg);

#endif
