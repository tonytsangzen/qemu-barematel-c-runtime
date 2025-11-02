#include "mm.h"
#include "mmu_arch.h"
#include "mmu.h"

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
