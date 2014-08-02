// (c) 2014 mkellner@robotranch.org

#include "Arduino.h"
#include "RGBUtils.h"

void setRGB(struct RGBcolor *color, int r, int g, int b) {
  color->red = r;
  color->green = g;
  color->blue = b;
}
void clearRGB(RGBcolor *color) {
  color->red = 0;
  color->green = 0;
  color->blue = 0;
}
void copyRGB(struct RGBcolor *colorTo, RGBcolor *colorFrom) {
  colorTo->red = colorFrom->red;
  colorTo->green = colorFrom->green;
  colorTo->blue = colorFrom->blue;
}

 int mixValues(int a, int b, int val, int maxVal) {
   unsigned long v, u;
   v = a;
   v *= val;
   v /= maxVal;
   u = b;
   u *= (maxVal - val);
   u /= maxVal;
   return v/2 + u/2;
}

int fadeValue(int a, int val, int maxVal) {
   unsigned long v;
   v = a;
   v *= val;
   v /= maxVal;
   return v;
} 

//-----------------------------
void setRGBFaded(RGBcolor *colorOut, RGBcolor *colorIn, int val, int maxVal) {
   colorOut->red = fadeValue(colorIn->red, val, maxVal);
   colorOut->green = fadeValue(colorIn->green, val, maxVal);
   colorOut->blue = fadeValue(colorIn->blue, val, maxVal);
}
 
 void setRGBMerged(RGBcolor *colorOut, RGBcolor *colorA, RGBcolor *colorB, int val, int maxVal) {
   colorOut->red = mixValues(colorA->red, colorB->red, val, maxVal);
   colorOut->green = mixValues(colorA->green, colorB->green, val, maxVal);
   colorOut->blue = mixValues(colorA->blue, colorB->blue, val, maxVal);
 }
 

