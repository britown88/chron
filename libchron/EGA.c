#include "EGA.h"
#include "libutils/BitTwiddling.h"

static void _buildColorTable(ColorRGB *table) {
   byte i;
   //                   00 01  10   11
   byte rgbLookup[] = { 0, 85, 170, 255 };
   for (i = 0; i < EGA_COLORS; ++i) {
      byte shift = 5;

      byte r = getBit(i, shift--);
      byte g = getBit(i, shift--);
      byte b = getBit(i, shift--);
      byte R = getBit(i, shift--);
      byte G = getBit(i, shift--);
      byte B = getBit(i, shift);

      byte rgb_r = rgbLookup[(R << 1) + r];
      byte rgb_g = rgbLookup[(G << 1) + g];
      byte rgb_b = rgbLookup[(B << 1) + b];

      table[i] = (ColorRGB) { rgb_r, rgb_g, rgb_b };
   }
}

ColorRGB egaGetColor(EGAColor c) {
   static ColorRGB lookup[EGA_COLORS] = { 0 };
   static bool loaded = 0;

   if (!loaded) {
      _buildColorTable(lookup);
      loaded = 1;
   }

   return lookup[c];
}