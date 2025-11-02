#include "types.h"
#include "mmu_arch.h"
#include "mm.h"

typedef struct free_page_node {
    struct free_page_node *next;  
} mm_free_page_t;

static mm_free_page_t *_free_list = NULL;
static void *_mem_start = NULL;            
static unsigned long _mem_size = 0;        
static unsigned long _total_pages = 0;     
static unsigned long _used_pages = 0;      

void mm_page_init(void* start, unsigned long size) {
    _total_pages = size / PAGE_SIZE;
    _used_pages = 0;
    
    _free_list = NULL;
    
    uint64_t addr = (uint64_t)start;
    for (unsigned long i = 0; i < _total_pages; i++) {
        mm_free_page_t *page = (mm_free_page_t *)addr;
        page->next = _free_list;
        _free_list = page;
        addr += PAGE_SIZE;
    }
}

void* mm_page_alloc(unsigned int count) {
    unsigned int available = 0;
    mm_free_page_t *current_page = _free_list;
    mm_free_page_t *prev_page = NULL;
    mm_free_page_t *start_block = _free_list;
    
    while (current_page != NULL) {
        if (prev_page != NULL && (uint64_t)current_page != (uint64_t)prev_page + PAGE_SIZE) {
            available = 0;
            start_block = current_page;
        }
        
        available++;
        
        if (available == count) {
            if (start_block == _free_list) {
                _free_list = current_page->next;
            } else {
                mm_free_page_t *temp = _free_list;
                while (temp->next != start_block) {
                    temp = temp->next;
                }
                temp->next = current_page->next;
            }
            
            current_page->next = NULL;
            _used_pages += count;
            return start_block;
        }
        
        prev_page = current_page;
        current_page = current_page->next;
    }
    
    return NULL;
}

void* mm_page_calloc(unsigned int count) {
    void* addr = mm_page_alloc(count);
    if (addr != NULL) {
        for(unsigned int i=0; i<count * PAGE_SIZE / 4; i++) {
            ((uint32_t*)addr)[i] = 0;
        }
    }
    return addr;
}

void mm_page_free(void* addr, unsigned int count) {
    if (addr == NULL) {
        return;
    }
    
    if (((uint64_t)addr < (uint64_t)_mem_start) || 
        ((uint64_t)addr + (count - 1) * PAGE_SIZE >= (uint64_t)_mem_start + _mem_size) ||
        ((uint64_t)addr & (PAGE_SIZE - 1)) != 0) {
        return;  
    }
    
    uint64_t current_addr = (uint64_t)addr;
    mm_free_page_t *first_page = NULL;
    mm_free_page_t *last_page = NULL;
    
    for (unsigned int i = 0; i < count; i++) {
        mm_free_page_t *page = (mm_free_page_t *)current_addr;
        
        if (i == 0) {
            first_page = page;
        } else {
            last_page->next = page;
        }
        
        last_page = page;
        current_addr += PAGE_SIZE;
    }
    
    last_page->next = NULL;
    
    first_page->next = _free_list;
    _free_list = first_page;
    
    _used_pages -= count;
}
