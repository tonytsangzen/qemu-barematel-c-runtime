#include "mmu.h"
#include "mm.h"
#include "mmu_arch.h"

extern uint64_t _page_tbl_start;

void __init__(void){
	mm_page_init(&_page_tbl_start, 16*MB);
    page_dir_entry_t *vm = mm_page_calloc(1);
    map_pages_size(vm, 0x8000000, 0x8000000, 128*MB, 0, PTE_ATTR_DEV);
    map_pages_size(vm, 0x40000000, 0x40000000, 128*MB, 0, PTE_ATTR_DEV);
    set_translation_table_base(vm);
}
