
#pragma once

#include "Defs.h"

typedef struct {
   i32 x, y;
} Int2;

typedef struct {
   i32 x, y, z;
} Int3;

typedef struct {
   f32 x, y;
} Float2;

typedef struct {
   f32 x, y, z;
} Float3;

Float3 vCross(Float3 a, Float3 b);
f32 vDot(Float3 a, Float3 b);
Float3 vSubtract(Float3 a, Float3 b);
Float3 vAdd(Float3 a, Float3 b);
Float3 vNormalized(Float3 v);
Float3 *vNormalize(Float3 *v);
Float3 vScale(Float3 v, f32 s);
