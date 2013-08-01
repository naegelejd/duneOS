OUTPUT_FORMAT("binary")
ENTRY(main)
SECTIONS
{
  .text  0x1000 : {
    code = .; _code = .; __code = .;
    *(.text)
    . = ALIGN(0x1000);
  }
  .data  : {
    data = .; _data = .; __data = .;
    *(.data)
    . = ALIGN(0x1000);
  }
  .bss  : {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(0x1000);
  }
  end = .; _end = .; __end = .;
}
