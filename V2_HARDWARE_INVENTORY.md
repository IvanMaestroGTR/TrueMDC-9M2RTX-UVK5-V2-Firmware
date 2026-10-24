# UV-K5 V2 hardware port inventory

This document is for the `v2-k5-port` branch. TrueMDC remains the source tree; `muzkr/uv-k5-firmware-custom` branch `v2` is a hardware reference only.

## Verified hardware/build deltas

| Area | TrueMDC V1 target | V2 reference |
|---|---|---|
| MCU | DP32G030, Cortex-M0 | PY32F030, Cortex-M0+ |
| RAM | 16 KiB; current startup stack hard-coded to 0x20003FF0 | 8 KiB; V2 linker stack top is 0x20002000 |
| Application flash | Current linker starts at 0x00000000 with 60 KiB | V2 reference links application at 0x08001000 with 60 KiB |
| Startup | DP32 vector table and custom DATA_Init/BSS_Init | PY32 vector table, SystemInit, standard data/BSS init |
| MCU access | Direct DP32G030 register headers under `bsp/dp32g030` | PY32 CMSIS device definitions and PY32 LL drivers |
| Build | Makefile-based; V1 BSP/startup objects | V2 reference uses CMake and a PY32 LL-driver tree |

The RAM size and flash/vector-table placement are hard blockers. Existing startup and linker scripts cannot be reused unchanged. The PY32 register definitions cannot safely be aliased to DP32 registers.

## Initial TrueMDC MCU dependency map

- `driver/system.c`: clock setup and delay base
- `driver/adc.c`: SAR ADC registers and clock selection
- `driver/spi.c`: SPI registers and IRQ numbers
- `driver/uart.c`: UART, DMA, and clock-gate registers
- `driver/timer.c`, `driver/rtc.c`: timer/RTC setup and interrupts
- `driver/flash.h`, `bsp/dp32g030/flash.h`: flash controller/programming assumptions
- `main.c`, `start.S`, `init.c`, `firmware.ld`: startup, vector table, memory map, runtime init
- Other direct `bsp/dp32g030/*` includes across drivers and application code

## Implementation order

1. Add a separately selectable PY32F030 startup, linker, and runtime-initialisation path while keeping V1 as the unchanged default.
2. Port clocks and GPIO, then SPI/BK4819 access, timers/interrupts, ADC, UART/DMA, and flash/EEPROM support.
3. Resolve all direct DP32 BSP dependencies; preserve TrueMDC's higher-level radio, UI, MDC1200, FleetSync, PTT-ID, contact aliases, preamble, TPT, and CHIRP behaviour.
4. Compile after each coherent hardware layer and verify the resulting ELF/map/bin against the V2 flash/RAM layout.
5. Only after build validation, test boot, display/keypad, receive/transmit, signalling and EEPROM on actual V2 hardware.

## Safety/status

This is a hardware port, not a feature migration. No MCU driver has yet been ported and no V2 firmware image has been built. Do not flash a V1 binary to V2 hardware; the MCU generations are not firmware-compatible. Successful compilation is necessary but not sufficient for declaring the port complete.
