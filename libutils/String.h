#pragma once
#include <string.h>

#include "libutils/Defs.h"
#include "Vector.h"

typedef struct String_t String;

//String *foo = stringCreate("test");
//printf((char*)foo);
//stringDestroy(foo);

static const u32 stringNPos = (-1);

String *stringCreate(const char *str);
String *stringCopy(String *other);
void stringDestroy(String *self);
u32 stringLen(String *self);
u32 stringFindLastOf(String *self, const char *characters);
void stringClear(String *self);
void stringSubStr(String *self, u32 start, u32 len);
String *stringGetFilename(String *str); //returns new, truncates after last '.' and before last '/'
String *stringGetDirectory(String *str); //returns new, truncates after last '/'
void stringConcat(String *self, const char*str);
void stringConcatChar(String *self, const char c);
void stringConcatEX(String *self, const char*str, u32 length);
void stringSet(String *self, const char*str);
void stringInsert(String *self, char c, u32 pos);
void stringErase(String *self, u32 pos);
bool stringEqual(String *s1, String *s2);
bool stringEqualRaw(String *s1, const char *s2);
bool stringStartsWith(const char *s1, const char *s2, bool caseSensitive);
const char *c_str(String *str);


typedef String* StringPtr;
void stringPtrDestroy(StringPtr *self);
#define VectorTPart StringPtr
#include "Vector_Decl.h"

vec(StringPtr) *stringSplit(const char *self, char delim);

bool stringPtrCompare(StringPtr *str1, StringPtr *str2);
