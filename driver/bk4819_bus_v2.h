/*
 * PY32F030 UV-K5 V2 bit-banged BK4819 bus.
 * Pins are from the V2 reference: PB3=SCN, PB4=SCL, PB5=SDA.
 * This isolated driver is not wired into the legacy DP32 BK4819 module yet.
 */
#ifndef TRUEMDC_BK4819_BUS_V2_H
#define TRUEMDC_BK4819_BUS_V2_H

#include <stdint.h>

void BK4819_V2_BusInit(void);
uint16_t BK4819_V2_ReadRegister(uint8_t reg);
void BK4819_V2_WriteRegister(uint8_t reg, uint16_t data);
void BK4819_V2_WriteU8(uint8_t data);
void BK4819_V2_WriteU16(uint16_t data);

#endif
