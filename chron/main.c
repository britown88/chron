#include "libchron/DeviceContext.h"
#include "libchron/Renderer.h"
#include "libchron/App.h"

#include "libutils/CheckedMemory.h"

i32 main(char **argc, i32 argv) {
   DeviceContext *context = deviceContextCreate();
   Renderer *renderer = rendererCreate(context);
   App *app = appCreate(renderer, context);

   appRun(app);

   rendererDestroy(renderer);
   deviceContextDestroy(context);
   appDestroy(app);

   printMemoryLeaks();


   return 0;
}