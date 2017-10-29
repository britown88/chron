#pragma once

typedef struct Game Game;
typedef struct AppData AppData;

Game *gameCreate(AppData *data);
void gameDestroy(Game *self);

void gameStart(Game *self, AppData *data);
void gameUpdate(Game *self, AppData *data);
