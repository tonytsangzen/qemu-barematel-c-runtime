#include <stdint.h>

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define PAGE_SIZE (4 * KB)
#define PAGE_TABLE_SIZE (4 * KB)

#define PAGE_DIR_NUM 1024
#define PAGE_DIR_SIZE (PAGE_DIR_NUM * 4)

#define PDE_SHIFT 21
#define NUM_PAGE_DIRS 512
#define NUM_PAGE_TABLE_ENTRIES 4096
// support 0 - 4GB @ aarch64 mode

#define PAGE_L1_INDEX(x) (((uint64_t)x >> 30) & 0x1FF)
#define PAGE_L2_INDEX(x) (((uint64_t)x >> 21) & 0x1FF)
#define PAGE_L3_INDEX(x) (((uint64_t)x >> 12) & 0x1FF)

#define PAGE_TABLE_TO_BASE(x) ((uint32_t)x >> 10)
#define BASE_TO_PAGE_TABLE(x) ((void *)((uint32_t)x << 10))
#define PAGE_TO_BASE(x) ((uint32_t)x >> 12)

#define TYPE_INVALID 0
#define TYPE_BLOCK 1
#define TYPE_PAGE 3
#define TYPE_TABLE 3

#define MT_DEVICE_NGNRNE 0
#define MT_DEVICE_NGNRE 1
#define MT_NORMAL_NC 2
#define MT_NORMAL 3

typedef struct
{
    uint64_t EntryType : 2; // @0-1     1 for a block table, 3 for a page table
    uint64_t MemAttr : 4;   // @2-5
    enum
    {
        STAGE2_S2AP_NOREAD_EL0 = 1, //          No read access for EL0
        STAGE2_S2AP_NO_WRITE = 2,   //          No write access
    } S2AP : 2;                     // @6-7
    enum
    {
        STAGE2_SH_OUTER_SHAREABLE = 2, //          Outter shareable
        STAGE2_SH_INNER_SHAREABLE = 3, //          Inner shareable
    } SH : 2;                          // @8-9
    uint64_t AF : 1;                   // @10      Accessable flag
    uint64_t PTE_NG : 1;               // @11      no global
    uint64_t Address : 36;             // @12-47   36 Bits of address
    uint64_t _reserved48_51 : 4;       // @48-51   Set to 0
    uint64_t Contiguous : 1;           // @52      Contiguous
    uint64_t PXN : 1;                  // @53     kernel No execute if bit set
    uint64_t UXN : 1;                  // @54     user No execute if bit set
    uint64_t _reserved55_58 : 4;       // @55-58   Set to 0
    uint64_t PXNTable : 1;             // @59      Never allow execution from a lower EL level
    uint64_t XNTable : 1;              // @60      Never allow translation from a lower EL level
    enum
    {
        APTABLE_NOEFFECT = 0,         // No effect
        APTABLE_NO_EL0 = 1,           // Access at EL0 not permitted, regardless of permissions in subsequent levels of lookup
        APTABLE_NO_WRITE = 2,         // Write access not permitted, at any Exception level, regardless of permissions in subsequent levels of lookup
        APTABLE_NO_WRITE_EL0_READ = 3 // Write access not permitted, at any Exception level, Read access not permitted at EL0.
    } APTable : 2;                    // @61-62   AP Table control .. see enumerate options
    uint64_t NSTable : 1;             // @63      Secure state, for accesses from Non-secure state this bit is RES0 and is ignored
} page_table_entry_t, page_dir_entry_t;

static __attribute__((__aligned__(PAGE_DIR_SIZE)))
page_dir_entry_t startup_page_dir[NUM_PAGE_DIRS] = {0};

static __attribute__((__aligned__(PAGE_DIR_SIZE)))
page_table_entry_t startup_page_table[NUM_PAGE_TABLE_ENTRIES] = {0};

static page_table_entry_t *entry_head;

static void boot_pgt_init(void)
{
    entry_head = startup_page_table;
    for (int i = 0; i < NUM_PAGE_DIRS; i++)
    {
        startup_page_dir[i].EntryType = 0;
    }
}

static page_table_entry_t *get_free_page_table(void)
{
    if (entry_head >= &startup_page_table[NUM_PAGE_TABLE_ENTRIES])
    {
        /*no more free page table*/
        while (1)
            ;
    }

    page_table_entry_t *entry = entry_head;
    entry_head += 512;
    return entry;
}

static void set_boot_pgt(uint64_t virt, uint64_t phy, uint32_t len, int is_dev)
{
    // convert all the parameters to indexes
    page_table_entry_t *entry;
    uint32_t l1 = PAGE_L1_INDEX(virt);
    uint32_t l2 = PAGE_L2_INDEX(virt);

    if (startup_page_dir[l1].EntryType == 0)
    {
        entry = get_free_page_table();
        startup_page_dir[l1] = (page_dir_entry_t){
            .NSTable = 1,
            .EntryType = TYPE_TABLE,
            .Address = (uint64_t)entry >> 12,
            .AF = 1};
    }
    else
    {
        entry = startup_page_dir[l1].Address << 12;
    }

    phy >>= PDE_SHIFT;
    len >>= PDE_SHIFT;
    for (uint32_t idx = 0; idx < len; idx++)
    {
        // Each block descriptor (2 MB)
        entry[l2] = (page_table_entry_t){
            .NSTable = 1,
            .EntryType = TYPE_BLOCK,
            .Address = phy << (21 - 12),
            .AF = 1,
            .SH = STAGE2_SH_OUTER_SHAREABLE,
            .S2AP = 0,
            .MemAttr = is_dev ? MT_DEVICE_NGNRNE : MT_NORMAL,
        };
        l2++;
        phy++;
    }
}

extern void load_boot_pgt(uint32_t page_table);

void mmu_init(void)
{
    boot_pgt_init();
    set_boot_pgt(0x00000000, 0x00000000, 1 * GB, 1);
    set_boot_pgt(0x40000000, 0x40000000, 1 * GB, 1);
    set_boot_pgt(0x80000000, 0x80000000, 1 * GB, 1);
    set_boot_pgt(0xC0000000, 0xC0000000, 1 * GB, 1);
    load_boot_pgt(startup_page_dir);
}
