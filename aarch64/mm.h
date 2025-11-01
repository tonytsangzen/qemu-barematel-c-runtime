#ifndef MM_H
#define MM_H

void mm_init(void* start, unsigned long size);
void* page_alloc(int count);
void* page_calloc(int count);
void page_free(void* addr, int count);

#endif