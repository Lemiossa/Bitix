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

	.bss : {
		*(.COMMON*)
		*(.bss*)
	}
}
