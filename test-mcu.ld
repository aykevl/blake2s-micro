/*
  This linker script is here to test code size for Cortex MCUs. It is not
  for a real microcontroller.
*/

MEMORY
{
  FLASH_TEXT (rx):  ORIGIN = 0,          LENGTH = 4K
  RAM        (rwx): ORIGIN = 0x20000000, LENGTH = 4K
}

SECTIONS
{
  .text :
  {
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
  } >FLASH_TEXT

  .data :
  {
    *(.data)
    *(.data*)
  } >RAM AT>FLASH_TEXT
}
