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
    GPIO_V2_Set(GPIOA, PIN(9) | PIN(10) | PIN(13));

    /* ADC inputs: battery voltage/current channels on PA4 and PB1. */
    (void)GPIO_V2_Configure(GPIOA, PIN(4),
        GPIO_V2_MODE_ANALOG, GPIO_V2_PULL_NONE, 0U, 0U);
    (void)GPIO_V2_Configure(GPIOB, PIN(1),
        GPIO_V2_MODE_ANALOG, GPIO_V2_PULL_NONE, 0U, 0U);
}


/*
 * PY32F030 battery monitor:
 *   ADC channel 4 / PA4 = battery voltage sense
 *   ADC channel 9 / PB1 = charging-current sense
 * Keep each conversion single-channel; reading one DR value for a two-channel
 * sequence would incorrectly report the same measurement for both quantities.
 */
#include "../bsp/py32f030/device.h"

void BOARD_V2_ADC_Init(void)
{
    RCC->APBENR2 |= RCC_APBENR2_ADCEN;

    /* Reset and calibrate with the ADC disabled. */
    RCC->APBRSTR2 |= RCC_APBRSTR2_ADCRST;
    RCC->APBRSTR2 &= ~RCC_APBRSTR2_ADCRST;
    ADC1->CR = 0U;
    ADC1->CFGR1 = 0U; /* 12-bit, right aligned, software-triggered, single shot. */
    ADC1->CFGR2 = ADC_CFGR2_CKMODE_1; /* Synchronous PCLK / 4, as in the V2 reference. */
    ADC1->SMPR = ADC_SMPR_SMP_Msk; /* Longest sample time for the board's sense network. */
    ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_EOSMP;

    ADC1->CR |= ADC_CR_ADCAL;
    for (volatile uint32_t timeout = 1000000U;
         (ADC1->CR & ADC_CR_ADCAL) != 0U && timeout != 0U; --timeout) {
    }

    ADC1->CR |= ADC_CR_ADEN;
    /* Allow the analog front end to settle after enabling. */
    for (volatile uint32_t settle = 20000U; settle != 0U; --settle) {
        __NOP();
    }
}

static uint16_t BOARD_V2_ADC_ReadChannel(uint32_t channel_mask)
{
    if ((ADC1->CR & ADC_CR_ADEN) == 0U) {
        return 0U;
    }

    /* Clear stale conversion flags and select exactly one input channel. */
    ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_EOSMP;
    ADC1->CHSELR = channel_mask;

    for (volatile uint32_t settle = 2000U; settle != 0U; --settle) {
        __NOP();
    }

    ADC1->CR |= ADC_CR_ADSTART;
    for (volatile uint32_t timeout = 1000000U;
         (ADC1->ISR & ADC_ISR_EOC) == 0U && timeout != 0U; --timeout) {
    }

    if ((ADC1->ISR & ADC_ISR_EOC) == 0U) {
        return 0U;
    }

    const uint16_t value = (uint16_t)(ADC1->DR & 0x0FFFU);
    ADC1->ISR = ADC_ISR_EOC;
    return value;
}

void BOARD_V2_ADC_GetBatteryInfo(uint16_t *pVoltage, uint16_t *pCurrent)
{
    if (pVoltage != 0) {
        *pVoltage = BOARD_V2_ADC_ReadChannel(ADC_CHSELR_CHSEL4);
    }
    if (pCurrent != 0) {
        *pCurrent = BOARD_V2_ADC_ReadChannel(ADC_CHSELR_CHSEL9);
    }
}
