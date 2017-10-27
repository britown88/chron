#pragma once

#include "Defs.h"

u32 hashPtr(void* ptr);

i32 minByteCount(i32 bitCount);
i32 minIntCount(i32 bitCount);
void setBit(byte *dest, byte pos/*0-7*/, byte value/*0-1*/);
void setBitInArray(byte *dest, i32 pos, byte value/*0-1*/);
byte getBit(byte dest, byte pos/*0-7*/);
byte getBitFromArray(const byte *dest, i32 pos);

byte asciiFrom4BitHex(byte b);

//returns 0 if compressed size is larger than inBitCount
i32 compressBitsRLE(const byte *in, const i32 inBitCount, byte *out);
void decompressRLE(byte *src, i32 compressedBitCount, byte *dest);

byte arrayIsSolid(byte *src, i32 bitCount);

u32 BSR32(u32 value);
void STOSD(u32 *dest, u32 val, u32 count);

