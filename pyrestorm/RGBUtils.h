// (c) 2014 mkellner@robotranch.org

#ifndef __RGBUTILS_H__
#define __RGBUTILS_H__ 1
struct RGBcolor {
  int red;
  int green;
  int blue;  
};
typedef struct RGBColor RGBColor;

void setRGB(struct RGBcolor *color, int r, int g, int b);
void clearRGB(RGBcolor *color);
void copyRGB(struct RGBcolor *colorTo, RGBcolor *colorFrom);
int mixValues(int a, int b, int val, int maxVal);
int fadeValue(int a, int val, int maxVal);
void setRGBFaded(RGBcolor *colorOut, RGBcolor *colorIn, int val, int maxVal);
void setRGBMerged(RGBcolor *colorOut, RGBcolor *colorA, RGBcolor *colorB, int val, int maxVal);

#endif // __RGBUTILS_H__
