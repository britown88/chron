#include "Game.h"
#include "libutils/CheckedMemory.h"
#include "LogSpud.h"
#include "App.h"
#include "AppData.h"


struct Game {
   int UNUSED;
};

Game *gameCreate(AppData *data) {
   Game *out = checkedCalloc(1, sizeof(Game));

   return out;
}
void gameDestroy(Game *self) {
   checkedFree(self);
}

void gameStart(Game *self, AppData *data) {

}

void gameUpdate(Game *self, AppData *data) {

}
