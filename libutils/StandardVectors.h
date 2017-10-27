#pragma once

#include <stddef.h>
#include "Vector.h"
#include "StringView.h"

#define VectorTPart u32
#include "Vector_Decl.h"

#define VectorTPart char
#include "Vector_Decl.h"

#define VectorTPart byte
#include "Vector_Decl.h"

#define VectorTPart i32
#include "Vector_Decl.h"

#define VectorTPart Int2
#include "Vector_Decl.h"

#define VectorTPart Int3
#include "Vector_Decl.h"

