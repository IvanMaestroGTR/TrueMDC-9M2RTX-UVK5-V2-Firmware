# PY32F030 system initialization note

Added `driver/system_v2.c` in commit `18416dce6eb4ddb89bf0d2129905468850386264`.

It is isolated from the V1 build and provides the PY32 CMSIS system-clock
variables, clock-frequency update function, and system initialization. The
vector table offset is set to `0x1000`, matching the application origin in
`firmware_v2.ld`.

This file is not yet compiled: the matching `py32f0xx.h` device/CMSIS headers
are not yet present in this branch. The V2 target is not ready to build or
flash. The next blockers are the DP32-specific clock/pin initialization in
`main.c` and the register-level GPIO, SPI, I2C, ADC, UART/DMA, timer, RTC, and
flash drivers. The default V1 target remains unchanged.
