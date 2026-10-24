/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifndef TARGET_UVK5_V2
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/portcon.h"
#include "driver/gpio.h"
#else
#include "driver/gpio_v2.h"
#endif
#include "driver/i2c.h"
#include "driver/systick.h"
#ifdef TARGET_UVK5_V2
#define I2C_SCL_SET() GPIO_V2_Set(GPIOA, (uint16_t)(1U << 5))
#define I2C_SCL_CLEAR() GPIO_V2_Clear(GPIOA, (uint16_t)(1U << 5))
#define I2C_SDA_SET() GPIO_V2_Set(GPIOA, (uint16_t)(1U << 6))
#define I2C_SDA_CLEAR() GPIO_V2_Clear(GPIOA, (uint16_t)(1U << 6))
#define I2C_SDA_READ() ((GPIO_V2_Read(GPIOA) & (1U << 6)) != 0U)
#define I2C_SDA_INPUT() ((void)GPIO_V2_Configure(GPIOA, (uint16_t)(1U << 6), GPIO_V2_MODE_INPUT, GPIO_V2_PULL_UP, 0U, 0U))
#define I2C_SDA_OUTPUT() ((void)GPIO_V2_Configure(GPIOA, (uint16_t)(1U << 6), GPIO_V2_MODE_OUTPUT, GPIO_V2_PULL_UP, 3U, 0U))
#else
#define I2C_SCL_SET() GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL)
#define I2C_SCL_CLEAR() GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL)
#define I2C_SDA_SET() GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA)
#define I2C_SDA_CLEAR() GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA)
#define I2C_SDA_READ() GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA)
#define I2C_SDA_INPUT() do { PORTCON_PORTA_IE |= PORTCON_PORTA_IE_A11_BITS_ENABLE; PORTCON_PORTA_OD &= ~PORTCON_PORTA_OD_A11_MASK; GPIOA->DIR &= ~GPIO_DIR_11_MASK; } while (0)
#define I2C_SDA_OUTPUT() do { PORTCON_PORTA_IE &= ~PORTCON_PORTA_IE_A11_MASK; PORTCON_PORTA_OD |= PORTCON_PORTA_OD_A11_BITS_ENABLE; GPIOA->DIR |= GPIO_DIR_11_BITS_OUTPUT; } while (0)
#endif


void I2C_Start(void) {
    I2C_SDA_SET();
    SYSTICK_DelayUs(1);
    I2C_SCL_SET();
    SYSTICK_DelayUs(1);
    I2C_SDA_CLEAR();
    SYSTICK_DelayUs(1);
    I2C_SCL_CLEAR();
    SYSTICK_DelayUs(1);
}

void I2C_Stop(void) {
    I2C_SDA_CLEAR();
    SYSTICK_DelayUs(1);
    I2C_SCL_CLEAR();
    SYSTICK_DelayUs(1);
    I2C_SCL_SET();
    SYSTICK_DelayUs(1);
    I2C_SDA_SET();
    SYSTICK_DelayUs(1);
}

uint8_t I2C_Read(bool bFinal) {
    uint8_t i, Data;

    PORTCON_PORTA_IE |= PORTCON_PORTA_IE_A11_BITS_ENABLE;
    PORTCON_PORTA_OD &= ~PORTCON_PORTA_OD_A11_MASK;
    GPIOA->DIR &= ~GPIO_DIR_11_MASK;

    Data = 0;
    for (i = 0; i < 8; i++) {
        I2C_SCL_CLEAR();
        SYSTICK_DelayUs(1);
        I2C_SCL_SET();
        SYSTICK_DelayUs(1);
        Data <<= 1;
        SYSTICK_DelayUs(1);
        if (I2C_SDA_READ()) {
            Data |= 1U;
        }
        I2C_SCL_CLEAR();
        SYSTICK_DelayUs(1);
    }

    PORTCON_PORTA_IE &= ~PORTCON_PORTA_IE_A11_MASK;
    PORTCON_PORTA_OD |= PORTCON_PORTA_OD_A11_BITS_ENABLE;
    GPIOA->DIR |= GPIO_DIR_11_BITS_OUTPUT;
    I2C_SCL_CLEAR();
    SYSTICK_DelayUs(1);
    if (bFinal) {
        I2C_SDA_SET();
    } else {
        I2C_SDA_CLEAR();
    }
    SYSTICK_DelayUs(1);
    I2C_SCL_SET();
    SYSTICK_DelayUs(1);
    I2C_SCL_CLEAR();
    SYSTICK_DelayUs(1);

    return Data;
}

int I2C_Write(uint8_t Data) {
    uint8_t i;
    int ret = -1;

    I2C_SCL_CLEAR();
    SYSTICK_DelayUs(1);
    for (i = 0; i < 8; i++) {
        if ((Data & 0x80) == 0) {
            I2C_SDA_CLEAR();
        } else {
            I2C_SDA_SET();
        }
        Data <<= 1;
        SYSTICK_DelayUs(1);
        I2C_SCL_SET();
        SYSTICK_DelayUs(1);
        I2C_SCL_CLEAR();
        SYSTICK_DelayUs(1);
    }

    PORTCON_PORTA_IE |= PORTCON_PORTA_IE_A11_BITS_ENABLE;
    PORTCON_PORTA_OD &= ~PORTCON_PORTA_OD_A11_MASK;
    GPIOA->DIR &= ~GPIO_DIR_11_MASK;
    I2C_SDA_SET();
    SYSTICK_DelayUs(1);
    I2C_SCL_SET();
    SYSTICK_DelayUs(1);

    for (i = 0; i < 255; i++) {
        if (I2C_SDA_READ() == 0) {
            ret = 0;
            break;
        }
    }

    I2C_SCL_CLEAR();
    SYSTICK_DelayUs(1);
    PORTCON_PORTA_IE &= ~PORTCON_PORTA_IE_A11_MASK;
    PORTCON_PORTA_OD |= PORTCON_PORTA_OD_A11_BITS_ENABLE;
    GPIOA->DIR |= GPIO_DIR_11_BITS_OUTPUT;
    I2C_SDA_SET();

    return ret;
}

int I2C_ReadBuffer(void *pBuffer, uint8_t Size) {
    uint8_t *pData = (uint8_t *) pBuffer;
    uint8_t i;

    if (Size == 1) {
        *pData = I2C_Read(true);
        return 1;
    }

    for (i = 0; i < Size - 1; i++) {
        SYSTICK_DelayUs(1);
        pData[i] = I2C_Read(false);
    }

    SYSTICK_DelayUs(1);
    pData[i++] = I2C_Read(true);

    return Size;
}

int I2C_WriteBuffer(const void *pBuffer, uint8_t Size) {
    const uint8_t *pData = (const uint8_t *) pBuffer;
    uint8_t i;

    for (i = 0; i < Size; i++) {
        if (I2C_Write(*pData++) < 0) {
            return -1;
        }
    }

    return 0;
}

