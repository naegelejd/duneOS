#include "mem.h"
#include "bget.h"
#include "string.h"


enum {
    PAGE_POWER = 12,
    PAGE_SIZE = (unsigned)(1 << PAGE_POWER),
    PAGE_MASK = (~(PAGE_SIZE - 1))
};

enum {
    PAGE_AVAIL  = 0x1,  /* page on freelist */
    PAGE_KERN   = 0x2,  /* page used by kernel */
    PAGE_HDWARE = 0x4,  /* page used by hardware (ISA hole) */
    PAGE_ALLOC  = 0x8,  /* page allocated */
    PAGE_UNUSED = 0x10, /* page unused */
    PAGE_HEAP   = 0x20  /* page in kernel heap */
};

enum {
    HDWARE_RAM_START = 0x9F000, /* 0x9FC00 page aligned */
    HDWARE_RAM_END   = 0x100000 /* end of Video memory */
};

enum {
    KERNEL_HEAP_SIZE = 0x100000    /* 1M heap */
};

struct page {
    uint32_t flags;
    struct page *next;
};
typedef struct page page_t;

page_t* g_page_list;

static page_t* g_free_page_head;
static page_t* g_free_page_tail;
static unsigned int g_free_page_count;

/*
 * Determine if given address is a multiple of the page size.
 */
static inline bool is_page_aligned(uintptr_t addr)
{
    return addr == (addr & PAGE_MASK);
}

static uintptr_t page_align_up(uintptr_t addr)
{
    /* only align forward a page if not already aligned */
    /* so for 4K pages, least significant 3 bytes are not zero */
    if ((addr & (~PAGE_MASK)) != 0) {
        addr &= PAGE_MASK;
        addr += PAGE_SIZE;
    }
    return addr;
}

static uintptr_t page_align_down(uintptr_t addr)
{
    return addr & PAGE_MASK;
}

static unsigned int page_index(uintptr_t addr)
{
    return addr >> PAGE_POWER;
}

static page_t* page_from_addr(uintptr_t addr)
{
    return &g_page_list[page_index(addr)];
}

static uintptr_t addr_from_page(page_t *page)
{
    unsigned int index = page - g_page_list;
    return index << PAGE_POWER;
}

static void mark_page_range(uintptr_t start, uintptr_t end, uint32_t flags)
{
    char *flagname;
    switch (flags) {
        case PAGE_AVAIL:
            flagname = "AVAIL";
            break;
        case PAGE_KERN:
            flagname = "KERN";
            break;
        case PAGE_HDWARE:
            flagname = "HDWARE";
            break;
        case PAGE_ALLOC:
            flagname = "ALLOC";
            break;
        case PAGE_UNUSED:
            flagname = "USED";
            break;
        case PAGE_HEAP:
            flagname = "HEAP";
            break;
        default:
            flagname = "BAD FLAG";
    }
    kprintf("Add range: 0x%x - 0x%x - %s\n", start, end, flagname);
    KASSERT(is_page_aligned(start));
    KASSERT(is_page_aligned(end));
    KASSERT(start < end);

    uintptr_t addr;
    for (addr = start; addr < end; addr += PAGE_SIZE) {
        page_t* page = page_from_addr(addr);
        page->flags = flags;

        if (flags & PAGE_AVAIL) {
            /* TODO: append to freelist */
            g_free_page_count++;
        } else {
            page->next = NULL;
        }
    }
}


void mem_init(struct multiboot_info *mbinfo)
{
    extern char g_start, g_end;

    uintptr_t kernel_start = (uintptr_t)&g_start;
    uintptr_t kernel_end = (uintptr_t)&g_end;

    /* for now, require valid memory limits in multiboot info */
    KASSERT(mbinfo->flags & MULTIBOOT_INFO_MEMORY);
    uint32_t mem_upper = mbinfo->mem_upper * 1024;   /* mem_upper is in KB */
    kprintf("Mem low: 0x%x, Mem high: 0x%x\n", mbinfo->mem_lower * 1024, mem_upper);

    uint32_t num_pages = mem_upper / PAGE_SIZE;
    uintptr_t end_of_memory = num_pages * PAGE_SIZE;
    kprintf("Number of pages: %u\n", num_pages);

    /* align kernel_start down a page because technically the multiboot
     * header sits in front of the kernel's entry point */
    kernel_start = page_align_down(kernel_start);

    /* account for the size of the struct page array */
    uint32_t page_list_bytes = num_pages * sizeof(page_t);
    kernel_end = page_align_up(kernel_end + page_list_bytes);

    uintptr_t heap_start = kernel_end;
    uintptr_t heap_end = kernel_end + KERNEL_HEAP_SIZE;

    /* mark first page as unused */
    mark_page_range(0, PAGE_SIZE, PAGE_UNUSED);                      /* unused first page */
    mark_page_range(PAGE_SIZE, HDWARE_RAM_START, PAGE_AVAIL);        /* extra RAM */
    mark_page_range(HDWARE_RAM_START, kernel_start, PAGE_HDWARE);    /* Extended BIOS and Video RAM */
    mark_page_range(kernel_start, kernel_end, PAGE_KERN);            /* kernel pages */
    mark_page_range(kernel_end, heap_end, PAGE_HEAP);                /* heap pages */
    mark_page_range(heap_end, end_of_memory, PAGE_AVAIL);            /* available RAM */

/*
    if (mbinfo->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t*)mbinfo->mmap_addr;
        KASSERT(mbinfo->mmap_length > 0);   // assume memory map has > 1 entry

        uintptr_t range_start = 0, range_end = PAGE_SIZE;
        while ((uintptr_t)mmap < mbinfo->mmap_addr + mbinfo->mmap_length) {
            kprintf("Base Addr: 0x%x, Length: 0x%x, %s\n", (uint32_t)mmap->addr, (uint32_t)mmap->len,
                    (mmap->type == 1 ? "free" : "used"));

            uint32_t type = mmap->type;
            if (type == MULTIBOOT_MEMORY_AVAILABLE) {
                range_start = page_align_up(mmap->addr);
                range_end = page_align_down(mmap->addr + mmap->len);
                mark_page_range(range_start, range_end, PAGE_AVAIL);
            } else {
                range_start = page_align_down(mmap->addr);
                range_end = page_align_up(mmap->addr + mmap->len);
                mark_page_range(range_start, range_end, PAGE_HDWARE);
            }

            mmap = (multiboot_memory_map_t*) ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
        }
    }
*/
    /* initialize the kernel's heap */
    kprintf("Creating kernel heap: start=0x%x, size=0x%x\n", heap_start, KERNEL_HEAP_SIZE);
    bpool((void*) heap_start, KERNEL_HEAP_SIZE);
}

void* alloc_page(void)
{
    void* addr = NULL;

    bool iflag = beg_int_atomic();

    if (g_free_page_count > 0) {
        /* TODO: remove page from free page list */
        page_t* page = g_free_page_head;
        g_free_page_head = page->next;
        KASSERT(page->flags & (~PAGE_ALLOC));
        page->flags |= PAGE_ALLOC;
        g_free_page_count--;
        addr = (void*)addr_from_page(page);
    }

    end_int_atomic(iflag);

    return addr;
}

void free_page(void* page_addr)
{
    uintptr_t addr = (uintptr_t)page_addr;
    KASSERT(is_page_aligned(addr));

    bool iflag = beg_int_atomic();

    page_t* page = page_from_addr(addr);
    KASSERT(page->flags & PAGE_ALLOC);

    page->flags &= ~PAGE_ALLOC;

    /* TODO: append to page freelist */
    g_free_page_count++;

    end_int_atomic(iflag);
}

void bss_init(void)
{
    extern char g_bss, g_end;

    uintptr_t bss_start = (uintptr_t)&g_bss;
    uintptr_t bss_end = (uintptr_t)&g_end;

    /* zero BSS section */
    kprintf("BSS: 0x%x: %u\n", bss_start, bss_end - bss_start);
    memset((void*)bss_start, 0, bss_end - bss_start);
}

void* malloc(size_t size)
{
    void *buffer = NULL;
    bool iflag;

    KASSERT(size > 0);

    iflag = beg_int_atomic();
    buffer = bget(size);
    end_int_atomic(iflag);

    return buffer;
}

void free(void *buffer)
{
    bool iflag;

    iflag = beg_int_atomic();
    brel(buffer);
    end_int_atomic(iflag);
}
