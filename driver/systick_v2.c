/*
 * PY32F030 SysTick timing driver for the isolated UV-K5 V2 port.
 *
 * Keeps the legacy driver API while deriving timing from the configured
 * SystemCoreClock instead of the V1-only hard-coded 48 MHz value.
 */
#include <stdint.h>

#include "../bsp/py32f030/device.h"
#include "systick.h"

extern uint32_t SystemCoreClock;

static uint32_t gTickMultiplier;

void SYSTICK_Init(void)
{
    /* 10 ms SysTick period, matching the V1 scheduler cadence. */
    (void)SysTick_Config(SystemCoreClock / 100U);
    gTickMultiplier = SystemCoreClock / 1000000U;
}

static void SYSTICK_WaitTicks(uint32_t ticks)
{
    const uint32_t load = SysTick->LOAD;
    uint32_t elapsed = 0U;
    uint32_t previous = SysTick->VAL;

    while (elapsed < ticks) {
        uint32_t current;
        do {
            current = SysTick->VAL;
        } while (current == previous);

        /* SysTick is a down-counter; account for a reload between samples. */
        if (current < previous) {
            elapsed += previous - current;
        } else {
            elapsed += previous + (load + 1U - current);
        }
        previous = current;
    }
}

void SYSTICK_DelayUs(uint32_t delay)
{
    SYSTICK_WaitTicks(delay * gTickMultiplier);
}

void SYSTICK_Delay250ns(const uint32_t delay)
{
    SYSTICK_WaitTicks((delay * gTickMultiplier) / 4U);
}
