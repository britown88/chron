#include "Diagnostics.h"

#include "libutils/IncludeWindows.h"

void diagShowMessagebox(const char *msg) {
   MessageBox(NULL, msg, "Diagnostics", MB_OK);
}