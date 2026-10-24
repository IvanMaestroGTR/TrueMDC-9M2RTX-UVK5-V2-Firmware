/*
 * Target-selectable BK4819 bus adapter for the UV-K5 V2 PY32F030 port.
 *
 * This header is included by the legacy BK4819 driver only when
 * TARGET_UVK5_V2 is defined. It intentionally leaves non-bus control GPIOs
 * to their own board-specific port; those signals must not be guessed from
 * DP32 GPIOC bit numbers.
 */
#ifndef TRUEMDC_BK4819_V2_ADAPTER_H
#define TRUEMDC_BK4819_V2_ADAPTER_H

#ifdef TARGET_UVK5_V2
#include <stdint.h>
#include "bk4819_bus_v2.h"

static inline void BK4819_TARGET_BusInit(void)
{
    BK4819_V2_BusInit();
}

static inline uint16_t BK4819_TARGET_ReadRegister(uint8_t reg)
{
    return BK4819_V2_ReadRegister(reg);
}

static inline void BK4819_TARGET_WriteRegister(uint8_t reg, uint16_t value)
{
    BK4819_V2_WriteRegister(reg, value);
}

static inline void BK4819_TARGET_WriteU8(uint8_t value)
{
    BK4819_V2_WriteU8(value);
}

static inline void BK4819_TARGET_WriteU16(uint16_t value)
{
    BK4819_V2_WriteU16(value);
}
#endif

#endif
