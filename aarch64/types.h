#ifndef __TYPE_H__
#define __TYPE_H__

#define NULL (0)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef uint64_t uintptr_t;

#define V2P(x) ((uint64_t)(x))
#define P2V(x) ((uint64_t)(x))

#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))
#define ALIGN_UP(x, a)   (((x)+(a)-1) & ~((a)-1))

#define KB 1024
#define MB (1024*KB)
#define GB (1024*MB)

#define PAGE_SIZE (4*KB)

#endif
