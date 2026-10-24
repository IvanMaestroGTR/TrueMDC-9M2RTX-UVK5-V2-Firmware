/*
 * PY32F030 system clock and vector-table initialization for the TrueMDC V2
 * port. Adapted from Puya's CMSIS system_py32f0xx.c distributed by
 * muzkr/uv-k5-firmware-custom (branch v2).
 *
 * Copyright (c) 2023 Puya Semiconductor Co.
 * Copyright (c) 2016 STMicroelectronics.
 * Licensed under the BSD 3-Clause License. See the vendor source and license
 * notices in the reference repository before redistribution.
 *
 * This file is not selected by the V1 build. PY32F030 CMSIS device headers
 * are vendored under bsp/py32f030/ for the isolated V2 runtime.
 */
#include "../bsp/py32f030/device.h"

#ifndef HSE_VALUE
#define HSE_VALUE 24000000U
#endif
#ifndef HSI_VALUE
#define HSI_VALUE 8000000U
#endif
#ifndef LSI_VALUE
#define LSI_VALUE 32768U
#endif
#ifndef LSE_VALUE
#define LSE_VALUE 32768U
#endif

/* The application image begins at 0x08001000 in the V2 layout. */
#define VECT_TAB_OFFSET 0x1000U

/* SystemInit selects the 24 MHz HSI frequency field below. */
uint32_t SystemCoreClock = 24000000U;

const uint32_t AHBPrescTable[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
};
const uint32_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};
const uint32_t HSIFreqTable[8] = {
    4000000U, 8000000U, 16000000U, 22120000U,
    24000000U, 4000000U, 4000000U, 4000000U
};

void SystemCoreClockUpdate(void)
{
    uint32_t tmp;
    uint32_t hsidiv;
    uint32_t hsifs;

    switch (RCC->CFGR & RCC_CFGR_SWS) {
    case RCC_CFGR_SWS_0:
        SystemCoreClock = HSE_VALUE;
        break;
    case (RCC_CFGR_SWS_1 | RCC_CFGR_SWS_0):
        SystemCoreClock = LSI_VALUE;
        break;
#if defined(RCC_LSE_SUPPORT)
    case RCC_CFGR_SWS_2:
        SystemCoreClock = LSE_VALUE;
        break;
#endif
#if defined(RCC_PLL_SUPPORT)
    case RCC_CFGR_SWS_1:
        if ((RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI) {
            hsifs = (READ_BIT(RCC->ICSCR, RCC_ICSCR_HSI_FS) >>
                     RCC_ICSCR_HSI_FS_Pos);
            SystemCoreClock = 2U * HSIFreqTable[hsifs];
        } else {
            SystemCoreClock = 2U * HSE_VALUE;
        }
        break;
#endif
    case 0U:
    default:
        hsifs = (READ_BIT(RCC->ICSCR, RCC_ICSCR_HSI_FS) >>
                 RCC_ICSCR_HSI_FS_Pos);
        hsidiv = 1UL << (READ_BIT(RCC->CR, RCC_CR_HSIDIV) >>
                         RCC_CR_HSIDIV_Pos);
        SystemCoreClock = HSIFreqTable[hsifs] / hsidiv;
        break;
    }

    tmp = AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos];
    SystemCoreClock >>= tmp;
}

void SystemInit(void)
{
    /*
     * Match the actual V2 reference startup sequence. The legacy application
     * Main() cannot be entered until the PY32 clock tree is configured:
     * it used to do DP32-specific clock-gate writes before SYSTICK_Init().
     */
    RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN;
    RCC->APBENR1 |= RCC_APBENR1_PWREN;

    /* Select calibrated 24 MHz HSI while preserving the factory trim bits. */
    RCC->ICSCR = (RCC->ICSCR & 0xFFFF0000U) |
                 (4U << RCC_ICSCR_HSI_FS_Pos) |
                 ((*(const uint32_t *)0x1FFF0F04U) & 0x1FFFU);

    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {
        /* Wait until the internal oscillator is stable before switching. */
    }

    /* HSI system clock, AHB/APB undivided; 24 MHz requires zero wait states. */
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE);
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {
        /* Wait until HSI is confirmed as the system clock. */
    }
    FLASH->ACR &= ~FLASH_ACR_LATENCY;

    SystemCoreClock = 24000000U;

#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
#endif
}
