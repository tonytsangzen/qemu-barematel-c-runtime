#ifndef __MMU_H__
#define __MMU_H__

#include "mmu_arch.h"

void unmap_page(page_dir_entry_t *vm, uint64_t virtual_addr);
void map_pages(page_dir_entry_t *vm, uint64_t vaddr, uint64_t pstart, uint64_t pend, uint32_t permissions, uint32_t pte_attr);
void map_pages_size(page_dir_entry_t *vm, uint64_t vaddr, uint64_t pstart, uint64_t size, uint32_t permissions, uint32_t pte_attr);
void unmap_pages(page_dir_entry_t *vm, uint64_t virtual_addr, uint64_t pages);

#endif
