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

#include <stdint.h>
#include <stdio.h>     // NULL

#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/spi.h"
#include "driver/gpio.h"
#include "driver/spi.h"
#include "driver/st7565.h"
#ifndef TARGET_UVK5_V2
#include "driver/system.h"
#endif
#include "misc.h"
#include "settings.h"   // for gEeprom.SCREEN_INVERT
#ifdef TARGET_UVK5_V2
#define LCD_CS_LOW()   GPIO_V2_Clear(GPIOA, (uint16_t)(1U << 9))
#define LCD_CS_HIGH()  GPIO_V2_Set(GPIOA, (uint16_t)(1U << 9))
#define LCD_A0_LOW()   GPIO_V2_Clear(GPIOA, (uint16_t)(1U << 11))
#define LCD_A0_HIGH()  GPIO_V2_Set(GPIOA, (uint16_t)(1U << 11))
#define LCD_SCK_LOW()  GPIO_V2_Clear(GPIOA, (uint16_t)(1U << 10))
#define LCD_SCK_HIGH() GPIO_V2_Set(GPIOA, (uint16_t)(1U << 10))
#define LCD_SDA_LOW()  GPIO_V2_Clear(GPIOA, (uint16_t)(1U << 12))
#define LCD_SDA_HIGH() GPIO_V2_Set(GPIOA, (uint16_t)(1U << 12))
#define LCD_TX_BYTE(value) do { uint8_t lcd_b_ = (uint8_t)(value); for (unsigned lcd_i_ = 0; lcd_i_ < 8U; ++lcd_i_) { LCD_SCK_LOW(); if ((lcd_b_ & 0x80U) != 0U) LCD_SDA_HIGH(); else LCD_SDA_LOW(); LCD_SCK_HIGH(); lcd_b_ <<= 1; } } while (0)
#define LCD_TX_WAIT() ((void)0)
#define LCD_SPI_INIT() do { LCD_CS_HIGH(); LCD_A0_HIGH(); LCD_SCK_HIGH(); LCD_SDA_HIGH(); } while (0)
#define LCD_SPI_BEGIN() LCD_CS_LOW()
#define LCD_SPI_END() LCD_CS_HIGH()
#define LCD_RESET_PIN_HIGH() ((void)0)
#define LCD_RESET_PIN_LOW() ((void)0)
#else
#define LCD_A0_LOW() GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0)
#define LCD_A0_HIGH() GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0)
#define LCD_TX_BYTE(value) do { while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {} SPI0->WDR = (value); } while (0)
#define LCD_TX_WAIT() SPI_WaitForUndocumentedTxFifoStatusBit()
#define LCD_SPI_INIT() SPI0_Init()
#define LCD_SPI_BEGIN() SPI_ToggleMasterMode(&SPI0->CR, false)
#define LCD_SPI_END() SPI_ToggleMasterMode(&SPI0->CR, true)
#define LCD_RESET_PIN_HIGH() GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES)
#define LCD_RESET_PIN_LOW() GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES)
#endif


uint8_t gStatusLine[LCD_WIDTH];
uint8_t gFrameBuffer[FRAME_LINES][LCD_WIDTH];

static void DrawLine(uint8_t column, uint8_t line, const uint8_t *lineBuffer, unsigned size_defVal) {
    ST7565_SelectColumnAndLine(column + 4, line);
    LCD_A0_HIGH();
    for (unsigned i = 0; i < size_defVal; i++) {
        uint8_t byte = lineBuffer ? lineBuffer[i] : size_defVal;
        if (gEeprom.SCREEN_INVERT)
            byte ^= 0xFF;
        while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
        SPI0->WDR = byte;
    }
    LCD_TX_WAIT();
}

void
ST7565_DrawLine(const unsigned int Column, const unsigned int Line, const uint8_t *pBitmap, const unsigned int Size) {
    LCD_SPI_BEGIN();
    DrawLine(Column, Line, pBitmap, Size);
    LCD_SPI_END();
}

void ST7565_BlitFullScreen(void) {
    SPI_ToggleMasterMode(&SPI0->CR, false);
    ST7565_WriteByte(0x40);
    for (unsigned line = 0; line < FRAME_LINES; line++) {
        DrawLine(0, line + 1, gFrameBuffer[line], LCD_WIDTH);
    }
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

void ST7565_BlitLine(unsigned line) {
    SPI_ToggleMasterMode(&SPI0->CR, false);
    ST7565_WriteByte(0x40);    // start line ?
    DrawLine(0, line + 1, gFrameBuffer[line], LCD_WIDTH);
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

void ST7565_BlitStatusLine(void) {    // the top small text line on the display
    SPI_ToggleMasterMode(&SPI0->CR, false);
    ST7565_WriteByte(0x40);    // start line ?
    DrawLine(0, 0, gStatusLine, LCD_WIDTH);
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

void ST7565_FillScreen(uint8_t value) {
    SPI_ToggleMasterMode(&SPI0->CR, false);
    for (unsigned i = 0; i < 8; i++) {
        DrawLine(0, i, NULL, value);
    }
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

// Software reset
const uint8_t ST7565_CMD_SOFTWARE_RESET = 0xE2;
// Bias Select
// 1 0 1 0 0 0 1 BS
// Select bias setting 0=1/9; 1=1/7 (at 1/65 duty)
const uint8_t ST7565_CMD_BIAS_SELECT = 0xA2;
// COM Direction
// 1 1 0 0 MY - - -
// Set output direction of COM
// MY=1, reverse direction
// MY=0, normal direction
const uint8_t ST7565_CMD_COM_DIRECTION = 0xC0;
// SEG Direction
// 1 0 1 0 0 0 0 MX
// Set scan direction of SEG
// MX=1, reverse direction
// MX=0, normal direction
const uint8_t ST7565_CMD_SEG_DIRECTION = 0xA0;
// Inverse Display
// 1 0 1 0 0 1 1 INV
// INV =1, inverse display
// INV =0, normal display
const uint8_t ST7565_CMD_INVERSE_DISPLAY = 0xA6;
// All Pixel ON
// 1 0 1 0 0 1 0 AP
// AP=1, set all pixel ON
// AP=0, normal display
const uint8_t ST7565_CMD_ALL_PIXEL_ON = 0xA4;
// Regulation Ratio
// 0 0 1 0 0 RR2 RR1 RR0
// This instruction controls the regulation ratio of the built-in regulator
const uint8_t ST7565_CMD_REGULATION_RATIO = 0x20;
// Double command!! Set electronic volume (EV) level
// Send next: 0 0 EV5 EV4 EV3 EV2 EV1 EV0  contrast 0-63
const uint8_t ST7565_CMD_SET_EV = 0x81;
// Control built-in power circuit ON/OFF - 0 0 1 0 1 VB VR VF
// VB: Built-in Booster
// VR: Built-in Regulator
// VF: Built-in Follower
const uint8_t ST7565_CMD_POWER_CIRCUIT = 0x28;
// Set display start line 0-63
// 0 0 0 1 S5 S4 S3 S2 S1 S0
const uint8_t ST7565_CMD_SET_START_LINE = 0x40;
// Display ON/OFF
// 0 0 1 0 1 0 1 1 1 D
// D=1, display ON
// D=0, display OFF
const uint8_t ST7565_CMD_DISPLAY_ON_OFF = 0xAE;

uint8_t cmds[] = {
        ST7565_CMD_BIAS_SELECT | 0,            // Select bias setting: 1/9
        ST7565_CMD_COM_DIRECTION | (0 << 3),    // Set output direction of COM: normal
        ST7565_CMD_SEG_DIRECTION | 1,            // Set scan direction of SEG: reverse
        ST7565_CMD_INVERSE_DISPLAY | 0,        // Inverse Display: false
        ST7565_CMD_ALL_PIXEL_ON | 0,            // All Pixel ON: false - normal display
        ST7565_CMD_REGULATION_RATIO | (4 << 0), // Regulation Ratio 5.0

        ST7565_CMD_SET_EV,                        // Set contrast
        31,

        ST7565_CMD_POWER_CIRCUIT | 0b111,        // Built-in power circuit ON/OFF: VB=1 VR=1 VF=1
        ST7565_CMD_SET_START_LINE | 0,            // Set Start Line: 0
        ST7565_CMD_DISPLAY_ON_OFF | 1,            // Display ON/OFF: ON
};

void ST7565_Init(void) {
    LCD_SPI_INIT();
    ST7565_HardwareReset();
    SPI_ToggleMasterMode(&SPI0->CR, false);
    ST7565_WriteByte(ST7565_CMD_SOFTWARE_RESET);   // software reset
    SYSTEM_DelayMs(120);

    for (uint8_t i = 0; i < 8; i++)
        ST7565_WriteByte(cmds[i]);

    ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b011);   // VB=0 VR=1 VF=1
    SYSTEM_DelayMs(1);
    ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b110);   // VB=1 VR=1 VF=0
    SYSTEM_DelayMs(1);

    for (uint8_t i = 0; i < 4; i++) // why 4 times?
        ST7565_WriteByte(ST7565_CMD_POWER_CIRCUIT | 0b111);   // VB=1 VR=1 VF=1

    SYSTEM_DelayMs(40);

    ST7565_WriteByte(ST7565_CMD_SET_START_LINE | 0);   // line 0
    ST7565_WriteByte(ST7565_CMD_DISPLAY_ON_OFF | 1);   // D=1
    SPI_WaitForUndocumentedTxFifoStatusBit();
    SPI_ToggleMasterMode(&SPI0->CR, true);

    ST7565_FillScreen(0x00);
}

void ST7565_FixInterfGlitch(void) {
    SPI_ToggleMasterMode(&SPI0->CR, false);
    for (uint8_t i = 0; i < ARRAY_SIZE(cmds); i++)
        ST7565_WriteByte(cmds[i]);
    SPI_WaitForUndocumentedTxFifoStatusBit();
    SPI_ToggleMasterMode(&SPI0->CR, true);
}

void ST7565_HardwareReset(void) {
    LCD_RESET_PIN_HIGH();
    SYSTEM_DelayMs(1);
    LCD_RESET_PIN_LOW();
    SYSTEM_DelayMs(20);
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(120);
}

void ST7565_SelectColumnAndLine(uint8_t Column, uint8_t Line) {
    LCD_A0_LOW();
    LCD_TX_BYTE(Line + 176);
    LCD_TX_BYTE(((Column >> 4) & 0x0F) | 0x10);
    LCD_TX_BYTE((Column >> 0) & 0x0F);
    SPI_WaitForUndocumentedTxFifoStatusBit();
}

void ST7565_WriteByte(uint8_t Value) {
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
    LCD_TX_BYTE(Value);
}