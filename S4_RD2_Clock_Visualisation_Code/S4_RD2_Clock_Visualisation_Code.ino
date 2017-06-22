#include <Time.h>
#include <TimeLib.h>
#include <SPI.h>

#include "FastLED.h"

#define NUM_LEDS 215 

#define DATA_PIN 3

// Define the array of leds
CRGB leds[NUM_LEDS];

#define ringCount 4 //Total Number of Rings. AdaFruit Disk has 10

//Map rings on disk to indicies.
//This is where all the magic happens. 
//Each represents one of the concentric rings.
uint8_t rings[ringCount][2] = {
    {169,215},    //0 Center Point
    {118,168},    //1
    {61,117},    //2
    {0,61},    //3
};

//For convenience, last ring index
uint8_t lastRing = ringCount - 1;

//Arrays containing degrees between each pixel for each ring.
//This is to speed up later calculations by doing these ahead of time.
//I've given everything with the option of using 360 degres per cirlce 
//or 256. The latter fits nicer into a single byte and *should* be a bit
//faster since it's only doing single byte math and not having to deal with
//negative angles since uint8_t already rolls around to a positive value.
//In the 256 degree per circle methods, 64 is equivalent to 90, 128 to 180, 
//and 192 to 270
float * ringSteps360;
float * ringSteps256;

//360 Degree Helper function to map angle and ring index to pixel
uint16_t angleToPixel360(int16_t angle, uint8_t ring)
{
  if(ring >= ringCount) return 0;
  angle = angle%360;
  if(angle < 0) angle = 360 + angle;
  return rings[ring][0] + int(angle/ringSteps360[ring]);
}

//256 Degree Helper function to map angle and ring index to pixel
uint16_t angleToPixel256(uint8_t angle, uint8_t ring)
{
  if(ring >= ringCount) return 0;
  return rings[ring][0] + int(angle/ringSteps256[ring]);
}

//Fill in the ringSteps arrays for later use.
inline void setupRings()
{
  ringSteps360 = (float*)malloc(ringCount * sizeof(float));
  uint8_t count = 0;
  for(int r=0; r<ringCount; r++)
  {
    count = (rings[r][1] - rings[r][0] + 1);
    ringSteps360[r] = (360.0/float(count));
  }

  ringSteps256 = (float*)malloc(ringCount * sizeof(float));
  count = 0;
  for(int r=0; r<ringCount; r++)
  {
    count = (rings[r][1] - rings[r][0] + 1);
    ringSteps256[r] = (256.0/float(count));
  }
}

//360 Degree helper to set a pixel, given angle and ring index
void setPixel360(int16_t angle, uint8_t ring, CRGB color)
{
  uint16_t pixel = angleToPixel360(angle, ring);
  leds[pixel] = color;
}

//256 Degree helper to set a pixel, given angle and ring index
void setPixel256(uint8_t angle, uint8_t ring, CRGB color)
{
  uint16_t pixel = angleToPixel256(angle, ring);
  leds[pixel] = color;
}

//360 Degree function to draw a line along a given angle from one ring to another
void drawRadius360(int16_t angle, CRGB color, uint8_t startRing, uint8_t endRing)
{
  if(startRing > lastRing) startRing = 0;
  if(endRing > lastRing) endRing = lastRing;
  for(uint8_t r=startRing; r<=endRing; r++)
  {
    setPixel360(angle, r, color);
  }
}

//256 Degree function to draw a line along a given angle from one ring to another
void drawRadius256(uint8_t angle, CRGB color, uint8_t startRing, uint8_t endRing)
{
  if(startRing > lastRing) startRing = 0;
  if(endRing > lastRing) endRing = lastRing;
  for(uint8_t r=startRing; r<=endRing; r++)
  {
    setPixel256(angle, r, color);
  }
}

//360 Degree function to fill a ring from one angle to another (draw an arc)
void fillRing360(uint8_t ring, CRGB color, int16_t startAngle, int16_t endAngle)
{
  uint8_t start = angleToPixel360(startAngle, ring);
  uint8_t end = angleToPixel360(endAngle, ring);
  if(start > end)
  {
    for(int i=start; i<=rings[ring][1]; i++)
    {
      leds[i] = color;
    }
    for(int i=rings[ring][0]; i<=end; i++)
    {
      leds[i] = color;
    }
  }
  else if(start == end)
  {
    for(int i=rings[ring][0]; i<=rings[ring][1]; i++)
    {
      leds[i] = color;
    } 
  }
  else
  {
    for(int i=start; i<=end; i++)
    {
      leds[i] = color;
    }
  }
}

//256 Degree function to fill a ring from one angle to another (draw an arc)
void fillRing256(uint8_t ring, CRGB color, uint8_t startAngle, uint8_t endAngle)
{
  uint8_t start = angleToPixel256(startAngle, ring);
  uint8_t end = angleToPixel256(endAngle, ring);
  if(start > end)
  {
    for(int i=start; i<=rings[ring][1]; i++)
    {
      leds[i] = color;
    }
    for(int i=rings[ring][0]; i<=end; i++)
    {
      leds[i] = color;
    }
  }
  else if(start == end)
  {
    for(int i=rings[ring][0]; i<=rings[ring][1]; i++)
    {
      leds[i] = color;
    } 
  }
  else
  {
    for(int i=start; i<=end; i++)
    {
      leds[i] = color;
    }
  }
}

void setup() { 
  setupRings(); 
  randomSeed(analogRead(0)); 

  Serial.begin(115200);
  LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  FastLED.clear();
}

int randomHolder = 500;
int lastMinuteAngle = 0;
bool resetted = false;

void loop() { 
  //FastLED.clear();


  int angle = map(minute(), 0, 59, 0, 256);
  if(minute() == 0 && second() < 5)
  {
    FastLED.clear();
    resetted = true;
  }
  if(angle > lastMinuteAngle + 5 || angle < lastMinuteAngle - 5)
  {
    updateRow(angle);
    lastMinuteAngle = angle;
  }
}

void updateRow(uint16_t angle)
{
    //FastLED.clear();

    if(random(0, 4) == 0 )
    {
      randomHolder = randomHolder + random(randomHolder < -200 ? 0 : -100, 100);
    }
    else
    {
      randomHolder = randomHolder + random(-100, 50);
    }
    //randomHolder = randomHolder + random(-200, 200);
    if(randomHolder < 0) randomHolder = 0;
    if(randomHolder > 255*4) randomHolder = 255*4;
    lightRow(angle, randomHolder);

    FastLED.show();    
}

void lightRow(uint16_t angle, uint16_t value)
{
  fillRing256(0, CRGB::Black, angle - 5, angle + 5);
  fillRing256(1, CRGB::Black, angle - 5, angle + 5);
  fillRing256(2, CRGB::Black, angle - 5, angle + 5);
  fillRing256(3, CRGB::Black, angle - 5, angle + 5);
  
  uint8_t rows = value / 255;
  uint8_t rest = value % 255;

  CRGB colorLeds[4];
  fill_gradient_RGB(colorLeds, 4, CRGB::Green, CRGB::Red);

  for(uint8_t row = 0; row < rows; row++)
  {
    fillRing256(row, colorLeds[row], angle - 5, angle + 5);
  }
  if(rest == 0) return;

  colorLeds[rows].fadeToBlackBy(rest);
  fillRing256(rows, colorLeds[rows], angle - 5, angle + 5);
}

