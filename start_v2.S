/*
 * TrueMDC UV-K5 V2 startup scaffold.
 *
 * Target: PY32F030 (Cortex-M0+), application vector base 0x08001000.
 * This file is intentionally separate from start.S (DP32G030 / UV-K5 V1).
 *
 * Copyright (c) 2025 muzkr startup/vector layout reference:
 * https://github.com/muzkr/uv-k5-firmware-custom (branch v2)
 * Reference startup is MIT-licensed; this scaffold is a fresh minimal
 * adaptation. PY32F030 device IRQ order follows the vendor startup file.
 *
 * NOT YET SELECTED BY THE DEFAULT MAKEFILE. Do not flash until the PY32
 * peripheral drivers, interrupt bindings, clock setup, and linker/runtime
 * have all been ported and a complete image has been built and tested.
 */

.syntax unified
.cpu cortex-m0plus
.fpu softvfp
.thumb

.global g_pfnVectors
.global Reset_Handler
.global Default_Handler

.extern SystemInit
.extern DATA_Init
.extern BSS_Init
.extern Main

.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler, %function
Reset_Handler:
    ldr   r0, =_estack
    mov   sp, r0
    bl    SystemInit
    bl    DATA_Init
    bl    BSS_Init
    bl    Main
1:
    b     1b
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.type Default_Handler, %function
Default_Handler:
2:
    b     2b
.size Default_Handler, .-Default_Handler

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word 0
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_1_IRQHandler
    .word EXTI2_3_IRQHandler
    .word EXTI4_15_IRQHandler
    .word 0
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_3_IRQHandler
    .word 0
    .word ADC_COMP_IRQHandler
    .word TIM1_BRK_UP_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word 0
    .word TIM3_IRQHandler
    .word LPTIM1_IRQHandler
    .word 0
    .word TIM14_IRQHandler
    .word 0
    .word TIM16_IRQHandler
    .word TIM17_IRQHandler
    .word I2C1_IRQHandler
    .word 0
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word 0
    .word LED_IRQHandler
    .word 0
.size g_pfnVectors, .-g_pfnVectors

.macro weak_default name
    .weak \name
    .thumb_set \name, Default_Handler
.endm

weak_default NMI_Handler
weak_default HardFault_Handler
weak_default SVC_Handler
weak_default PendSV_Handler
weak_default SysTick_Handler
weak_default WWDG_IRQHandler
weak_default PVD_IRQHandler
weak_default RTC_IRQHandler
weak_default FLASH_IRQHandler
weak_default RCC_IRQHandler
weak_default EXTI0_1_IRQHandler
weak_default EXTI2_3_IRQHandler
weak_default EXTI4_15_IRQHandler
weak_default DMA1_Channel1_IRQHandler
weak_default DMA1_Channel2_3_IRQHandler
weak_default ADC_COMP_IRQHandler
weak_default TIM1_BRK_UP_TRG_COM_IRQHandler
weak_default TIM1_CC_IRQHandler
weak_default TIM3_IRQHandler
weak_default LPTIM1_IRQHandler
weak_default TIM14_IRQHandler
weak_default TIM16_IRQHandler
weak_default TIM17_IRQHandler
weak_default I2C1_IRQHandler
weak_default SPI1_IRQHandler
weak_default SPI2_IRQHandler
weak_default USART1_IRQHandler
weak_default USART2_IRQHandler
weak_default LED_IRQHandler
