ENTRY(_start)
OUTPUT_FORMAT(binary)

SECTIONS {
  . = 0x500;

  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss COMMON) }

  _end = .;
}
