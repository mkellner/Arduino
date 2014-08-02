// (c) 2014 mkellner@robotranch.org

#include <Tlc5940.h>

#define MAX_LEDS  10
#define LEDS_PER_BLOCK  10
#define MAX_OUTPUT  4096
#define STD_OUTPUT  300

int blockmapM[] = {       // circular mapping
  5,  6,  7,  8,  9,
  4,  3,  2,  1,  0  };
#define blockmap(a)  blockmapM[(a) % LEDS_PER_BLOCK]

int blockmapZ[] = {      // zigzag mapping
  5, 0, 6, 1, 7, 2, 8, 3, 9, 4 };
#define zigzagmap(a)  blockmapZ[(a) % LEDS_PER_BLOCK]

struct RGBcolor {
  int red;
  int green;
  int blue;  
};
typedef struct RGBColor RGBColor;

struct RGBblock {
  int blockNum;            // number of block in chain
  int routine;
  unsigned long lastTime;
  int cycleTime;   // varies per routine
  int varA;        // varies per routine
  int varB;  
  int dir;          // direction
  struct RGBcolor colorA;
  struct RGBcolor colorB;
  struct RGBcolor leds[LEDS_PER_BLOCK];
};

struct RGBcolor rgbRED;
struct RGBcolor rgbGREEN;
struct RGBcolor rgbBLUE;
struct RGBcolor rgbYELLOW;
struct RGBcolor rgbORANGE;
struct RGBcolor rgbPURPLE;
struct RGBcolor rgbWHITE;
struct RGBcolor rgbBLACK;

int switchPos = 0;
int dir = 0;
int downPin = 4;
int upPin = 8;

unsigned long theTime;

int checkSwitch() {
  int changed = 0;
  if (digitalRead(downPin)) {
    Serial.print("Dn ");
    if (switchPos != -1) {
      switchPos = -1;
      changed = 1;
    }
  }
  else if (digitalRead(upPin)) {
    Serial.print("Up ");
    if (switchPos != 1) {
      switchPos = 1;
      changed = 1;
    }
  }
  else {
    if (switchPos != 0) {
      switchPos = 0;
      changed = 1;
    }
  }
}


void setLED(int block, int led, struct RGBcolor *color) {
  int channel;

  channel = block * 32;
  channel += led * 3;
  if (led > 4)
    channel += 1;  // skips one element
  Tlc.set(channel+2, color->red);
  Tlc.set(channel+1, color->green);
  Tlc.set(channel, color->blue);
#if 0
  Serial.print(" LED:");
  Serial.print(led);
  Serial.print(" r:");
  Serial.print(color->red);
  Serial.print(" g:");
  Serial.print(color->green);
  Serial.print(" b:");
  Serial.print(color->blue);
  Serial.println();
#endif
}

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

//-----------------------------
void setRGBFaded(RGBcolor *colorOut, RGBcolor *colorIn, int val, int maxVal) {
   unsigned long v;
   v = colorIn->red;
   v *= val;
   v /= maxVal;
   colorOut->red = v;

   v = colorIn->green;
   v *= val;
   v /= maxVal;   
   colorOut->green = v;
   
   v = colorIn->blue;
   v *= val;
   v /= maxVal;
   colorOut->blue = v;
 }
 
 void setRGBMerged(RGBcolor *colorOut, RGBcolor *colorA, RGBcolor *colorB, int val, int maxVal) {
   unsigned long u, v;
   v = colorA->red;
   v *= val;
   v /= maxVal;
   u = colorB->red;
   u *= (maxVal - val);
   u /= maxVal;
   colorOut->red = v/2 + u/2;
   v = colorA->green;
   v *= val;
   v /= maxVal;
   u = colorB->green;
   u *= (maxVal - val);
   u /= maxVal;
   colorOut->green = v/2 + u/2;
   v = colorA->blue;
   v *= val;
   v /= maxVal;
   u = colorB->blue;
   u *= (maxVal - val);
   u /= maxVal;
   colorOut->blue = v/2 + u/2;
 }
 

void incrementVarA(struct RGBblock *block) {
  block->varA += block->dir;
  if (block->varA < 0)
    block->varA = LEDS_PER_BLOCK - 1;
  if (block->varA >= LEDS_PER_BLOCK)
    block->varA = 0;
}
void decrementVarA(struct RGBblock *block) {
  block->varA -= block->dir;
  if (block->varA < 0)
    block->varA = LEDS_PER_BLOCK - 1;
  if (block->varA >= LEDS_PER_BLOCK)
    block->varA = 0;
}

//-----------------------------
#define colorBlockTypePulse 1
#define colorBlockPulseTime 10   // ms  - cycle time for running routine
#define colorBlockTypeChase  2
#define colorBlockTypeChaseOne 3 // uses varA for currently lit
#define colorBlockCycleTime 300  // ms  - cycle time for running routine
#define colorBlockPulseCycle 200
#define colorBlockTypeComet 4
#define colorBlockTypeComet2 5
#define colorBlockCometCycleTime 150
#define colorBlockTypeBlip  6
#define colorBlockBlipCycleTime 200
#define colorBlockTypeZigZag 7
#define colorBlockZigZagCycleTime 150
#define colorBlockTypeRandomWack 8
#define colorBlockTypeInterleavedPulse 9
#define colorBlockLongPulseCycle 2000
#define colorBlockTypeMergeColor 10
#define colorBlockTypeFlag 11

#define colorBlockChristmas 12
#define colorBlockMarquee 13

#define maxColorBlockTypes 11

#define colorBlockPulseInc  (STD_OUTPUT / (colorBlockPulseCycle / colorBlockPulseTime))
#define colorBlockPulseLongInc  (STD_OUTPUT / (colorBlockLongPulseCycle / colorBlockPulseTime))

void colorBlock(struct RGBblock *block)
{
  int i, b;
  unsigned long val;
  
  if ((theTime - block->lastTime) < block->cycleTime)
        return;
        
  switch (block->routine) {
    
    case colorBlockTypePulse:  // varA -> led, varB -> pulseVal
      val = block->varB;
      val += colorBlockPulseInc * block->dir;
      block->varB = val;
      if ((block->varB > STD_OUTPUT) || (block->varB < 1)) {
        block->dir = -block->dir;
        if (block->varB < 1) {
          block->varB = 0;
          clearRGB(&block->leds[blockmap(block->varA)]);
          incrementVarA(block);
        }
      }
     setRGBFaded(&block->leds[blockmap(block->varA)], &block->colorA, block->varB, STD_OUTPUT);
     break;
     
    case colorBlockTypeInterleavedPulse:
      val = block->varB;
      val += colorBlockPulseLongInc * block->dir;
      block->varB = val;
      if ((block->varB >= STD_OUTPUT) || (block->varB < 1))
        block->dir = -block->dir;
      for (i=0; i < LEDS_PER_BLOCK; i++) {
        if (i%2 == 1)
          setRGBFaded(&block->leds[blockmap(i)], &block->colorA, block->varB, STD_OUTPUT);
        else
          setRGBFaded(&block->leds[blockmap(i)], &block->colorB, STD_OUTPUT - block->varB, STD_OUTPUT);
      }
      break;

    case colorBlockTypeRandomWack:
       for (i=0; i < LEDS_PER_BLOCK; i++) {
         b = random(1,3);
         switch (b) {
            case 1:
              setRGB(&block->leds[i], random(0, STD_OUTPUT/3), random(0, STD_OUTPUT/3), 0);
              break;
            case 2:
              setRGB(&block->leds[i], random(0, STD_OUTPUT/3), 0, random(0, STD_OUTPUT/3));
              break;
            case 3:
              setRGB(&block->leds[i], 0, random(0, STD_OUTPUT/3), random(0, STD_OUTPUT/3));
              break;
         }
       }
     break;

   case colorBlockTypeComet:
      clearRGB(&block->leds[blockmap(block->varA+4)]);
      decrementVarA(block);
      setRGBFaded(&block->leds[blockmap(block->varA)], &block->colorA, STD_OUTPUT, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+1)], &block->colorA, STD_OUTPUT/2, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+2)], &block->colorA, STD_OUTPUT/8, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+3)], &block->colorA, STD_OUTPUT/32, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+4)], &block->colorA, STD_OUTPUT/128, STD_OUTPUT);
      break;
    
    case colorBlockTypeComet2:
      clearRGB(&block->leds[blockmap(block->varA+4)]);
      decrementVarA(block);
      setRGBFaded(&block->leds[blockmap(block->varA)], &block->colorA, STD_OUTPUT, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+1)], &block->colorA, STD_OUTPUT/2, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+2)], &block->colorA, STD_OUTPUT/8, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+3)], &block->colorA, STD_OUTPUT/32, STD_OUTPUT);
      setRGBFaded(&block->leds[blockmap(block->varA+4)], &block->colorA, STD_OUTPUT/128, STD_OUTPUT);
      break;
      
    case colorBlockTypeBlip:
      b = block->varA;
      if (block->varB % 4 == 0)
        b += 5;
      clearRGB(&block->leds[b]);
      
      block->varA += 1;
      if (block->varA > 5)
        block->varA = 0;
      b = block->varA;
      block->varB += 1;
      if (block->varB % 4 == 0)
        b += 5;
      copyRGB(&block->leds[b], &block->colorA);
      break;
      
    case colorBlockTypeZigZag:
      clearRGB(&block->leds[zigzagmap(block->varA)]);
      incrementVarA(block);
      copyRGB(&block->leds[zigzagmap(block->varA)], &block->colorA);
      break;

    case colorBlockTypeChase:
      clearRGB(&block->leds[blockmap(block->varA)]);
      clearRGB(&block->leds[blockmap(block->varA + LEDS_PER_BLOCK / 2)]);
      incrementVarA(block);
      copyRGB(&block->leds[blockmap(block->varA)], &block->colorA);
      copyRGB(&block->leds[blockmap(block->varA + LEDS_PER_BLOCK / 2)], &block->colorB);
        
      break;
    case colorBlockTypeChaseOne:
      clearRGB(&block->leds[block->varA]);
      incrementVarA(block);
      copyRGB(&block->leds[block->varA], &block->colorA);
      break;
    
    case colorBlockTypeFlag:
      incrementVarA(block);
      copyRGB(&block->leds[0], &rgbBLUE);
      copyRGB(&block->leds[2], &rgbBLUE);
      if (block->varA % 2 == 1) {
        copyRGB(&block->leds[1], &rgbBLUE);
        for (i=3; i<5; i++)
          copyRGB(&block->leds[i], &rgbWHITE);
        for (i=5; i<10; i++)
          copyRGB(&block->leds[i], &rgbRED);
      }
      else {
        copyRGB(&block->leds[1], &rgbWHITE);
        for (i=3; i<5; i++)
          copyRGB(&block->leds[i], &rgbRED);
        for (i=5; i<10; i++)
          copyRGB(&block->leds[i], &rgbWHITE);
      }
      break;
    default:
    case colorBlockTypeMergeColor:
      val = block->varB;
      val += colorBlockPulseInc * block->dir;
      block->varB = val;
      if ((block->varB >= STD_OUTPUT) || (block->varB <= 0)) {
        block->dir = -block->dir;
      }
      setRGBMerged(&block->leds[blockmap(block->varA)], &block->colorA, &block->colorB, block->varB, STD_OUTPUT);
      setRGBMerged(&block->leds[blockmap(block->varA+1)], &block->colorB, &block->colorA, block->varB, STD_OUTPUT);
      break;
 }
  block->lastTime = theTime;
}

void clearBlock(struct RGBblock *block, int blockNum, int playRoutine, int cycleTime,
  struct RGBcolor *colorA, RGBcolor *colorB) {
  int i;
  block->blockNum = blockNum;
  block->lastTime = theTime;
  block->varA = 0;
  block->varB = 0;
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



struct RGBblock blockArray[maxColorBlockTypes];
int curBlock = 0;

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
  
  clearBlock(&blockArray[0], 0, colorBlockTypePulse, colorBlockPulseTime, &rgbGREEN, &rgbBLUE);
  clearBlock(&blockArray[1], 0, colorBlockTypeChase, colorBlockCycleTime, &rgbRED, &rgbBLUE);
  clearBlock(&blockArray[2], 0, colorBlockTypeChaseOne, colorBlockCycleTime, &rgbGREEN, &rgbBLUE);
  clearBlock(&blockArray[3], 0, colorBlockTypeComet, colorBlockCometCycleTime, &rgbPURPLE, &rgbBLUE);
  clearBlock(&blockArray[4], 0, colorBlockTypeComet2, colorBlockCometCycleTime, &rgbRED, &rgbBLUE);
  clearBlock(&blockArray[5], 0, colorBlockTypeBlip, colorBlockBlipCycleTime, &rgbPURPLE, &rgbBLUE);
  clearBlock(&blockArray[6], 0, colorBlockTypeZigZag, colorBlockZigZagCycleTime, &rgbRED, &rgbBLUE);
  clearBlock(&blockArray[7], 0, colorBlockTypeRandomWack, colorBlockCycleTime, &rgbRED, &rgbBLUE);
  clearBlock(&blockArray[8], 0, colorBlockTypeInterleavedPulse, colorBlockPulseTime, &rgbRED, &rgbORANGE);
  clearBlock(&blockArray[9], 0, colorBlockTypeInterleavedPulse, colorBlockPulseTime, &rgbGREEN, &rgbORANGE);
  clearBlock(&blockArray[10], 0, colorBlockTypeFlag, colorBlockCycleTime, &rgbRED, &rgbBLUE);
}
colorBlockTypeFlag
void loop() {
  int changed = 0;
  theTime = millis();

  changed = checkSwitch();
  colorBlock(&blockArray[curBlock]);
  blastBlock(&blockArray[curBlock]);

  if (changed) {
    if (switchPos > 0) {
      dir = 1;
      curBlock += dir;
      if (curBlock >= maxColorBlockTypes)
        curBlock = 0;
    }
    else if (switchPos < 0) {
      dir = -1;
      curBlock += dir;
      if (curBlock < 0)
        curBlock = maxColorBlockTypes - 1;
    }
  }
  Tlc.update();
  delay(10);
}
