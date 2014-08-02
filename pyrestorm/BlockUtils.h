// (c) 2014 mkellner@robotranch.org

#ifndef __BLOCKUTILS_H__
#define __BLOCKUTILS_H__ 1

#include <Tlc5940.h>
#include "RGBUtils.h"

extern unsigned long theTime;

#define MAX_LEDS  10
#define LEDS_PER_BLOCK  10
#define MAX_OUTPUT  4096
#define STD_OUTPUT  300

const char blockmapM[] = {       // circular mapping
  5,  6,  7,  8,  9,
  4,  3,  2,  1,  0  };
#define blockmap(a)  blockmapM[(a) % LEDS_PER_BLOCK]

const char blockmapZ[] = {      // zigzag mapping
  5, 0, 6, 1, 7, 2, 8, 3, 9, 4 };
#define zigzagmap(a)  blockmapZ[(a) % LEDS_PER_BLOCK]

const char blockmapFlag[] = {
  5, 6, 7, 8, 9, 0, 1, 2, 3, 4 };
#define flagmap(a) blockmapFlag[(a) % LEDS_PER_BLOCK]


struct RGBblock {
  int blockNum;            // number of block in chain
  int routine;
  unsigned long lastTime;
  int cycleTime;   // varies per routine
  int varA;        // varies per routine
  int varB;
  int dirA;
  int dirB;
  int dir;          // direction
  struct RGBcolor colorA;
  struct RGBcolor colorB;
  struct RGBcolor leds[LEDS_PER_BLOCK];
};

void setLED(int block, int led, struct RGBcolor *color);
void clearBlock(struct RGBblock *block, int blockNum, int playRoutine, int cycleTime,
  struct RGBcolor *colorA, RGBcolor *colorB);
void blastBlock(struct RGBblock *block);

#endif // __BLOCKUTILS_H__
