#pragma once

#include "Vector.h"

typedef struct {
   f32 data[16];
}Matrix;

Matrix matrixMultiply(Matrix *lhs, Matrix *rhs);
Float2 matrixMultiplyV(Matrix *lhs, Float2 rhs);
void matrixIdentity(Matrix *m);
void matrixOrtho(Matrix *m, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
void matrixScale(Matrix *m, Float2 v);
void matrixTranslate(Matrix *m, Float2 v);

