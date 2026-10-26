#include "types.h"
#include "mm.h"
#include "mmu_arch.h"

#define PAGE_DIR_NUM 512
#define PAGE_DIR_SIZE (PAGE_DIR_NUM*8)

#define PAGE_L1_INDEX(x) (((uint64_t)x >> 30) & 0x1FF) 
#define PAGE_L2_INDEX(x) (((uint64_t)x >> 21) & 0x1FF)
#define PAGE_L3_INDEX(x) (((uint64_t)x >> 12) & 0x1FF)

#define TYPE_ARCH_INVALID		0
#define TYPE_ARCH_BLOCK			1
#define TYPE_ARCH_PAGE			3
#define TYPE_ARCH_TABLE			3

#define MT_ARCH_NORMAL			 0
#define MT_ARCH_NORMAL_TAGGED    1
#define MT_ARCH_NORMAL_NC        2
#define MT_ARCH_DEVICE_nGnRnE    3
#define MT_ARCH_DEVICE_nGnRE     4

/*
 * map_page adds to the given virtual memory the mapping of a single virtual page
 * to a physical page.
 * Notice: virtual and physical address inputed must be all aliend by PAGE_SIZE !
 */
int map_page(page_dir_entry_t *vm, uint64_t virtual_addr,
		     uint64_t physical, uint32_t permissions, uint32_t pte_attr) {
	page_table_entry_t *l2_table = 0;
	page_table_entry_t *l3_table = 0;

	uint32_t l1_index = PAGE_L1_INDEX(virtual_addr);
	uint32_t l2_index = PAGE_L2_INDEX(virtual_addr);
	uint32_t l3_index = PAGE_L3_INDEX(virtual_addr);
	/* if this page_dirEntry is not mapped before, map it to a new page table */
	if (vm[l1_index].EntryType == 0) {
		l2_table = mm_page_calloc(1);
		if(l2_table == NULL)
			return -1;
		vm[l1_index].NSTable = 1;
		vm[l1_index].EntryType = TYPE_ARCH_TABLE;
		vm[l1_index].Address = (uint64_t)V2P(l2_table) >> 12;	
		vm[l1_index].AF = 1;
	}else{
		l2_table = P2V(vm[l1_index].Address << 12); 
	}

	if(l2_table[l2_index].EntryType == 0){
		l3_table = mm_page_calloc(1);	
		if(l3_table == NULL)
			return -1;	
		l2_table[l2_index].NSTable = 1;
		l2_table[l2_index].EntryType = TYPE_ARCH_TABLE;
		l2_table[l2_index].Address = (uint64_t)V2P(l3_table) >> 12;	
		l2_table[l2_index].AF = 1;
	}else{
		l3_table = P2V(l2_table[l2_index].Address << 12); 	
	}

	l3_table[l3_index].NSTable = 1;
	l3_table[l3_index].EntryType = TYPE_ARCH_PAGE;
	l3_table[l3_index].AF = 1;
	l3_table[l3_index].SH = STAGE2_SH_OUTER_SHAREABLE;
	l3_table[l3_index].S2AP = permissions;
	l3_table[l3_index].MemAttr = (pte_attr == PTE_ATTR_DEV) ? MT_ARCH_DEVICE_nGnRnE : MT_ARCH_NORMAL;
	l3_table[l3_index].Address = (uint64_t)physical >> 12; 
	return 0;
}

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

/*
 * resolve_physical_address simulates the virtual memory hardware and maps the
 * given virtual address to physical address. This function can be used for
 * debugging if given virtual memory is constructed correctly.
 */
uint64_t resolve_phy_address(page_dir_entry_t *vm, uint64_t virtual) {
	page_table_entry_t *l2_table = 0;
	page_table_entry_t *l3_table = 0;
	uint32_t l1_index = PAGE_L1_INDEX(virtual);
	uint32_t l2_index = PAGE_L2_INDEX(virtual);
	uint32_t l3_index = PAGE_L3_INDEX(virtual);

	l2_table = (page_table_entry_t*)P2V(vm[l1_index].Address << 12); 
	l3_table = (page_table_entry_t*)P2V(l2_table[l2_index].Address << 12); 
	uint32_t phy = (l3_table[l3_index].Address << 12) | (virtual & 0xFFF);	
	return phy;
}

uint64_t resolve_virt_address(page_dir_entry_t *vm, uint64_t phys) {
	page_table_entry_t *l2_table = 0;
	page_table_entry_t *l3_table = 0;

	for(int i = 0; i < PAGE_DIR_NUM; i++){
		if(vm[i].EntryType != 0){
			l2_table = (page_table_entry_t*)P2V(vm[i].Address << 12); 
			for(int j = 0; j < PAGE_DIR_NUM; j++){
				if(l2_table[j].EntryType != 0){
					l3_table = (page_table_entry_t*)P2V(l2_table[j].Address << 12);
					mm_page_free(l3_table, 1);
				}
			}
			mm_page_free(l2_table, 1);
		}
	}

}

/*
get page entry(virtual addr) by virtual address
*/
page_table_entry_t* get_page_table_entry(page_dir_entry_t *vm, uint64_t virtual) {
	page_table_entry_t *l2_table = 0;

	uint32_t l1_index = PAGE_L1_INDEX(virtual);
	uint32_t l2_index = PAGE_L2_INDEX(virtual);

	l2_table = (page_table_entry_t*)P2V(vm[l1_index].Address << 12); 
	return (page_table_entry_t*)P2V(l2_table[l2_index].Address << 12); 	
}


void free_page_tables(page_dir_entry_t *vm) {
	page_table_entry_t *l2_table = 0;
	page_table_entry_t *l3_table = 0;

	for(int i = 0; i < PAGE_DIR_NUM; i++){
		if(vm[i].EntryType != 0){
			l2_table = (page_table_entry_t*)P2V(vm[i].Address << 12); 
			for(int j = 0; j < PAGE_DIR_NUM; j++){
				if(l2_table[j].EntryType != 0){
					l3_table = (page_table_entry_t*)P2V(l2_table[j].Address << 12);
					mm_page_free(l3_table, 1);
				}
			}
			mm_page_free(l2_table, 1);
		}
	}
}

#define MAIR1VAL ((0x00ul << (0 * 8)) | \
                  (0x04ul << (1 * 8)) | \
                  (0x44ul << (2 * 8)) | \
                  (0xfful << (3 * 8)))

#define TCREL1VAL ((0x0ul << 37) | /* TBI=0, no tagging */ \
                   (0x2ul << 32) | /* IPS= 32 bit */ \
                   (0x2ul << 30) | /* TG1=4k */ \
                   (0x3ul << 28) | /* SH1=3 inner */ \
                   (0x1ul << 26) | /* ORGN1=1 write back */ \
                   (0x1ul << 24) | /* IRGN1=1 write back */ \
                   (0x0ul << 23) | /* EPD1=0 walk */ \
                   (0x2UL << 16) | /* T1SZ=2 */ \
                   (0x0ul << 14) | /* TG0=4k */ \
                   (0x3ul << 12) | /* SH0=3 inner */ \
                   (0x1ul << 10) | /* ORGN0=1 write back */ \
                   (0x1ul << 8)  | /* IRGN0=1 write back */ \
                   (0x0ul << 7) |  /* EPD0=0 walk */ \
                   (25ul  << 0))   /* T0SZ=25 */

#define SCTLREL1VAL ((0xC00800) | \
                     (1 << 12) | /* I, Instruction cache enable */ \
                     (0 << 4)  | /* SA0, Stack Alignment Check Enable for EL0 */ \
                     (0 << 3)  | /* SA, Stack Alignment Check Enable */ \
                     (1 << 2)  | /* C, Data cache enable */ \
                     (0 << 1)  | /* A, Alignment check enable */ \
                     (1 << 0))   /* M, enable MMU */

void set_translation_table_base(uint64_t table)
{
    uint64_t reg_val;

    // 设置 MAIR_EL1
    reg_val = MAIR1VAL;
    __asm__ volatile("msr mair_el1, %0" : : "r" (reg_val));

    // 设置 TTBR0_EL1
    __asm__ volatile("msr ttbr0_el1, %0" : : "r" (table));
    __asm__ volatile("msr ttbr1_el1, %0" : : "r" (table));
    __asm__ volatile("isb");

    // 设置 TCR_EL1
    reg_val = TCREL1VAL;
    __asm__ volatile("msr tcr_el1, %0" : : "r" (reg_val));
    __asm__ volatile("isb sy");
    __asm__ volatile("dsb sy");

    // 设置 SCTLR_EL1
    __asm__ volatile("mrs %0, sctlr_el1" : "=r" (reg_val));
    reg_val |= SCTLREL1VAL;
    __asm__ volatile("msr sctlr_el1, %0" : : "r" (reg_val));

    __asm__ volatile("dsb sy");
    __asm__ volatile("isb sy");
}

void flush_cache(void *start, void *end) {
    uintptr_t addr = (uintptr_t)start;
    uintptr_t end_addr = (uintptr_t)end;
    
    addr &= ~(64 - 1);

    for (; addr < end_addr; addr += 64) {
        __asm volatile("dc civac, %0" :: "r"(addr));
    }

    __asm volatile("ic iallu");
    __asm volatile("dsb sy");
    __asm volatile("isb");      
}
