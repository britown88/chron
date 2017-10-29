#include "String.h"
#include "StandardVectors.h"
#include "libutils\CheckedMemory.h"
#include "Defs.h"

#pragma pack(push, 1)
struct String{
   vec(char) *str;
};
#pragma pack(pop)

String *stringCreate(const char *str){
   vec(char) *out = vecCreate(char)(NULL);

   stringConcat((String*)out, str);

   return (String*)out;
}
String *stringCopy(String *other) {
   return stringCreate(c_str(other));
}
void stringDestroy(String *self){
   vecDestroy(char)((vec(char)*)self);
}
u32 stringLen(String *self){
   return vecSize(char)((vec(char)*)self) - 1;
}
u32 stringFindLastOf(String *self, const char *chrs) {
   u32 len = stringLen(self);
   u32 chrLen = (u32)strlen(chrs);
   u32 i = stringNPos;

   for (i = len - 1; i < len; --i) {
      u32 chIndex = 0;
      for (chIndex = 0; chIndex < chrLen; ++chIndex) {
         if (c_str(self)[i] == chrs[chIndex]) {
            return i;
         }
      }
   }

   return i;
}

void stringSubStr(String *self, u32 start, u32 len) {
   static char close = 0;
   u32 actuallen = stringLen(self);
   u32 finalLen = MIN(len - start, actuallen - start);

   if (start >= actuallen) {
      return;
   }

   memcpy((char*)c_str(self), (char*)c_str(self) + start, finalLen);
   vecResize(char)((vec(char)*)self, finalLen, &(char){0});
   vecPushBack(char)((vec(char)*)self, &(char){0});
}

String *stringGetFilename(String *self) {
   u32 dotPos = stringFindLastOf(self, ".");
   u32 dirPos = stringFindLastOf(self, "\\/");
   u32 oLen = stringLen(self);
   u32 begin = dirPos < stringNPos ? dirPos + 1 : 0;
   u32 len = dotPos < stringNPos ? dotPos - begin : oLen;

   if (!dotPos) {
      len = oLen - (dirPos+1);
   }
   
   String *out = stringCreate("");
   vecResize(char)((vec(char)*)out, len + 1, &(char){0});
   memcpy((char*)c_str(out), (char*)c_str(self) + begin, len);
   return out;
}

String *stringGetDirectory(String *self) {
   u32 dirPos = stringFindLastOf(self, "\\/");
   String *out = stringCreate("");

   if (dirPos != stringNPos) {
      vecResize(char)((vec(char)*)out, dirPos + 1, &(char){0});
      memcpy((char*)c_str(out), (char*)c_str(self), dirPos);
   }

   return out;

}

void stringClear(String *self){
   vecClear(char)((vec(char)*)self);
   vecPushBack(char)((vec(char)*)self, &(char){0});
}
void stringConcat(String *self, const char*str){
   stringConcatEX(self, str, (u32)strlen(str));
}
void stringConcatChar(String *self, const char c) {
   *(vecEnd(char)((vec(char)*)self) - 1) = c;
   vecPushBack(char)((vec(char)*)self, &(char){0});
}
void stringConcatEX(String *self, const char*str, u32 length) {
   vecPopBack(char)((vec(char)*)self);//kill the terminator
   vecPushArray(char)((vec(char)*)self, (char*)str, length);
   vecPushBack(char)((vec(char)*)self, &(char){0});
}
void stringSet(String *self, const char*str) {
   vecClear(char)((vec(char)*)self);
   vecPushArray(char)((vec(char)*)self, (char*)str, (u32)strlen(str));
   vecPushBack(char)((vec(char)*)self, &(char){0});
}

const char *c_str(String *str){
   if (!str) {
      return NULL;
   }
   //return vecAt(char)((vec(char)*)str, 0);
   return *((char**)str);
}

void stringInsert(String *self, char c, u32 pos) {
   if (pos <= stringLen(self)) {      
      vecInsert(char)((vec(char)*)self, pos, &c);
   }
}
void stringErase(String *self, u32 pos) {
   if (pos < stringLen(self)) {
      vecRemoveAt(char)((vec(char)*)self, pos);
   }
}

bool stringEqual(String *s1, String *s2) {
   u32 len1 = stringLen(s1);
   u32 len2 = stringLen(s2);

   if (len1 != len2) {
      return false;
   }

   return memcmp(c_str(s1), c_str(s2), len1) == 0;   
}

bool stringEqualRaw(String *s1, const char *s2) {
   u32 len1 = stringLen(s1);
   u32 len2 = (u32)strlen(s2);

   if (len1 != len2) {
      return false;
   }

   return memcmp(c_str(s1), s2, len1) == 0;
}

bool stringStartsWith(const char *s1, const char *s2, bool caseSensitive) {

   char *p1 = (char *)s1;
   char *p2 = (char *)s2;

   while (*p1 && *p2) {
      char c1 = *p1++;
      char c2 = *p2++;

      if (!caseSensitive) {
         if (c1 >= 'A' && c1 <= 'Z') { c1 -= 'A' - 'a'; }
         if (c2 >= 'A' && c2 <= 'Z') { c2 -= 'A' - 'a'; }
      }

      if (c1 != c2) {
         return false;
      }
   }

   return !*p2;
}

bool stringPtrCompare(StringPtr *str1, StringPtr *str2) {
   char *p1 = (char *)c_str(*str1);
   char *p2 = (char *)c_str(*str2);
   char c1, c2;

   while (*p1 && *p2) {
      c1 = *p1;
      c2 = *p2;

      if (c1 >= 'A' && c1 <= 'Z') { c1 -= 'A' - 'a'; }
      if (c2 >= 'A' && c2 <= 'Z') { c2 -= 'A' - 'a'; }

      if (c1 != c2) {
         break;
      }

      ++p1;
      ++p2;
   }

   return c1 == c2 ? stringLen(*str1) < stringLen(*str2) : c1 < c2;
}

void stringPtrDestroy(StringPtr *self) {
   stringDestroy(*self);
}
#define VectorTPart StringPtr
#include "Vector_Impl.h"

vec(StringPtr) *stringSplit(const char *self, char delim) {
   vec(StringPtr) *out = vecCreate(StringPtr)(&stringPtrDestroy);
   static char buff[256] = { 0 };
   u32 bufflen = 0;
   String *item = NULL;

   char c;
   char *str = (char *)self;

   while (c = *str++) {
      if (c == delim) {         
         buff[bufflen] = 0;
         item = stringCreate(buff);
         vecPushBack(StringPtr)(out, &item);
         bufflen = 0;
      }
      else {
         buff[bufflen++] = c;
      }
   }

   buff[bufflen] = 0;
   item = stringCreate(buff);
   vecPushBack(StringPtr)(out, &item);

   return out;
}