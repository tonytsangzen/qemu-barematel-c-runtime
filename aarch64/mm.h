#ifndef MM_H
#define MM_H

void mm_page_init(void* start, unsigned long size);
void* mm_page_alloc(unsigned int count);
void* mm_page_calloc(unsigned int  count);
void mm_page_free(void* addr, unsigned int count);

#endif
