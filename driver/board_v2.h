/*
 * UV-K5 V2 board GPIO setup, based on the PY32F030 V2 reference board.c.
 * This configures only documented board pins; peripheral-specific init
 * (display, radio serial/bit-bang, ADC calibration, etc.) remains separate.
 */
#ifndef TRUEMDC_BOARD_V2_H
#define TRUEMDC_BOARD_V2_H

#include <stdint.h>

void BOARD_V2_GPIO_Init(void);

#endif
