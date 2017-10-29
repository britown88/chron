#include "Preprocessor.h"
#include <string.h>

#define T VectorTPart
#define VEC_NAME CONCAT(vec_, T)

typedef struct VEC_NAME VEC_NAME;

#include "Vector_Functions.h"

VEC_NAME *vecCreate(T)(void(*destroy)(T*));
void vecDestroy(T)(VEC_NAME *self);
void vecResize(T)(VEC_NAME *self, u32 size, T *initialValue);
void vecPushBack(T)(VEC_NAME *self, T *data);
void vecPushArray(T)(VEC_NAME *self, T*arr, u32 count);
VEC_NAME *vecInitArray(T)(void(*destroy)(T*), T *arr, u32 count);
void vecPopBack(T)(VEC_NAME *self);
void vecInsert(T)(VEC_NAME *self, u32 pos, T*data);
T *vecAt(T)(VEC_NAME *self, u32 index);
i32 vecIsEmpty(T)(VEC_NAME *self);
u32 vecSize(T)(VEC_NAME *self);
void vecClear(T)(VEC_NAME *self);
T *vecBegin(T)(VEC_NAME *self);
T *vecEnd(T)(VEC_NAME *self);
T *vecBack(T)(VEC_NAME *self);
void vecReverse(T)(VEC_NAME *self);
u32 vecIndexOf(T)(VEC_NAME *self, T*item);
void vecRemoveAt(T)(VEC_NAME *self, u32 index);
void vecRemove(T)(VEC_NAME *self, T*item);
void vecSort(T)(VEC_NAME *self, bool(*lessThan)(T*,T*));

#undef VEC_NAME
#undef VectorTPart
#undef T