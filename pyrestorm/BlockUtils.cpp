// (c) 2014 mkellner@robotranch.org

#include "Arduino.h"
#include "BlockUtils.h"

void setLED(int block, int led, struct RGBcolor *color) {
  int channel;

  channel = block * 32;
  channel += led * 3;
  if (led > 4)
    channel += 1;  // skips one element
  Tlc.set(channel+2, color->red);
  Tlc.set(channel+1, color->green);
  Tlc.set(channel, color->blue);
}

void clearBlock(struct RGBblock *block, int blockNum, int playRoutine, int cycleTime,
  struct RGBcolor *colorA, RGBcolor *colorB) {
  int i;
  block->blockNum = blockNum;
  block->lastTime = theTime;
  block->varA = 0;
  block->dirA = 1;
  block->varB = 0;
  block->dirB = 1;
  for (i=0; i<LEDS_PER_BLOCK; i++) {
    block->leds[i].red = 0;
    block->leds[i].green = 0;
    block->leds[i].blue = 0;
  }
  copyRGB(&block->colorA, colorA);
  copyRGB(&block->colorB, colorB);
  block->dir = 1;
  block->cycleTime = cycleTime;
  block->routine = playRoutine;
}

void blastBlock(struct RGBblock *block)  {
  int i;
  for (i=0; i<LEDS_PER_BLOCK; i++)
    setLED(block->blockNum, i, &block->leds[i]);
}


