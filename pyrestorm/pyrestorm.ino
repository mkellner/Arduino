// (c) 2014 mkellner@robotranch.org

#include <Tlc5940.h>
#include "RGBUtils.h"
#include "DirSwitch.h"
#include "BlockUtils.h"

#define DEBUG 0
#if DEBUG
  #define BUG(x)  Serial.print(x)
  #define BUGLN(x)  Serial.println(x)
#else
  #define BUG(x)
  #define BUGLN(x)
#endif
  
struct RGBcolor rgbRED;
struct RGBcolor rgbGREEN;
struct RGBcolor rgbBLUE;
struct RGBcolor rgbYELLOW;
struct RGBcolor rgbORANGE;
struct RGBcolor rgbPURPLE;
struct RGBcolor rgbWHITE;
struct RGBcolor rgbBLACK;

unsigned long theTime;
struct RGBblock block;

//-----------------------------
#define colorBlockTypeWave 1
#define colorBlockTypeWaveAccMax  100
#define colorBlockTypeWaveCycleTime 20
#define colorBlockTypeWaveWaveTime 3000
#define colorBlockTypeWaveWaveCycles 300

int c = 0;
float a = 5;
const float pi = 3.14159;
float range = 2 * pi;
float inc = range / colorBlockTypeWaveWaveTime;

#define addDarkness .5      // up to 2
void colorBlock(struct RGBblock *block)
{
  int i, b;
  unsigned long val;
  float m, x, y;
  
  if ((theTime - block->lastTime) < block->cycleTime)
        return;

  switch (block->routine) {
    case -3:
        block->varA += 1;
        for (i=0; i<LEDS_PER_BLOCK; i++) {
          x = (float)(block->varA % 10);
          y = 1.0 / ( x + (float)10.0) * (x + (float)10.0);
          m = y;
          if (m > STD_OUTPUT)
            m = STD_OUTPUT;
BUG(" x: ");
BUG(x);
BUG(" y: ");
BUG(y);
BUG(" m: ");
BUGLN(m);
           if (m < 0)
            clearRGB(&block->leds[blockmap(i)]);
          else
            setRGBFaded(&block->leds[blockmap(i)], &block->colorA, m, STD_OUTPUT);
        }
        break;
      case colorBlockTypeWave:
        block->varA += 10;
        for (i=0; i<LEDS_PER_BLOCK; i++) {
          x = (float)(block->varA % colorBlockTypeWaveWaveTime) * inc;
          x += i * (range / LEDS_PER_BLOCK);
          y = sin(x) + 1 - addDarkness;
          m = STD_OUTPUT * (y / 2.0);
BUG(" x: ");
BUG(x);
BUG(" y: ");
BUG(y);
BUG(" m: ");
BUGLN(m);
           if (m < 0)
            clearRGB(&block->leds[blockmap(i)]);
          else
            setRGBFaded(&block->leds[blockmap(i)], &block->colorA, m, STD_OUTPUT);
        }
        break;       
    case -1:
        block->varA += 10;
        x = (float)(block->varA % colorBlockTypeWaveWaveTime) * inc;
        y = sin(x) + 1;
        m = STD_OUTPUT * (y / 2.0);
Serial.print(" x: ");
Serial.print(x);
Serial.print(" y: ");
Serial.print(y);
Serial.print(" m: ");
Serial.println(m);
           if (m < 0)
            clearRGB(&block->leds[blockmap(i)]);
          else
            setRGBFaded(&block->leds[blockmap(i)], &block->colorA, m, STD_OUTPUT);
        break;       
    case 0:
        block->varA += 10;
        x = (float)(block->varA % 1000);
Serial.print("Wave x: ");
Serial.println(x);
        for (i=0; i<LEDS_PER_BLOCK; i++) {
          // offset
          b = i * (colorBlockTypeWaveWaveCycles / LEDS_PER_BLOCK);
          // calculate intensity
          m = (sin(x) / x) * STD_OUTPUT;
Serial.print(" i: ");
Serial.print(i);
Serial.print(" b: ");
Serial.print(b);
Serial.print(" m: ");
Serial.println(m);
          if (m < 0)
            clearRGB(&block->leds[blockmap(i)]);
          else
            setRGBFaded(&block->leds[blockmap(i)], &block->colorA, m, STD_OUTPUT);
        }
        b = block->varB;
        m = a * x * x + b * x + colorBlockTypeWaveWaveTime;
Serial.print(" b: ");
Serial.print(b);
Serial.print(" dirB: ");
Serial.print(block->dirB);
Serial.print(" m: ");
Serial.println(m);
        block->varB += m * block->dirB;
        if (block->varB > colorBlockTypeWaveWaveTime) {
            block->dirB = -1;
            block->varB = colorBlockTypeWaveWaveTime;
        }
        else if (block->varB < 0) {
          block->dirB = 1;
          block->varB = 0;
        }
      break;
  }    
}
//-----------------------------

void setup() {
  Serial.begin(9600);
  Serial.println("Begin"); 
  Tlc.init();
  
  setRGB(&rgbRED, STD_OUTPUT, 0, 0);
  setRGB(&rgbGREEN, 0, STD_OUTPUT, 0);
  setRGB(&rgbBLUE, 0, 0, STD_OUTPUT);
  setRGB(&rgbYELLOW, STD_OUTPUT, STD_OUTPUT, 0);
  setRGB(&rgbPURPLE, STD_OUTPUT, 0, STD_OUTPUT);
  setRGB(&rgbORANGE, STD_OUTPUT, STD_OUTPUT/4, 0);
  setRGB(&rgbWHITE, STD_OUTPUT, STD_OUTPUT, STD_OUTPUT);
  setRGB(&rgbBLACK, 0, 0, 0);

  Tlc.clear();
  
  clearBlock(&block, 0, colorBlockTypeWave, colorBlockTypeWaveCycleTime, &rgbBLUE, &rgbPURPLE);
}

void loop() {
  int swChanged = 0;
  theTime = millis();
  
  swChanged = checkSwitch();
  colorBlock(&block);
  blastBlock(&block);
  
  Tlc.update();
  delay(10);
  
}
