/*
 * Minimal PY32F030 GPIO primitives for the isolated UV-K5 V2 port.
 *
 * Uses the CMSIS register definitions from Puya's PY32F030 device header.
 * No DP32G030 register definitions or V1 pin assumptions are used here.
 */
#include "gpio_v2.h"

static int GPIO_V2_EnableClock(GPIO_TypeDef *port)
{
    if (port == GPIOA) {
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    } else if (port == GPIOB) {
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    } else if (port == GPIOF) {
        RCC->IOPENR |= RCC_IOPENR_GPIOFEN;
    } else {
        return -1;
    }

    /* Ensure the peripheral clock-enable write has taken effect. */
    (void)RCC->IOPENR;
    return 0;
}

int GPIO_V2_Configure(GPIO_TypeDef *port, uint16_t pins,
                      GPIO_V2_Mode mode, GPIO_V2_Pull pull,
                      uint32_t speed, uint32_t output_type)
{
    uint32_t pin;

    if (port == (GPIO_TypeDef *)0 || pins == 0U ||
        (uint32_t)mode > (uint32_t)GPIO_V2_MODE_ANALOG ||
        (uint32_t)pull > (uint32_t)GPIO_V2_PULL_DOWN ||
        speed > 3U || output_type > 1U ||
        GPIO_V2_EnableClock(port) != 0) {
        return -1;
    }

    for (pin = 0U; pin < 16U; ++pin) {
        const uint32_t bit = 1UL << pin;
        const uint32_t shift = pin * 2U;

        if (((uint32_t)pins & bit) == 0U) {
            continue;
        }

        port->MODER = (port->MODER & ~(3UL << shift)) |
                      ((uint32_t)mode << shift);
        port->PUPDR = (port->PUPDR & ~(3UL << shift)) |
                      ((uint32_t)pull << shift);
        port->OSPEEDR = (port->OSPEEDR & ~(3UL << shift)) |
                         (speed << shift);
        if (output_type != 0U) {
            port->OTYPER |= bit;
        } else {
            port->OTYPER &= ~bit;
        }
    }

    return 0;
}

void GPIO_V2_Set(GPIO_TypeDef *port, uint16_t pins)
{
    if (port != (GPIO_TypeDef *)0 && GPIO_V2_EnableClock(port) == 0) {
        port->BSRR = (uint32_t)pins;
    }
}

void GPIO_V2_Clear(GPIO_TypeDef *port, uint16_t pins)
{
    if (port != (GPIO_TypeDef *)0 && GPIO_V2_EnableClock(port) == 0) {
        port->BSRR = (uint32_t)pins << 16;
    }
}

uint16_t GPIO_V2_Read(GPIO_TypeDef *port)
{
    if (port == (GPIO_TypeDef *)0 || GPIO_V2_EnableClock(port) != 0) {
        return 0U;
    }
    return (uint16_t)port->IDR;
}
