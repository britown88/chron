#pragma once

#include <malloc.h>

#include "Defs.h"

void* checkedMallocImpl(u32 sz, char *func, char* file, u32 line);
void* checkedCallocImpl(u32 count, u32 sz, char *func, char* file, u32 line);
void checkedFreeImpl(void* mem);
void printMemoryLeaks();


#ifdef _DEBUG
#define checkedMalloc(sz) checkedMallocImpl(sz, __FUNCTION__, __FILE__, __LINE__)
#define checkedCalloc(count, sz) checkedCallocImpl(count, sz, __FUNCTION__, __FILE__, __LINE__)
#define checkedFree(sz) checkedFreeImpl(sz)
#else
#define checkedMalloc(sz) malloc(sz)
#define checkedCalloc(count, sz) calloc(count, sz)
#define checkedFree(sz) free(sz)
#endif


 
