//#define ClosureT \
//    CLOSURE_RET(i32) \
//    CLOSURE_NAME(SampleClosure) \
//    CLOSURE_ARGS(i32)
//#include "Closure_Create.h"
//
//i32 foo(ClosureData a, i32 b){
//   return 0;
//}
//
//SampleClosure c;
//closureInit(SampleClosure)(&c, data, &foo, &destructor);
//closureCall(&c, 1);
//closureDestroy(SampleClosure)(&c);

#define ClosureTPart ClosureT
#include "Closure_Decl.h"
#define ClosureTPart ClosureT
#include "Closure_Impl.h"
#undef ClosureT