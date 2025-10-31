#include <stdint-gcc.h>

// 页目录和页表项的定义
#define PAGE_PRESENT     (1 << 0)
#define PAGE_WRITE       (1 << 1)
#define PAGE_USER        (1 << 2)
#define PAGE_WRITETHROUGH (1 << 3)
#define PAGE_CACHE_DISABLE (1 << 4)
#define PAGE_ACCESSED    (1 << 5)
#define PAGE_DIRTY       (1 << 6)
#define PAGE_SIZE_4MB    (1 << 7)
#define PAGE_GLOBAL      (1 << 8)

// 定义页目录和页表
#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024

// 页目录和页表对齐到4KB边界
uint32_t page_directory[PAGE_DIR_SIZE] __attribute__((aligned(4096)));
uint32_t page_tables[PAGE_DIR_SIZE][PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

// 初始化MMU并设置线性映射
void mmu_init() {
    // 1. 初始化页目录
    for (int i = 0; i < PAGE_DIR_SIZE; i++) {
        // 设置页目录项指向页表
        // 页表物理地址 = (uint32_t)&page_tables[i]
        // 设置标志位：存在、可写、用户可访问
        page_directory[i] = ((uint32_t)&page_tables[i]) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    // 2. 初始化所有页表，建立线性映射
    for (int i = 0; i < PAGE_DIR_SIZE; i++) {
        for (int j = 0; j < PAGE_TABLE_SIZE; j++) {
            // 计算物理地址 = (i * 1024 + j) * 4096
            uint32_t physical_addr = (i * PAGE_TABLE_SIZE + j) * 4096;
            
            // 设置页表项：物理地址 + 标志位
            // 使用4KB页，设置存在、可写、用户可访问
            page_tables[i][j] = physical_addr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        }
    }
    
    // 3. 设置CR3寄存器（页目录基址）
    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r" ((uint32_t)page_directory)
    );
    
    // 4. 启用分页
    uint32_t cr0;
    __asm__ volatile (
        "mov %%cr0, %0"
        : "=r" (cr0)
    );
    cr0 |= 0x80000000;  // 设置PG位（分页使能）
    __asm__ volatile (
        "mov %0, %%cr0"
        :
        : "r" (cr0)
    );
}

// 可选：使用4MB大页的简化版本
void init_mmu_4mb() {
    // 直接使用页目录项映射4MB页
    for (int i = 0; i < PAGE_DIR_SIZE; i++) {
        // 每个页目录项映射4MB
        uint32_t physical_addr = i * 0x400000;  // 4MB对齐
        page_directory[i] = physical_addr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER | PAGE_SIZE_4MB;
    }
    
    // 设置CR3寄存器
    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r" ((uint32_t)page_directory)
    );
    
    // 启用分页
    uint32_t cr0;
    __asm__ volatile (
        "mov %%cr0, %0"
        : "=r" (cr0)
    );
    cr0 |= 0x80000000;
    __asm__ volatile (
        "mov %0, %%cr0"
        :
        : "r" (cr0)
    );
}

