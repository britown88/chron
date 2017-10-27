#include "BitBuffer.h"
#include "libutils\CheckedMemory.h"
#include "BitTwiddling.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



BitBuffer bitBufferCreate(byte *existingData, i32 deleteData) {
   BitBuffer out = { .buffer = existingData,  .deleteData = deleteData, .pos = 0 };
   return out;
}
void bitBufferDestroy(BitBuffer *self) {
   if(self->deleteData) {
      checkedFree(self->buffer);
   }
}

//void bitBufferReadBits(BitBuffer *self, byte *destination, i32 bitCount) {
//   i32 i;
//   for (i = 0; i < bitCount; ++i) {
//      i32 position = self->pos + i;
//      byte value = getBitFromArray(self->buffer, position);
//      setBitInArray(destination, i, value);
//   }
//
//   self->pos += bitCount;
//}

void bitBufferReadBits(BitBuffer *self, byte *destination, i32 bitCount) {
   i32 i;
   i32 startBits = self->pos;
   i32 startBytes = self->pos >> 3;
   i32 toAlign = self->pos & 7;
   i32 byteCount;
   i32 byteBitCount;
   i32 leftover = bitCount;

   if (toAlign > 0) {
      startBytes += 1;
      for (i = 0; i < 8 - toAlign && i < bitCount; ++i) {
         i32 position = self->pos + i;
         byte value = getBitFromArray(self->buffer, position);
         setBitInArray(destination, i, value);
      }
      self->pos += i;
      leftover -= i;
   }

   byteCount = leftover >> 3;
   byteBitCount = byteCount << 3;

   if (byteCount > 0 && toAlign == 0) {
      memcpy(destination + (toAlign > 0 ? 1 : 0), self->buffer + startBytes, byteCount);
      self->pos += byteBitCount;
      leftover -= byteBitCount;
   } 

   for(i = 0; i < leftover; ++i) {
      i32 position = self->pos + i;
      byte value = getBitFromArray(self->buffer, position);
      setBitInArray(destination, self->pos - startBits + i, value);
   }

   self->pos += leftover;
}
short bitBufferReadShort(BitBuffer *self) {
   static byte readBuffer[sizeof(uint16_t)] = {0};
   bitBufferReadBits(self, readBuffer, sizeof(uint16_t)*8);
   return *(short*)readBuffer;
}

void bitBufferWriteBits(BitBuffer *self, i32 bitCount, byte *data) {
   i32 i;
   for(i = 0; i < bitCount; ++i) {
      i32 position = self->pos + i;

      byte value = getBitFromArray(data, i);
      setBitInArray(self->buffer, position, value);
   }

   self->pos += bitCount;
}

byte *readFullFile(const char *path, i32 *fsize) {
   byte *string;
   i32 fsizeBuffer = 0;
   FILE *f = fopen(path, "rb");

   if (!fsize) {
      fsize = &fsizeBuffer;
   }

   if (!f) {
      return NULL;
   }

   fseek(f, 0, SEEK_END);
   *fsize = ftell(f);
   fseek(f, 0, SEEK_SET);

   string = checkedMalloc(*fsize + 1);
   fread(string, *fsize, 1, f);
   fclose(f);

   string[*fsize] = 0;

   return string;
}
