OUTPUT_FORMAT(elf32-i386)
ENTRY(g_start)
phys = 0x100000;
SECTIONS
{
    .text phys : AT(phys) {
	g_code = .;
	*(.text)
	*(.rodata)
	. = ALIGN(0x1000);
    }
    .data : AT(phys + (g_data - g_code)) {
	g_data = .;
	*(.data)
	. = ALIGN(0x1000);
    }
    .bss : AT(phys + (g_bss - g_code)) {
	g_bss = .;
	*(.bss)
	. = ALIGN(0x1000);
    }
    g_end = .;
    /DISCARD/ : {
	*(.comment)
	*(.eh_frame)
	*(.note.gnu.build-id)
    }
}
