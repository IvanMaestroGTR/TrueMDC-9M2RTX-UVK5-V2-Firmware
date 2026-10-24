/*
 * Minimal PY32F030 GPIO primitives for the isolated UV-K5 V2 port.
 *
 * This is a register-level foundation only. Board-specific pin assignments
 * must come from verified V2 schematics/reference code before use by drivers.
 */
#ifndef TRUEMDC_GPIO_V2_H
#define TRUEMDC_GPIO_V2_H

#include <stdint.h>
#include "../bsp/py32f030/device.h"

typedef enum {
    GPIO_V2_MODE_INPUT = 0U,
    GPIO_V2_MODE_OUTPUT = 1U,
    GPIO_V2_MODE_ALTERNATE = 2U,
    GPIO_V2_MODE_ANALOG = 3U
} GPIO_V2_Mode;

typedef enum {
    GPIO_V2_PULL_NONE = 0U,
    GPIO_V2_PULL_UP = 1U,
    GPIO_V2_PULL_DOWN = 2U
} GPIO_V2_Pull;

/* Returns 0 on success, -1 for an unsupported port or invalid setting. */
int GPIO_V2_Configure(GPIO_TypeDef *port, uint16_t pins,
                      GPIO_V2_Mode mode, GPIO_V2_Pull pull,
                      uint32_t speed, uint32_t output_type);
void GPIO_V2_Set(GPIO_TypeDef *port, uint16_t pins);
void GPIO_V2_Clear(GPIO_TypeDef *port, uint16_t pins);
uint16_t GPIO_V2_Read(GPIO_TypeDef *port);

#endif
