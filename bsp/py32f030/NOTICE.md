# Vendor notices for the V2 CMSIS files

The files under `bsp/py32f030/` are kept separate from the V1 DP32G030
implementation.

## Puya / ST device support

The following files are derived from the PY32F0xx CMSIS device support in
`muzkr/uv-k5-firmware-custom`, branch `v2`:

- `py32f030x8.h`
- `py32f0xx.h`
- `system_py32f0xx.h`
- `system_v2.c` (adapted implementation)

The source files carry Puya Semiconductor and STMicroelectronics copyright
notices and are licensed under the BSD 3-Clause License. Preserve their
copyright and license notices when redistributing or modifying them.

## Arm CMSIS core support

The following files are from the CMSIS core support distributed by the same
reference repository:

- `core_cm0plus.h`
- `cmsis_version.h`
- `cmsis_compiler.h`
- `cmsis_gcc.h`
- `mpu_armv7.h`

Retain the copyright and license notices embedded in each file. Consult those
file headers and the corresponding upstream license terms before redistributing
this branch.

Reference source: https://github.com/muzkr/uv-k5-firmware-custom/tree/v2/Drivers
