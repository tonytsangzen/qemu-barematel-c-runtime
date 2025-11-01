#include <stdint.h>
#include <stdio.h>
#include "mm.h"
#include "../lib/early_printf.h"
#include "mmu_arch.h"

#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))
#define ALIGN_UP(x, a)   (((x)+(a)-1) & ~((a)-1))

/* unmap_page clears the mapping for the given virtual address */
void unmap_page(page_dir_entry_t *vm, uint64_t virtual_addr) {
	page_table_entry_t *l2_table = 0;
	page_table_entry_t *l3_table = 0;

	uint32_t l1_index = PAGE_L1_INDEX(virtual_addr);
	uint32_t l2_index = PAGE_L2_INDEX(virtual_addr);
	uint32_t l3_index = PAGE_L3_INDEX(virtual_addr);

	if(vm[l1_index].EntryType != 0){
		l2_table = (page_dir_entry_t*)P2V(vm[l1_index].Address << 12);
		if(l2_table[l2_index].EntryType != 0){
			l3_table = (page_dir_entry_t*)P2V(l2_table[l2_index].Address << 12);
			l3_table[l3_index].EntryType = 0;	
		}
	}
}

void map_pages(page_dir_entry_t *vm, uint64_t vaddr, uint64_t pstart, uint64_t pend, uint32_t permissions, uint32_t pte_attr) {
    uint64_t physical_current = 0;
    uint64_t virtual_current = 0;

    uint64_t virtual_start = ALIGN_DOWN(vaddr, PAGE_SIZE);
    uint64_t physical_start = ALIGN_DOWN(pstart, PAGE_SIZE);
    uint64_t physical_end = ALIGN_UP( pend, PAGE_SIZE);

    /* iterate over pages and map each page */
    virtual_current = virtual_start;
    for (physical_current = physical_start;
            physical_current < physical_end;
            physical_current += PAGE_SIZE) {
        map_page(vm,  virtual_current, physical_current, permissions, pte_attr);
        virtual_current += PAGE_SIZE;
    }
}

void map_pages_size(page_dir_entry_t *vm, uint64_t vaddr, uint64_t pstart, uint64_t size, uint32_t permissions, uint32_t pte_attr) {
    map_pages(vm, vaddr, pstart, pstart + size, permissions, pte_attr);
}

void unmap_pages(page_dir_entry_t *vm, uint64_t virtual_addr, uint64_t pages) {
    uint64_t i;
    for(i=0; i<pages; i++) {
        unmap_page(vm, virtual_addr + PAGE_SIZE*i);
    }
}

extern void load_boot_pgt(page_dir_entry_t *page_table);
extern uint64_t _page_tbl_start;

void mmu_init(void)
{
    mm_init(&_page_tbl_start, 16*MB);
    page_dir_entry_t *vm = page_calloc(1);
    map_pages_size(vm, 0x8000000, 0x8000000, 128*MB, 0, PTE_ATTR_DEV);
    map_pages_size(vm, 0x40000000, 0x40000000, 128*MB, 0, PTE_ATTR_DEV);
    load_boot_pgt(vm);
}
