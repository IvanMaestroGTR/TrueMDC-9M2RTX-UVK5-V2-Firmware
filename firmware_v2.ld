/* TrueMDC UV-K5 V2 linker layout.
 *
 * Separate from firmware.ld: this file is not selected by the V1 build.
 * Use only with the matching PY32F030 startup/runtime path. The PY32F030 V2
 * board has 8 KiB RAM and its application begins at 0x08001000.
 *
 * Memory layout reference: muzkr/uv-k5-firmware-custom, branch v2,
 * Core/py32f030x8.ld. See V2_HARDWARE_INVENTORY.md.
 */

ENTRY(HandlerReset)

_estack = 0x20002000;       /* end of 8 KiB RAM */
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x400;

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08001000, LENGTH = 60K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 8K
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.text.isr))
        KEEP(*(.isr_vector))
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    . = ALIGN(4);
    flash_data_start = .;

    .data :
    {
        . = ALIGN(4);
        sram_data_start = .;
        *(.sramtext)
        *(.srambss)
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    sram_data_end = .;

    . = ALIGN(4);
    .bss :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    ._user_heap_stack :
    {
        . = ALIGN(8);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM

    /DISCARD/ :
    {
        *(.note.GNU-stack)
    }
}
