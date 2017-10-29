#pragma once

#include <stdio.h>

#define DIAG_MSGBOX(MSG, ...) { \
   char __msgBuff[256] = {0}; \
   sprintf(__msgBuff, MSG, __VA_ARGS__); \
   diagShowMessagebox(__msgBuff); \
}

void diagShowMessagebox(const char *msg);