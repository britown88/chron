#pragma once
#include "App.h"

typedef enum {
   PROFILE_FULL_FRAME = 0,
   PROFILE_UPDATE, //all app logic not spent during the hardware render step
   PROFILE_RENDER, //the hardware render step
   PROFILE_GAME_UPDATE, //all non-rendering game update
   PROFILE_GUI_UPDATE, // time spent in nuklear
   PROFILE_SNES_RENDER, // time it takes to render the SNES* to a buffer

   PROFILE_COUNT
} Profile;

#define PROFILE_FRAME_COUNT 10

typedef struct FrameProfiler {
   struct {
      Time entries[PROFILE_FRAME_COUNT];
      Time startTime;
   } profiles[PROFILE_COUNT];

   u64 frame;
}FrameProfiler;

static Time frameProfilerGetProfileAverage(FrameProfiler *self, Profile p) {
   u32 i = 0;
   Time total = { 0 };
   for (i = 0; i < PROFILE_FRAME_COUNT; ++i) {
      total.t += timeGetMicros(self->profiles[p].entries[i]);
   }
   return (Time) { total.t / PROFILE_FRAME_COUNT };
}

static void frameProfilerSetEntry(FrameProfiler *self, Profile p, Time time) {
   self->profiles[p].entries[self->frame%PROFILE_FRAME_COUNT] = time;
}

static void frameProfilerStartEntry(FrameProfiler *self, Profile p) {
   self->profiles[p].startTime = appGetTime(appGet());
}

static void frameProfilerEndEntry(FrameProfiler *self, Profile p) {
   self->profiles[p].entries[self->frame%PROFILE_FRAME_COUNT].t = appGetTime(appGet()).t - self->profiles[p].startTime.t;
}