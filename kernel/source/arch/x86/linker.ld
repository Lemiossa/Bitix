ENTRY(_start)
__load_addr = 0x10000;

SECTIONS
{
	. = __load_addr;
	.text : {
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
