#include "GUI.h"
#include "Renderer.h"
#include "LogSpud.h"
#include "AppData.h"
#include "DeviceContext.h"
#include "FrameProfiler.h"
#include "EncodedAssets.h"

#include "libutils/IncludeWindows.h"
#include "libutils/CheckedMemory.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
//#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "nuklear.h"


#define MAX_VERTEX_MEMORY 1024 * 1024
#define MAX_ELEMENT_MEMORY 512 * 1024

static const struct nk_color GUIColorWhite = { 210, 210, 210, 255 };
static const struct nk_color GUIColorBlue = { 95, 108, 255, 255 };
static const struct nk_color GUIColorYellow = { 255, 242, 135, 255 };
static const struct nk_color GUIColorGreen = { 70, 196, 70, 255 };
static const struct nk_color GUIColorRed = { 219, 43, 60, 255 };

#define _colorToNKColor(in) nk_rgb(in.r, in.g, in.b)

static const char *LogSpudWin = "LogSpud";
static const char *TAG = "GUI";

#pragma region BASE GUI

typedef struct GUIWindow GUIWindow;
typedef struct GUIWindow{
   String *name;
   GUI *parent;
   void(*destroy)(GUIWindow *self); //handle any destruction of extra data
   void(*update)(GUIWindow *self, AppData *data);
} GUIWindow;

static GUIWindow *guiWindowCreate(GUI *parent, const char *name) {
   GUIWindow *out = checkedCalloc(1, sizeof(GUIWindow));
   out->parent = parent;
   out->name = stringCreate(name);
   return out;
}

static void guiWindowDestroy(GUIWindow *self) {
   if (self->destroy) {
      self->destroy(self);
   }
   stringDestroy(self->name);
   checkedFree(self);
}

typedef GUIWindow *GUIWindowPtr;
#define VectorT GUIWindowPtr
#include "libutils/Vector_Create.h"

static void _guiWindowPtrDestroy(GUIWindowPtr *self) {  
   guiWindowDestroy(*self);
}

/*
typedef struct{
   GUIWindow base;
   int someotherdata;
}MyWindow;
*/

typedef struct {
   GLuint fontTexture;
   GLuint vbo, vao, ebo;
   VertexAttribute *attrs;
} OGLData;
typedef FVF_Pos2_Tex2_Col4 GUIVertex;

struct GUI {
   struct nk_context ctx;
   struct nk_font_atlas atlas;
   struct nk_buffer cmds;
   struct nk_draw_null_texture null;
   OGLData ogl;

   GUIWindow *viewer;
   vec(GUIWindowPtr) *dialogs;

   u32 charToolCount;
   u32 errorCount, processedLogLineCount;
   bool scrollLogToBot;
};

static void _createWindows(GUI *self);
static void _destroyWindows(GUI *self);
GUI *guiCreate() {
   GUI *out = checkedCalloc(1, sizeof(GUI));

   _createWindows(out);

   return out;
}
void guiDestroy(GUI *self) {
   _destroyWindows(self);
   nk_font_atlas_clear(&self->atlas);
   nk_free(&self->ctx);
   checkedFree(self);
}

#pragma region Init/Render

void guiBeginInput(GUI *self) {
   nk_input_begin(&self->ctx);
}
void guiEndInput(GUI *self) {
   nk_input_end(&self->ctx);
}
int guiProcessInputEvent(GUI *self, SDL_Event *evt) {
   struct nk_context *ctx = &self->ctx;
   if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
      /* key events */
      int down = evt->type == SDL_KEYDOWN;
      const Uint8* state = SDL_GetKeyboardState(0);
      SDL_Keycode sym = evt->key.keysym.sym;

      if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT) {
         nk_input_key(ctx, NK_KEY_SHIFT, down);
      }
      else if (sym == SDLK_DELETE) {
         nk_input_key(ctx, NK_KEY_DEL, down);
      }
      else if (sym == SDLK_RETURN) {
         nk_input_key(ctx, NK_KEY_ENTER, down);
      }
      else if (sym == SDLK_TAB) {
         nk_input_key(ctx, NK_KEY_TAB, down);
      }
      else if (sym == SDLK_BACKSPACE) {
         nk_input_key(ctx, NK_KEY_BACKSPACE, down);
      }
      else if (sym == SDLK_HOME) {
         nk_input_key(ctx, NK_KEY_TEXT_START, down);
         nk_input_key(ctx, NK_KEY_SCROLL_START, down);
      }
      else if (sym == SDLK_END) {
         nk_input_key(ctx, NK_KEY_TEXT_END, down);
         nk_input_key(ctx, NK_KEY_SCROLL_END, down);
      }
      else if (sym == SDLK_PAGEDOWN) {
         nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
      }
      else if (sym == SDLK_PAGEUP) {
         nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
      }
      else if (sym == SDLK_z) {
         nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_r) {
         nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_c) {
         nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_v) {
         nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_x) {
         nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_b) {
         nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_e) {
         nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
      }
      else if (sym == SDLK_UP) {
         nk_input_key(ctx, NK_KEY_UP, down);
      }
      else if (sym == SDLK_DOWN) {
         nk_input_key(ctx, NK_KEY_DOWN, down);
      }
      else if (sym == SDLK_LEFT) {
         if (state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
         }
         else {
            nk_input_key(ctx, NK_KEY_LEFT, down);
         }
      }
      else if (sym == SDLK_RIGHT) {
         if (state[SDL_SCANCODE_LCTRL]) {
            nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
         }
         else {
            nk_input_key(ctx, NK_KEY_RIGHT, down);
         }
      }
      else {
         return 0;
      }

      return 1;
   }
   else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
      /* mouse button */
      int down = evt->type == SDL_MOUSEBUTTONDOWN;
      const int x = evt->button.x, y = evt->button.y;
      if (evt->button.button == SDL_BUTTON_LEFT) {
         if (evt->button.clicks > 1)
            nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
         nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
      }
      else if (evt->button.button == SDL_BUTTON_MIDDLE)
         nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
      else if (evt->button.button == SDL_BUTTON_RIGHT)
         nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
      return 1;
   }
   else if (evt->type == SDL_MOUSEMOTION) {
      /* mouse motion */
      if (ctx->input.mouse.grabbed) {
         int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
         nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
      }
      else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
      return 1;
   }
   else if (evt->type == SDL_TEXTINPUT) {
      /* text input */
      nk_glyph glyph;
      memcpy(glyph, evt->text.text, NK_UTF_SIZE);
      nk_input_glyph(ctx, glyph);
      return 1;
   }
   else if (evt->type == SDL_MOUSEWHEEL) {
      /* mouse wheel */
      nk_input_scroll(ctx, nk_vec2((float)evt->wheel.x, (float)evt->wheel.y));
      return 1;
   }
   return 0;
}
static void _uploadAtlas(GUI *self, const void *image, int width, int height) {
   glGenTextures(1, &self->ogl.fontTexture);
   glBindTexture(GL_TEXTURE_2D, self->ogl.fontTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, image);
}
static void _fontStashBegin(GUI *self, struct nk_font_atlas **atlas){
   nk_font_atlas_init_default(&self->atlas);
   nk_font_atlas_begin(&self->atlas);
   *atlas = &self->atlas;
}
static void _fontStashEnd(GUI *self) {
   const void *image; int w, h;
   image = nk_font_atlas_bake(&self->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

   _uploadAtlas(self, image, w, h);
   nk_font_atlas_end(&self->atlas, nk_handle_id((int)self->ogl.fontTexture), &self->null);

   if (self->atlas.default_font) {
      nk_style_set_font(&self->ctx, &self->atlas.default_font->handle);
   }
}
static void _initOGLData(OGLData *self) {
   glGenBuffers(1, &self->vbo);
   glGenBuffers(1, &self->ebo);
   glGenVertexArrays(1, &self->vao);

   self->attrs = FVF_Pos2_Tex2_Col4_GetAttrs();
}
void guiInit(GUI *self) {
   nk_init_default(&self->ctx, 0);
   struct nk_font_atlas *atlas;
   _fontStashBegin(self, &atlas);
   _fontStashEnd(self);

   nk_buffer_init_default(&self->cmds);

   _initOGLData(&self->ogl);
}
void guiRender(GUI *self, Renderer *r) {
   OGLData *ogl = &self->ogl;
   void *vertices, *elements;

   glEnable(GL_BLEND);
   glBlendEquation(GL_FUNC_ADD);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_CULL_FACE);
   glDisable(GL_DEPTH_TEST);
   glEnable(GL_SCISSOR_TEST);
   glActiveTexture(GL_TEXTURE0);

   glBindVertexArray(ogl->vao);
   glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ebo);

   glHelperBindVertexAttrributes(ogl->attrs, sizeof(GUIVertex));

   glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_MEMORY, NULL, GL_STREAM_DRAW);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_MEMORY, NULL, GL_STREAM_DRAW);

   vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
   elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
   {
      /* fill convert configuration */
      struct nk_convert_config config;
      static const struct nk_draw_vertex_layout_element vertex_layout[] = {
         { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(GUIVertex, pos2) },
         { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(GUIVertex, tex2) },
         { NK_VERTEX_COLOR, NK_FORMAT_R32G32B32A32_FLOAT, NK_OFFSETOF(GUIVertex, col4) },
         { NK_VERTEX_LAYOUT_END }
      };
      NK_MEMSET(&config, 0, sizeof(config));
      config.vertex_layout = vertex_layout;
      config.vertex_size = sizeof(GUIVertex);
      config.vertex_alignment = NK_ALIGNOF(GUIVertex);
      config.null = self->null;
      config.circle_segment_count = 22;
      config.curve_segment_count = 22;
      config.arc_segment_count = 22;
      config.global_alpha = 1.0f;
      config.shape_AA = NK_ANTI_ALIASING_ON;
      config.line_AA = NK_ANTI_ALIASING_ON;

      /* setup buffers to load vertices and elements */
      {struct nk_buffer vbuf, ebuf;
      nk_buffer_init_fixed(&vbuf, vertices, (nk_size)MAX_VERTEX_MEMORY);
      nk_buffer_init_fixed(&ebuf, elements, (nk_size)MAX_ELEMENT_MEMORY);
      nk_convert(&self->ctx, &self->cmds, &vbuf, &ebuf, &config);}
   }

   glUnmapBuffer(GL_ARRAY_BUFFER);
   glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

   const struct nk_draw_command *cmd;
   const nk_draw_index *offset = NULL;
   Int2 winSize = r_getSize(r);

   nk_draw_foreach(cmd, &self->ctx, &self->cmds) {
      if (!cmd->elem_count) continue;
      glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);

      glScissor((GLint)(cmd->clip_rect.x),
         (GLint)((winSize.y - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h))),
         (GLint)(cmd->clip_rect.w),
         (GLint)(cmd->clip_rect.h));

      glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
      offset += cmd->elem_count;
   }
   nk_clear(&self->ctx);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   glDisable(GL_BLEND);
   glDisable(GL_SCISSOR_TEST);
}

#pragma endregion



static void _viewerUpdate(GUIWindow *self, AppData *data);


void _createWindows(GUI *self) {
   self->viewer = guiWindowCreate(self, "Viewer");
   self->viewer->update = &_viewerUpdate;

   self->dialogs = vecCreate(GUIWindowPtr)(&_guiWindowPtrDestroy);
}

void _destroyWindows(GUI *self){
   guiWindowDestroy(self->viewer);
   vecDestroy(GUIWindowPtr)(self->dialogs);
}


void _viewerUpdate(GUIWindow *self, AppData *data) {
   struct nk_context *ctx = &self->parent->ctx;
   Int2 windowSize = data->window->windowResolution;


   struct nk_rect viewerRect = nk_rect(0, 0, (f32)windowSize.x, (f32)windowSize.y);
   if (nk_begin(ctx, c_str(self->name), viewerRect,
      NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_BORDER | NK_WINDOW_TITLE  ))
   {
      viewerRect = nk_window_get_bounds(ctx);

      if (!nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)) {
         //if (viewerRect.x < 0 || viewerRect.y < 0) { 
         //   viewerRect.w = windowSize.x - optionsSize.x; 
         //}
         if (viewerRect.x < 0) { viewerRect.x = 0; }
         if (viewerRect.y < 0) { viewerRect.y = 0; }

         viewerRect.h = (viewerRect.w * 9) / 16.0f + 50;
         nk_window_set_bounds(ctx, viewerRect);
      }

      if (nk_window_has_focus(ctx) && nk_input_is_mouse_click_in_rect(&ctx->input, NK_BUTTON_RIGHT, viewerRect)) {
         viewerRect.w = (f32)windowSize.x; 

         viewerRect.x = 0;
         viewerRect.y = 0;
         viewerRect.h = (viewerRect.w * 9) / 16.0f + 50;
         nk_window_set_bounds(ctx, viewerRect);
      }

      struct nk_rect winBounds = nk_window_get_content_region(ctx);
      nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0, 0));

      float h = (winBounds.w * 9) / 16.0f;
      nk_layout_row_begin(ctx, NK_DYNAMIC, h, 1);
      nk_layout_row_push(ctx, 1.0f);

      enum nk_widget_layout_states state;
      struct nk_rect bounds;
      state = nk_widget(&bounds, ctx);
      if (state) {
         //TODO: draw ega viewer
         //uint32_t handle = textureGetGLHandle(data->snesTex);
         //struct nk_image img = nk_image_id(handle);
         //nk_draw_image(nk_window_get_canvas(ctx), bounds, &img, nk_rgb(255, 255, 255));
      }

      nk_layout_row_end(ctx);
      nk_style_pop_vec2(ctx);
   }
   nk_end(ctx);

}


static void _updateDialog(GUIWindowPtr *win, AppData *data, vec(GUIWindowPtr) *remList) {
   GUIWindow *dlg = *win;
   struct nk_context *ctx = &dlg->parent->ctx;

   dlg->update(dlg, data);

   if (nk_window_is_hidden(ctx, c_str(dlg->name))) {
      nk_window_close(ctx, c_str(dlg->name));
      vecPushBack(GUIWindowPtr)(remList, win);
   }
}


void guiUpdate(GUI *self, AppData *data) {
   _viewerUpdate(self->viewer, data);

   vec(GUIWindowPtr) *remList = vecCreate(GUIWindowPtr)(NULL);
   vecForEach(GUIWindowPtr, win, self->dialogs, {
      _updateDialog(win, data, remList);
   });

   vecForEach(GUIWindowPtr, win, remList, {
      vecRemove(GUIWindowPtr)(self->dialogs, win);
   });

   vecDestroy(GUIWindowPtr)(remList);
}

#pragma endregion
