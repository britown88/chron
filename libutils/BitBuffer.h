#pragma once

#include "Defs.h"

typedef struct BitBuffer{
   i32 pos;
   i32 deleteData;
   byte *buffer;
} BitBuffer;

BitBuffer bitBufferCreate(byte *existingData, i32 deleteData);
void bitBufferDestroy(BitBuffer *self);

void bitBufferReadBits(BitBuffer *self, byte *destination, i32 bitCount);
short bitBufferReadShort(BitBuffer *self);

void bitBufferWriteBits(BitBuffer *self, i32 bitCount, byte *data);

byte *readFullFile(const char *path, i32 *fsize);