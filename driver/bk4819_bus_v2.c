#include "bk4819_bus_v2.h"
#include "gpio_v2.h"
#include "systick.h"

#define PIN(n) ((uint16_t)(1U << (n)))
#define SCN PIN(3)
#define SCL PIN(4)
#define SDA PIN(5)

static void delay_edge(void)
{
    /* Match the V2 reference BK4819 bit-bang timing (1 us per edge). */
    SYSTICK_DelayUs(1U);
}

void BK4819_V2_BusInit(void)
{
    (void)GPIO_V2_Configure(GPIOB, SCN | SCL | SDA,
        GPIO_V2_MODE_OUTPUT, GPIO_V2_PULL_UP, 3U, 0U);
    GPIO_V2_Set(GPIOB, SCN | SCL | SDA);
}

void BK4819_V2_WriteU8(uint8_t data)
{
    unsigned int i;
    GPIO_V2_Clear(GPIOB, SCL);
    for (i = 0U; i < 8U; ++i) {
        if ((data & 0x80U) != 0U) {
            GPIO_V2_Set(GPIOB, SDA);
        } else {
            GPIO_V2_Clear(GPIOB, SDA);
        }
        delay_edge();
        GPIO_V2_Set(GPIOB, SCL);
        delay_edge();
        data <<= 1;
        GPIO_V2_Clear(GPIOB, SCL);
        delay_edge();
    }
}

void BK4819_V2_WriteU16(uint16_t data)
{
    unsigned int i;
    GPIO_V2_Clear(GPIOB, SCL);
    for (i = 0U; i < 16U; ++i) {
        if ((data & 0x8000U) != 0U) {
            GPIO_V2_Set(GPIOB, SDA);
        } else {
            GPIO_V2_Clear(GPIOB, SDA);
        }
        delay_edge();
        GPIO_V2_Set(GPIOB, SCL);
        delay_edge();
        data <<= 1;
        GPIO_V2_Clear(GPIOB, SCL);
        delay_edge();
    }
}

uint16_t BK4819_V2_ReadRegister(uint8_t reg)
{
    uint16_t value = 0U;
    unsigned int i;

    GPIO_V2_Set(GPIOB, SCN);
    GPIO_V2_Clear(GPIOB, SCL);
    delay_edge();
    GPIO_V2_Clear(GPIOB, SCN);
    BK4819_V2_WriteU8((uint8_t)(reg | 0x80U));

    (void)GPIO_V2_Configure(GPIOB, SDA,
        GPIO_V2_MODE_INPUT, GPIO_V2_PULL_NONE, 0U, 0U);
    delay_edge();
    for (i = 0U; i < 16U; ++i) {
        value <<= 1;
        if ((GPIO_V2_Read(GPIOB) & SDA) != 0U) {
            value |= 1U;
        }
        GPIO_V2_Set(GPIOB, SCL);
        delay_edge();
        GPIO_V2_Clear(GPIOB, SCL);
        delay_edge();
    }
    (void)GPIO_V2_Configure(GPIOB, SDA,
        GPIO_V2_MODE_OUTPUT, GPIO_V2_PULL_UP, 3U, 0U);
    GPIO_V2_Set(GPIOB, SCN);
    delay_edge();
    GPIO_V2_Set(GPIOB, SCL | SDA);
    return value;
}

void BK4819_V2_WriteRegister(uint8_t reg, uint16_t data)
{
    GPIO_V2_Set(GPIOB, SCN);
    GPIO_V2_Clear(GPIOB, SCL);
    delay_edge();
    GPIO_V2_Clear(GPIOB, SCN);
    BK4819_V2_WriteU8(reg);
    BK4819_V2_WriteU16(data);
    GPIO_V2_Set(GPIOB, SCN);
    delay_edge();
    GPIO_V2_Set(GPIOB, SCL | SDA);
}
