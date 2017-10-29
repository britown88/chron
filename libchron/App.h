#pragma once

#include "libutils/Defs.h"

typedef struct App App;
typedef struct DeviceContext DeviceContext;
typedef struct Renderer Renderer;
typedef struct AppData AppData;

App *appCreate(Renderer *renderer, DeviceContext *context);
void appDestroy(App *self);

void appRun(App *self);

App *appGet();
Time appGetTime(App *app);

AppData *appGetData(App *self);