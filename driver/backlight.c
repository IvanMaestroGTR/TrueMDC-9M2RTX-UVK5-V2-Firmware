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

#include "backlight.h"
#ifndef TARGET_UVK5_V2
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/pwmplus.h"
#include "bsp/dp32g030/portcon.h"
#include "driver/gpio.h"
#else
#include "driver/gpio_v2.h"
#include "bsp/py32f030/device.h"
#endif
#include "settings.h"

// this is decremented once every 500ms
uint16_t gBacklightCountdown_500ms = 0;
bool backlightOn;

#define BACKLIGHT_FADE_TICKS 20u

static uint8_t backlightTargetBrightness = 0;
static uint8_t backlightFadeTick = 0;
static uint8_t backlightFadeStart = 0;
static bool backlightFading = false;
static uint8_t currentBrightness;

static void BACKLIGHT_ApplyBrightness(uint8_t brightness) {
    currentBrightness = brightness;
    backlightOn = brightness > 0;
#ifdef TARGET_UVK5_V2
    /* V2 reference: PA8 / AF2 = TIM1_CH1, 1 kHz PWM. */
    TIM1->CCR1 = ((uint32_t)brightness * (TIM1->ARR + 1U)) / 255U;
#else
    if (brightness >= 10u) {
        PWM_PLUS0_CH0_COMP = PWM_PLUS0_PERIOD;
    } else {
        PWM_PLUS0_CH0_COMP = (uint16_t)((uint32_t)brightness * PWM_PLUS0_PERIOD / 10u);
    }
#endif
}

void BACKLIGHT_InitHardware() {
#ifdef TARGET_UVK5_V2
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
    RCC->APBRSTR2 |= RCC_APBRSTR2_TIM1RST;
    RCC->APBRSTR2 &= ~RCC_APBRSTR2_TIM1RST;
    (void)GPIO_V2_Configure(GPIOA, (uint16_t)(1U << 8),
        GPIO_V2_MODE_ALTERNATE, GPIO_V2_PULL_NONE, 3U, 0U);
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(0xFUL << 0)) | (2UL << 0);
    TIM1->CR1 = 0U;
    TIM1->PSC = (SystemCoreClock / 100000U) - 1U;
    TIM1->ARR = 99U; /* 1 kHz PWM at a 100 kHz timer tick. */
    TIM1->CCR1 = 0U;
    TIM1->CCMR1 = (TIM1->CCMR1 & ~0xFFU) | (6UL << 4) | (1UL << 3);
    TIM1->CCER = (TIM1->CCER & ~0xFU) | 1UL;
    TIM1->BDTR |= (1UL << 15); /* Advanced timer main output enable. */
    TIM1->EGR = 1UL;
    TIM1->CR1 |= 1UL;
#else
    // 48MHz / 94 / 1024 ~ 500Hz
    const uint32_t PWM_FREQUENCY_HZ = 25000;
    PWM_PLUS0_CLKSRC |= ((48000000 / 1024 / PWM_FREQUENCY_HZ) << 16);
    PWM_PLUS0_PERIOD = 1023;

    PORTCON_PORTB_SEL0 &= ~(0
                            | PORTCON_PORTB_SEL0_B6_MASK);
    PORTCON_PORTB_SEL0 |= PORTCON_PORTB_SEL0_B6_BITS_PWMP0_CH0;

    PWM_PLUS0_GEN = PWMPLUS_GEN_CH0_OE_BITS_ENABLE |
                    PWMPLUS_GEN_CH0_OUTINV_BITS_ENABLE;
    PWM_PLUS0_CFG = PWMPLUS_CFG_CNT_REP_BITS_ENABLE |
                    PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE;
#endif
}
unsigned short BACKLIGHT_MAP[7]={11,21,41,121,241,481,0};

void BACKLIGHT_TurnOn(void) {
    if (gEeprom.BACKLIGHT_TIME == 0) {
        BACKLIGHT_TurnOff();
        return;
    }

    backlightOn = true;
    backlightTargetBrightness = gEeprom.BACKLIGHT_MAX;
    backlightFadeStart = gEeprom.BACKLIGHT_MAX;
    backlightFadeTick = 0;
    backlightFading = false;
    BACKLIGHT_ApplyBrightness(gEeprom.BACKLIGHT_MAX);
    gBacklightCountdown_500ms = BACKLIGHT_MAP[gEeprom.BACKLIGHT_TIME - 1];
}

void BACKLIGHT_TurnOff() {
    backlightTargetBrightness = 0;
    backlightFadeStart = currentBrightness;
    backlightFadeTick = 0;
    backlightFading = true;
    gBacklightCountdown_500ms = 0;
}

void BACKLIGHT_TimeSlice10ms(void) {
    if (!backlightFading)
        return;

    if (backlightTargetBrightness == backlightFadeStart) {
        BACKLIGHT_ApplyBrightness(backlightTargetBrightness);
        backlightFading = false;
        return;
    }

    backlightFadeTick++;
    if (backlightFadeTick >= BACKLIGHT_FADE_TICKS) {
        BACKLIGHT_ApplyBrightness(backlightTargetBrightness);
        backlightFading = false;
        backlightFadeTick = 0;
        backlightFadeStart = backlightTargetBrightness;
        return;
    }

    const uint16_t startValue = backlightFadeStart;
    const uint16_t endValue = backlightTargetBrightness;
    const uint16_t blended = ((startValue * (BACKLIGHT_FADE_TICKS - backlightFadeTick)) +
                              (endValue * backlightFadeTick)) / BACKLIGHT_FADE_TICKS;
    BACKLIGHT_ApplyBrightness((uint8_t)blended);
}

bool BACKLIGHT_IsOn() {
    return backlightOn;
}

void BACKLIGHT_SetBrightness(uint8_t brigtness) {
    backlightFading = false;
    backlightFadeTick = 0;
    backlightTargetBrightness = brigtness;
    BACKLIGHT_ApplyBrightness(brigtness);
    backlightOn = brigtness > 0;
    //PWM_PLUS0_SWLOAD = 1;
}

uint8_t BACKLIGHT_GetBrightness(void) {
    return currentBrightness;
}