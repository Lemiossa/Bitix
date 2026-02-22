ENTRY(_start)
__load_addr = 0x8000;

SECTIONS
{
	. = __load_addr;
	.text : {
		*(.entry)
		*(.text*)
	}

	.data : {
		*(.data*)
	}

	__bss_start = .;
	.bss : {
		*(.COMMON*)
		*(.bss*)
	}
	__bss_end = .;
}
