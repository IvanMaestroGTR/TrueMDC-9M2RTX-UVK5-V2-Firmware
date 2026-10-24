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
2. `driver/system_v2.c` and the PY32 CMSIS device headers are now present
   under `bsp/py32f030/`. A smoke workflow compiles the system file and startup,
   then links a minimal image. This validates the scaffold, not the application.
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


## Runtime smoke test (2026-10-24)

- GitHub Actions run `40089751615` passed: the PY32F030 system initialization
  compiled with warnings treated as errors, the V2 startup assembled, and the
  startup/system objects linked against `firmware_v2.ld` with minimal stubs.
- The independent V1 build workflow also passed on the same branch commit.
- The initial `SystemCoreClock` value is explicitly 24 MHz to match the HSI
  frequency selected by `SystemInit`; the previous 8 MHz initial value was
  inconsistent until `SystemCoreClockUpdate` ran.
- This remains a runtime smoke test only. It does not compile or validate the
  DP32-coupled application drivers and does not produce a flashable TrueMDC V2
  image. Next milestone is a PY32 board/peripheral layer and a real V2-linked
  application, not simply enabling the V2 startup in the normal build.


## Clock initialization follow-up (2026-10-24)

- Expanded `SystemInit()` to perform the PY32F030-specific early clock setup
  before `Main()`: enable SYSCFG/PWR peripheral clocks, select calibrated
  24 MHz HSI, wait for oscillator-ready status, select undivided AHB/APB clocks,
  set zero flash wait states, set `SystemCoreClock`, and relocate VTOR to the
  application base. This follows the sequence in the F4HWN V2 reference's
  `Core/Src/main.c` without depending on its HAL/LL layer.
- This change is awaiting its own CI compile/link result. The earlier passing
  smoke run predates this expanded clock sequence.


## SysTick driver port (2026-10-24)

- Added `driver/systick_v2.c`, retaining the legacy `SYSTICK_Init`,
  `SYSTICK_DelayUs`, and `SYSTICK_Delay250ns` API while deriving the tick
  period and microsecond scale from `SystemCoreClock` (24 MHz on V2), rather
  than the V1 driver's hard-coded 48 MHz values.
- Delay polling now accounts for SysTick reloads correctly when a delay spans
  the 10 ms counter period.
- The V2 smoke workflow now compiles this driver and includes it in the minimal
  link. This driver is not yet selected by the default Makefile; V1 remains
  unchanged. The CI result for this latest addition is pending.


## Verified runtime and GPIO follow-up (2026-10-24)

- GitHub Actions run `40091262405` passed for commit
  `90dfc2609f1ad201d438306a4c965b80bded5a03`: the PY32F030 system
  initialization, SysTick driver, reset/vector assembly, and minimal link all
  completed successfully with compiler warnings treated as errors. The
  independent V1 `build` job also passed on that commit.
- Added `driver/gpio_v2.c` and `driver/gpio_v2.h` as an isolated CMSIS
  register-level GPIO foundation. It supports GPIOA, GPIOB and GPIOF clock
  enabling, input/output/alternate/analog mode, pull configuration, output
  speed/type, and pin set/clear/read.
- The new GPIO API deliberately contains no radio-board pin assignments.
  Those must be copied from verified V2 reference wiring; V1 DP32 pin/register
  assumptions are not safe to reuse.
- The V2 smoke workflow was expanded to compile and link the GPIO object.
  The result for the expanded workflow is pending; this does not yet validate
  the full app, RF signalling, display, keyboard, or EEPROM path.


## Board GPIO initialization and CI follow-up (2026-10-24)

- The first workflow that included `gpio_v2.c` failed before compilation because
  YAML shell continuation characters in the new step were accidentally doubled.
  The compiler log confirmed this was a workflow command-format issue, not a C
  diagnostic. The continuation characters were corrected in a follow-up commit.
- Added `driver/board_v2.c` and `driver/board_v2.h`. The initial board setup
  is based on the V2 reference `App/board.c`: output groups on GPIOA/GPIOB,
  input rows on GPIOA/GPIOF, PTT input with pull-up, PA9/PA10 held high, and
  PA4/PB1 placed in analog mode for battery ADC channels.
- This is only the GPIO stage of board initialization. It does not yet initialize
  ADC calibration/conversion, LCD, keyboard scanning, BK4819 signalling pins,
  EEPROM/flash access, or the radio's timing-sensitive bit-bang paths.
- The latest workflow now compiles and links both the GPIO primitives and the
  board GPIO setup. Its result is pending; do not treat the V2 firmware as
  boot-tested or flashable until that CI passes and the remaining peripherals
  and application dependencies are ported.


## BK4819 bus foundation (2026-10-24)

- Added `driver/bk4819_bus_v2.c/.h`, a separate PY32F030 bit-banged bus
  implementation using the V2 reference pin mapping PB3=SCN, PB4=SCL, PB5=SDA.
  It provides byte/word writes and register read/write primitives and uses the
  V2 SysTick delay API.
- The V2 smoke workflow now compiles and links the bus driver as well as the
  startup, clock, SysTick, GPIO and board-GPIO layers.
- This bus module is intentionally isolated: the legacy `driver/bk4819.c`
  still contains DP32G030 GPIO/PORTCON register accesses and has not been
  switched to this API. Before integrating it, protocol timing and read-turnaround
  behavior must be reviewed on the V2 hardware/reference, and the old direct
  register accesses must be routed through a target-specific abstraction.
- Latest expanded workflow is pending. A compile/link pass would establish
  only that these isolated objects build together, not that RF signalling works.


## V2 bus timing review (2026-10-24)

- The isolated runtime smoke workflow passed on commit `a113febf0ddc722eff0c15c1cb6917de84adc4f4`: both `compile-v2-runtime` and the normal V1 `build` check concluded successfully. This verifies compilation/linking of the isolated V2 support objects only.
- Compared the BK4819 bus routines against the V2 reference implementation in `muzkr/uv-k5-firmware-custom` branch `v2`. The reference uses 1 microsecond edge delays; the initial isolated bus used 250 ns delays inherited from V1 assumptions.
- Updated `driver/bk4819_bus_v2.c` to use `SYSTICK_DelayUs(1)` for bus edge timing, matching the reference timing scale. This change is awaiting a new CI run.
- The bus remains an isolated module. It has not yet replaced the DP32-specific transaction code in `driver/bk4819.c`; the full app, readback on hardware, MDC1200/FleetSync operation, display, keyboard, ADC and persistent settings are still unported/unverified. No flashable V2 firmware is claimed.


## Bus API link smoke test (2026-10-24)

- Expanded `.github/workflows/v2-cmsis-smoke.yml` so the minimal V2 entry point references `SYSTICK_Init`, `BOARD_V2_GPIO_Init`, and every public BK4819 V2 bus operation. This prevents linker garbage collection from silently excluding the bus object and gives a stronger API-level compile/link check.
- CI passed on commit `b2f9364b7ba89ddcdd34c48e0097e87580e46130`: both `compile-v2-runtime` and the independent V1 `build` job succeeded. The 1 us bus timing change and all isolated V2 support objects compile and link together.
- This is still a link smoke test, not execution on a radio. The stub entry point is never run on target; the bus is not yet wired into `driver/bk4819.c`, and the application is not a complete V2 image.


## BK4819 driver bus integration started (2026-10-24)

- Added `driver/bk4819_v2_adapter.h` as a target-selectable adapter over the verified PB3/4/5 bus and V2 SysTick timing API.
- Updated `driver/bk4819.c` so `TARGET_UVK5_V2` selects the V2 bus for register read/write and byte/word operations, and uses the V2 bus initializer. The legacy DP32 transaction path and timing remain selected when the macro is absent.
- Mapped the legacy audio-path GPIOC bit 4 semantic to the V2 reference's GPIOB7 audio-path output, while retaining the original GPIOC bit for V1 builds.
- Added a CI step that compiles `driver/bk4819.c` with `TARGET_UVK5_V2`, MDC1200 and FleetSync enabled. The workflow is currently running; its compile result has not yet been confirmed.
- This integration only addresses the BK4819 register bus and audio-path pin. The rest of the application, other DP32-dependent modules, interrupt mapping, LCD, keyboard, ADC calibration, flash settings and full image link are not ported yet. Do not flash this as a complete V2 image.


## Shared application-layer V2 adaptation (2026-10-24)

- `main.c` now selects `BOARD_V2_GPIO_Init()` and avoids the V1 DP32 clock-gate register writes when `TARGET_UVK5_V2` is defined. The original V1 path remains behind the default branch.
- The application startup's voice-chip reset is selected through a target-specific macro: V2 uses the V2 GPIO API for PA12; V1 keeps the original DP32 GPIO call.
- `app/app.c` now reads the PTT state from V2 PA3 through `GPIO_V2_Read(GPIOA)`; V1 retains its original GPIOC PTT input. DP32 GPIO/system headers are excluded from this application translation unit for V2 builds.
- `driver/system_v2.c` now provides the shared `SYSTEM_DelayMs()` API using the V2 SysTick timer, so shared application startup delays don't require the DP32 system driver.
- This is application source adaptation, not a claim that the whole app is ready to execute: EEPROM/I2C, ADC/battery readings, display SPI, keyboard matrix, audio/voice, backlight PWM, and the remaining DP32-dependent drivers still need their V2 implementations. The complete application image has not been built or tested on hardware.
