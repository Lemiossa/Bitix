ENTRY(_start)
__start = 0x00000500;

SECTIONS
{
	. = __start;

	__text_start = .;
	.text : {
		*(.entry)
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

