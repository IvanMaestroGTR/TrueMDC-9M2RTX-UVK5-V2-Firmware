#include "board_v2.h"
#include "gpio_v2.h"

#define PIN(n) ((uint16_t)(1U << (n)))

void BOARD_V2_GPIO_Init(void)
{
    /* Enable/configure the same three ports used by the V2 board reference. */
    (void)GPIO_V2_Configure(GPIOA,
        PIN(15) | PIN(13) | PIN(12) | PIN(11) | PIN(10) | PIN(9) |
        PIN(8) | PIN(7) | PIN(6) | PIN(5),
        GPIO_V2_MODE_OUTPUT, GPIO_V2_PULL_UP, 3U, 0U);

    (void)GPIO_V2_Configure(GPIOB,
        PIN(7) | PIN(6) | PIN(5) | PIN(4) | PIN(3) | PIN(0),
        GPIO_V2_MODE_OUTPUT, GPIO_V2_PULL_UP, 3U, 0U);

    (void)GPIO_V2_Configure(GPIOA, PIN(0) | PIN(1) | PIN(3),
        GPIO_V2_MODE_INPUT, GPIO_V2_PULL_UP, 0U, 0U);

    (void)GPIO_V2_Configure(GPIOF, PIN(0) | PIN(1),
        GPIO_V2_MODE_INPUT, GPIO_V2_PULL_UP, 0U, 0U);

    /* Hold the two board control lines high as the reference does. */
    GPIO_V2_Set(GPIOA, PIN(9) | PIN(10));

    /* ADC inputs: battery voltage/current channels on PA4 and PB1. */
    (void)GPIO_V2_Configure(GPIOA, PIN(4),
        GPIO_V2_MODE_ANALOG, GPIO_V2_PULL_NONE, 0U, 0U);
    (void)GPIO_V2_Configure(GPIOB, PIN(1),
        GPIO_V2_MODE_ANALOG, GPIO_V2_PULL_NONE, 0U, 0U);
}
