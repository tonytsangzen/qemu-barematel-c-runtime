#include <stdint.h>
#include <stddef.h>
#include "mm.h"
#include "mmu_arch.h"

typedef struct free_page_node {
    struct free_page_node *next;  // 指向下一个空闲页
} free_page_node_t;

static free_page_node_t *free_list = NULL;  // 空闲页链表头
static void *mem_start = NULL;              // 内存起始地址
static unsigned long mem_size = 0;          // 内存总大小
static unsigned long total_pages = 0;       // 总页数
static unsigned long used_pages = 0;        // 已使用的页数

/**
 * @brief 初始化内存管理器
 * @param start 可用内存的起始地址
 * @param size 可用内存的大小（字节）
 */
void mm_init(void* start, unsigned long size) {
    total_pages = size / PAGE_SIZE;
    used_pages = 0;
    
    // 初始化空闲链表
    free_list = NULL;
    
    // 将所有可用内存页加入空闲链表
    uint64_t addr = (uint64_t)start;
    for (unsigned long i = 0; i < total_pages; i++) {
        free_page_node_t *page = (free_page_node_t *)addr;
        page->next = free_list;
        free_list = page;
        addr += PAGE_SIZE;
    }
}

/**
 * @brief 分配指定数量的页面
 * @param count 需要分配的页数
 * @return 成功返回页面起始地址，失败返回NULL
 */
void* page_alloc(int count) {
    if (count <= 0) {
        return NULL;
    }
    
    // 检查是否有足够的空闲页
    int available = 0;
    free_page_node_t *current = free_list;
    free_page_node_t *prev = NULL;
    free_page_node_t *start_block = free_list;
    
    // 扫描空闲链表，寻找足够大的连续块
    while (current != NULL) {
        // 检查当前页是否与前一页连续
        if (prev != NULL && (uint64_t)current != (uint64_t)prev + PAGE_SIZE) {
            // 不连续，重置计数和起始位置
            available = 0;
            start_block = current;
        }
        
        available++;
        
        // 找到足够大的连续块
        if (available == count) {
            // 从空闲链表中移除这些页
            if (start_block == free_list) {
                // 从头开始移除
                free_list = current->next;
            } else {
                // 从中间移除，需要找到start_block的前一个节点
                free_page_node_t *temp = free_list;
                while (temp->next != start_block) {
                    temp = temp->next;
                }
                temp->next = current->next;
            }
            
            // 断开分配的块
            current->next = NULL;
            used_pages += count;
            return start_block;
        }
        
        prev = current;
        current = current->next;
    }
    
    // 没有找到足够的连续页
    return NULL;
}

void* page_calloc(int count) {
    void* addr = page_alloc(count);
    if (addr != NULL) {
        for(int i=0; i<count * PAGE_SIZE / 4; i++) {
            ((uint32_t*)addr)[i] = 0;
        }
    }
    return addr;
}

/**
 * @brief 释放之前分配的页面
 * @param addr 要释放的页面起始地址
 * @param count 要释放的页面数量
 */
void page_free(void* addr, int count) {
    if (addr == NULL || count <= 0) {
        return;
    }
    
    // 验证地址是否在有效的内存范围内且对齐到页边界
    if (((uint64_t)addr < (uint64_t)mem_start) || 
        ((uint64_t)addr + (count - 1) * PAGE_SIZE >= (uint64_t)mem_start + mem_size) ||
        ((uint64_t)addr & (PAGE_SIZE - 1)) != 0) {
        return;  // 无效地址或超出内存范围
    }
    
    // 将所有要释放的页面添加到空闲链表
    uint64_t current_addr = (uint64_t)addr;
    free_page_node_t *first_page = NULL;
    free_page_node_t *last_page = NULL;
    
    for (int i = 0; i < count; i++) {
        free_page_node_t *page = (free_page_node_t *)current_addr;
        
        if (i == 0) {
            first_page = page;
        } else {
            last_page->next = page;
        }
        
        last_page = page;
        current_addr += PAGE_SIZE;
    }
    
    // 确保最后一个页面的next指针为NULL
    last_page->next = NULL;
    
    // 将整个释放的块添加到空闲链表头部
    first_page->next = free_list;
    free_list = first_page;
    
    // 更新已使用页面计数
    used_pages -= count;
}