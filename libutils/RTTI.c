#include "RTTI.h"

u32 getUniqueRTTIID(){
   static u32 _eCount = 0;
   return _eCount++;
}
