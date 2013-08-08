OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
phys = 0x100000;
SECTIONS
{
    .text phys : AT(phys) {
	code = .;
	*(.text)
	*(.rodata)
	. = ALIGN(0x1000);
    }
    .data : AT(phys + (data - code)) {
	data = .;
	*(.data)
	. = ALIGN(0x1000);
    }
    .bss : AT(phys + (bss - code)) {
	bss = .;
	*(.bss)
	. = ALIGN(0x1000);
    }
    end = .;
    /DISCARD/ : {
	*(.comment)
	*(.eh_frame)
	*(.note.gnu.build-id)
    }
}
