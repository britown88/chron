#pragma once

#include "libutils/Defs.h"
#include "libutils/Math.h"
#include "libutils/String.h"


typedef struct DeviceContext DeviceContext;
typedef struct Renderer Renderer;
typedef struct AppData AppData;

DeviceContext *deviceContextCreate();
void deviceContextDestroy(DeviceContext *self);

int deviceContextCreateWindow(DeviceContext *self, AppData *data);
void deviceContextPrepareForRendering(DeviceContext *self);
void deviceContextCommitRender(DeviceContext *self);
void deviceContextPollEvents(DeviceContext *self, AppData *data);

void deviceContextRenderGUI(DeviceContext *self, Renderer *r);
void deviceContextUpdateGUI(DeviceContext *self, AppData *data);

Int2 deviceContextGetWindowSize(DeviceContext *self);
Int2 deviceContextGetDrawableSize(DeviceContext *self);
Time deviceContextGetTime(DeviceContext *self);
bool deviceContextGetShouldClose(DeviceContext *self);

#define DC_FILE_ALL 1 //returns both directories and files
#define DC_FILE_DIR_ONLY 2  //returns only directories
#define DC_FILE_FILE_ONLY 3 //return only files

#include "libutils/StandardVectors.h"
int deviceContextListFiles(const char *root, int type, vec(StringPtr) **out, const char *ext);

