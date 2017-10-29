#include "App.h"
#include "DeviceContext.h"
#include "Renderer.h"
#include "Config.h"
#include "AppData.h"
#include "EncodedAssets.h"

#include "libutils/CheckedMemory.h"
#include "libutils/Defs.h"
#include "libutils/IncludeWindows.h"

#include <time.h>

#include "FrameProfiler.h"
#include "LogSpud.h"
#include "Game.h"

static const char *TAG = "App";

static App *g_App;

typedef struct {
   Matrix view;
} UBOMain;

typedef struct {
   TextureManager *textureManager;
   
   Shader *baseShader;
   UBO *ubo;
   FBO *nativeFBO;
   Model *rectModel;

   StringView uModel, uColor, uTexture, uTextureSlot;
}RenderData;

struct App {
   bool running;
   Time lastUpdated;
   Renderer *renderer;
   DeviceContext *context;

   Window winData;
   RenderData rData;

   AppData data;
   FrameProfiler frameProfiler;

   LogSpud *log;
   Game *game;
};

static Window _buildWindowData() {
   return(Window){
      .windowResolution = { CONFIG_WINDOW_X , CONFIG_WINDOW_Y },
      .nativeResolution = { CONFIG_NATIVE_X , CONFIG_NATIVE_Y },
      .fullScreen = CONFIG_WINDOW_FULLSCREEN,
      .vsync = CONFIG_WINDOW_VSYNC,
      .targetFramerate = CONFIG_WINDOW_FRAMERATE
   };
}

static void _setupRenderData(App *app) {
   RenderData *self = &app->rData;
   self->textureManager = textureManagerCreate(NULL);
   self->baseShader = shaderCreateFromBuffer(enc_Shader, ShaderParams_DiffuseTexture|ShaderParams_Color);
   self->ubo = uboCreate(sizeof(UBOMain));

   self->nativeFBO = fboCreate(app->winData.nativeResolution, RepeatType_Clamp, FilterType_Nearest);

   FVF_Pos2_Tex2_Col4 vertices[] = {
      { .pos2 = { 0.0f, 0.0f },.tex2 = { 0.0f, 0.0f },.col4 = { 1.0f, 1.0f, 1.0f, 1.0f } },
      { .pos2 = { 1.0f, 0.0f },.tex2 = { 1.0f, 0.0f },.col4 = { 1.0f, 1.0f, 1.0f, 1.0f } },
      { .pos2 = { 1.0f, 1.0f },.tex2 = { 1.0f, 1.0f },.col4 = { 1.0f, 1.0f, 1.0f, 1.0f } },
      { .pos2 = { 0.0f, 0.0f },.tex2 = { 0.0f, 0.0f },.col4 = { 1.0f, 1.0f, 1.0f, 1.0f } },
      { .pos2 = { 1.0f, 1.0f },.tex2 = { 1.0f, 1.0f },.col4 = { 1.0f, 1.0f, 1.0f, 1.0f } },
      { .pos2 = { 0.0f, 1.0f },.tex2 = { 0.0f, 1.0f },.col4 = { 1.0f, 1.0f, 1.0f, 1.0f } },
   };

   self->rectModel = FVF_Pos2_Tex2_Col4_CreateModel(vertices, 6, ModelStreamType_Static);

   self->uModel = stringIntern("uModelMatrix");
   self->uColor = stringIntern("uColorTransform");
   self->uTexture = stringIntern("uTexMatrix");
   self->uTextureSlot = stringIntern("uTexture");
}

static void _renderDataDestroy(RenderData *self) {
   fboDestroy(self->nativeFBO);
   uboDestroy(self->ubo);
   shaderDestroy(self->baseShader);

   modelDestroy(self->rectModel);

   textureManagerDestroy(self->textureManager);
}


App *appCreate(Renderer *renderer, DeviceContext *context) {
   App *out = checkedCalloc(1, sizeof(App));
   g_App = out;

   out->log = logSpudCreate(&out->data);   

   out->renderer = renderer;
   out->context = context;

   out->winData = _buildWindowData();
   _setupRenderData(out); 

   out->data.log = out->log;

   out->data.textureManager = out->rData.textureManager;
   out->data.frameProfiler = &out->frameProfiler;

   (Window*)out->data.window = &out->winData;

#ifdef _DEBUG
   out->data.guiEnabled = true;
#endif

   out->game = gameCreate(&out->data);

   return out;
}
void appDestroy(App *self) {
   gameDestroy(self->game);

   _renderDataDestroy(&self->rData);
   logSpudDestroy(self->log);
   checkedFree(self);
}

AppData *appGetData(App *self) { return &self->data; }
Time appGetTime(App *self) { return deviceContextGetTime(self->context); }

App *appGet() {
   return g_App;
}

void appQuit(App *self) { self->running = false; }
int appRand(App *self, int lower, int upper) {
   return (rand() % (upper - lower)) + lower;
}

static void _start(App *self) {
   if(deviceContextCreateWindow(self->context, &self->data)) {
      return;
   }

   r_init(self->renderer);
   r_bindUBO(self->renderer, self->rData.ubo, 0);

   srand((unsigned int)time(NULL));

   gameStart(self->game, &self->data);

   self->running = true;
}

static void _renderBasicRectModel(App *self, Texture *tex, Float2 pos, Float2 size, ColorRGBAf color) {
   Renderer *r = self->renderer;

   Matrix model = { 0 };
   matrixIdentity(&model);
   matrixTranslate(&model, pos);
   matrixScale(&model, size);
   
   r_setMatrix(r, self->rData.uModel, &model);
   r_setColor(r, self->rData.uColor, &color);

   Matrix texMatrix = { 0 };
   matrixIdentity(&texMatrix);
   r_setMatrix(r, self->rData.uTexture, &texMatrix);

   r_bindTexture(r, tex, 0);
   r_setTextureSlot(r, self->rData.uTextureSlot, 0);

   r_renderModel(r, self->rData.rectModel, ModelRenderType_Triangles);
}

static void _gameStep(App *self) {
   frameProfilerStartEntry(&self->frameProfiler, PROFILE_GAME_UPDATE);
   gameUpdate(self->game, &self->data);
   frameProfilerEndEntry(&self->frameProfiler, PROFILE_GAME_UPDATE);
}

static void _renderGUI(App *self) {
   frameProfilerStartEntry(&self->frameProfiler, PROFILE_GUI_UPDATE);
   
   Renderer *r = self->renderer;
   Matrix model = { 0 };
   matrixIdentity(&model);

   r_setMatrix(r, self->rData.uModel, &model);
   r_setColor(r, self->rData.uColor, &White);

   Matrix texMatrix = { 0 };
   matrixIdentity(&texMatrix);
   r_setMatrix(r, self->rData.uTexture, &texMatrix);

   r_setTextureSlot(r, self->rData.uTextureSlot, 0);
   
   deviceContextUpdateGUI(self->context, &self->data);
   deviceContextRenderGUI(self->context, r);

   frameProfilerEndEntry(&self->frameProfiler, PROFILE_GUI_UPDATE);
}

static void _renderStep(App *self) {
   frameProfilerStartEntry(&self->frameProfiler, PROFILE_RENDER);


   //test render because maybe screw the fbo??
   Renderer *r = self->renderer;

   Int2 winSize = self->winData.windowResolution;
   const Recti winVP = { 0, 0, winSize.x, winSize.y };

   r_viewport(r, &winVP);
   r_clear(r, &DkGray);
   r_enableAlphaBlending(r, true);

   UBOMain ubo = { 0 };
   matrixIdentity(&ubo.view);
   matrixOrtho(&ubo.view, 0.0f, (float)winVP.w, (float)winVP.h, 0.0f, 1.0f, -1.0f);
   r_setUBOData(r, self->rData.ubo, ubo);

   r_setShader(r, self->rData.baseShader);

   if (self->data.guiEnabled) {
      _renderGUI(self);
   }
   else {
   }

   r_finish(r);
   r_flush(r);


   frameProfilerEndEntry(&self->frameProfiler, PROFILE_RENDER);
}

static void __updateDeviceContext(App *self) {
   deviceContextPollEvents(self->context, &self->data);

   //update the winsize into the appdata view
   self->winData.windowResolution = deviceContextGetWindowSize(self->context);

   if (deviceContextGetShouldClose(self->context)) {
      self->running = false;
      return;
   }
}

static void _step(App *self) {
   frameProfilerStartEntry(&self->frameProfiler, PROFILE_UPDATE);
   
   __updateDeviceContext(self);

   //game step
   _gameStep(self);

   //hardware render
   _renderStep(self);

   frameProfilerEndEntry(&self->frameProfiler, PROFILE_UPDATE);
}

static Time _getFrameTime() {
   static Time out;
   static bool outSet = false;
   if (!outSet) { out.t = (Microseconds)((1.0 / CONFIG_WINDOW_FRAMERATE) * 1000000); }
   return out;
}

static void freeUpCPU(Microseconds timeOffset) {
   if (timeOffset > 1500) {
      Sleep((DWORD)((timeOffset - 500) / 1000));
   }
   else if (timeOffset > 500) {
      SwitchToThread();
   }
}

static void _stepWithTiming(App *self) {
   Time usPerFrame = _getFrameTime();
   Time time = appGetTime(self);
   Time deltaTime = { time.t - self->lastUpdated.t };

   frameProfilerSetEntry(&self->frameProfiler, PROFILE_FULL_FRAME, deltaTime);
   if (deltaTime.t >= usPerFrame.t) {
      self->lastUpdated = time;
      _step(self);
   }
   else {
      freeUpCPU(usPerFrame.t - deltaTime.t);
   }
   ++self->frameProfiler.frame;
}

void appRun(App *self) {
   _start(self);
   while (self->running) {
      _stepWithTiming(self);
   }
   return;
}