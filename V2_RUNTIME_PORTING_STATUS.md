# V2 startup/runtime migration status

This file tracks the separate UV-K5 V2 startup path. The V1 startup and linker
remain unchanged.

## Added

- `start_v2.S`: isolated Cortex-M0+ / PY32F030 vector-table and reset-handler
  scaffold. It uses the V2 IRQ ordering from the `muzkr/uv-k5-firmware-custom`
  `v2` reference and keeps the application entry point as `Main`.
- `firmware_v2.ld`: separate V2 memory map (application flash at
  `0x08001000`, 60 KiB; RAM at `0x20000000`, 8 KiB).

## Not yet complete

1. `start_v2.S` is not selected by the Makefile. Do not change the default V1
   startup selection until the V2 target is explicitly selected.
2. The referenced `SystemInit` implementation and PY32 CMSIS device headers
   are not yet part of this TrueMDC branch. The V2 runtime must supply them,
   including the vector-table offset configuration.
3. Existing application ISRs use DP32 names and NVIC IRQ identifiers. They
   must be mapped to the actual PY32 peripherals; the current weak default
   handlers are placeholders, not functional interrupt bindings.
4. Existing `DATA_Init` / `BSS_Init` use the legacy linker symbols and should
   be checked against the final V2 linker layout and data-load address.
5. Hardware drivers still directly access DP32G030 registers. They have not
   been ported by adding this startup file.

## Safety gate

A V2 image is not build-ready or flash-ready. Before enabling a V2 build target,
port and compile the PY32 startup/runtime, GPIO/pin mux, clock/timing, SPI, I2C,
ADC, UART/DMA, timers/RTC, flash/EEPROM and interrupt handling. Then build from
a clean tree, inspect the map and image size, and test on a recoverable device.


## Follow-up audit (2026-10-24)

- Corrected `firmware_v2.ld` entry point from the V1 symbol
  `HandlerReset` to `Reset_Handler`, matching `start_v2.S`.
- Explicitly retain `.isr_vector` in the linker script so the PY32 vector
  table is not discarded when section garbage collection is enabled.
- The V1 startup's `DATA_Init` and `BSS_Init` are compatible in concept with
  the V2 scaffold's calls, because the V2 linker keeps the matching
  `flash_data_start`, `sram_data_start/end`, and `__bss_start__/end`
  symbols. This still needs a real link test.
- The legacy application entry point `Main` performs DP32-specific clock-gate
  writes before peripheral initialization. Therefore merely switching startup
  and linker scripts would be unsafe; the main initialization and every
  register-level BSP dependency must be converted before a V2 build can be
  selected.
- The reference repository's PY32 headers and HAL are under `Drivers/`, not
  `Core/Inc/`. Integration should bring in the matching device/CMSIS support
  and licensed vendor driver sources rather than fabricate register maps.
