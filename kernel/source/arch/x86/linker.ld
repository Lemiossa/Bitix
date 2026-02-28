ENTRY(_start)
__start = 0x100000;

SECTIONS
{
	. = __start;

	__text_start = .;
	.text : {
		*(.text*)
	}
	__text_end = .;

	__data_start = .;
	.data : {
		*(.data*)
	}
	__data_end = .;

	__bss_start = .;
	.bss : {
		*(.COMMON*)
		*(.bss*)
	}
	__bss_end = .;
	__end = .;
}
