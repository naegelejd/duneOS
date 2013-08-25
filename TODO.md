TODO
====

Implement
---------

- Virtual Filesystem

- Ramdisk (initrd)

- Ext2 filesystem or similar

- Paging

- Elf parsing (and relocation)

- User processes

- System calls


Fix
---

- Implement a real scheduling algorithm (currently FIFO)

- Use the APIC (cpu-local timer) in one-shot mode to wake up
  sleeping threads, rather than walking the entire sleep-queue on each all to `schedule()`

- Implement malloc/free myself rather than use `bget`
  (I could easily port a simple malloc implementation I wrote not too long ago)

- Calibrate the PIT before using it.

- Assess the impact of drift when using the PIT (at 100Hz)

- Verify that I'm delaying long enough when reading/writing
  keyboard and CMOS ports...
  (I only read when interrupts are triggered, so...)


