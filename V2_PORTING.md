# UV-K5 V2 hardware port

## Goal

Port TrueMDC to the UV-K5 V2 hardware while retaining TrueMDC as the source tree and behavioural reference. The V2 firmware from muzkr is a hardware reference only; it is not the application base for this port.

## Reference repositories

- TrueMDC source: https://github.com/IvanMaestroGTR/TrueMDC-9M2RTX-Losehu-UVK5-Firmware
- V2 hardware reference: https://github.com/muzkr/uv-k5-firmware-custom/tree/v2

## Compatibility findings

- TrueMDC currently documents support for V1 hardware only.
- The V2 radio uses a different MCU family from the original V1 DP32G030 target. The V2 port must therefore validate MCU peripheral/register access and flash programming assumptions, not merely change a build label.
- The startup vector layout and linker memory map inspected so far appear structurally similar between the two repositories. They must still be verified against the V2 MCU datasheet and the reference implementation before being reused.
- The V2 reference repository uses a different project/build layout from this TrueMDC repository. Its application code should not replace TrueMDC's application.

## Porting rules

1. Preserve TrueMDC application behaviour and user-facing features.
2. Adapt only MCU, board, peripheral, interrupt, clock, flash/EEPROM, and build integration that must change for V2.
3. Do not remove MDC1200, FleetSync, PTT-ID, contact aliases, preamble controls, TPT, CHIRP settings, or existing TrueMDC UI behaviour as a shortcut.
4. Do not call the port complete until a V2-targeted build succeeds and the binary has been checked for the correct MCU/flash layout.
5. Treat successful compilation as necessary but not sufficient: radio bring-up and on-device checks are still required.

## Work status

This document records the initial porting boundary only. It does not claim that V2 support has been implemented or that a V2 firmware image has been built. Next work is to map TrueMDC's low-level MCU/board dependencies against the V2 reference, then make and compile the smallest hardware-layer changes first.
