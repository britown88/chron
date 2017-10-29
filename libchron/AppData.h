#pragma once

#include "libutils/Defs.h"
#include "libutils/Rect.h"

typedef struct TextureManager TextureManager;
typedef struct Texture Texture;
typedef struct FrameProfiler FrameProfiler;
typedef struct LogSpud LogSpud;

typedef struct {
   Int2 windowResolution;
   
   Int2 nativeResolution;
   bool fullScreen;
   bool vsync;
   float targetFramerate;
}Window;

typedef struct AppData {
   TextureManager *textureManager;
   FrameProfiler *frameProfiler;
   LogSpud *log;

   const Window *window;
   bool guiEnabled;
}AppData;
