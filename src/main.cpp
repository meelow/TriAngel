// HARDWARE

//  60 Led/m --> 6led/10cm

// ground plane:
//  40cm x 30cm x 30cm
//  24led x 18led x 18led
//  60 leds

// pyramid:
//  30cm  x 30cm  x 30cm
//  18led x 18led x 18led
//  54 leds

#define FASTLED_ALLOW_INTERRUPTS 0

#include "Arduino.h"
#include "FastLED.h"
#include "Bounce2.h"

FASTLED_USING_NAMESPACE

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS_ALL   114
#define NUM_LEDS_GROUNDPLANE 60

#define BUTTON_PIN_1 2
#define BUTTON_PIN_2 4

// helper macro:
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// a vertex is a point of the pyramid containing 3 pixels, one on each edge
typedef struct
{
  uint16_t pixel1;
  uint16_t pixel2;
  uint16_t pixel3;
  CRGB color;
} type_vertex;

// a edge is a line connecting two vertices. it has a start and an end pixel
typedef struct
{
  uint16_t start;
  uint16_t end;
} type_edge;

// the pyramid contains 4 vertices
type_vertex v1; // ground point near the connector
type_vertex v2; // next ground point following the physical led string
type_vertex v3; // next ground point following the physical led string
type_vertex v4; // pyramid top

// the pyramid contains 6 edges connecting two vertices each:
type_edge edges[6] = {
  {0, 23},    // [0] between v1 and v2
  {24, 41},   // [1] between v2 and v3
  {42, 59},   // [2] between v3 amd v1
  {60, 77},   // [3] between v1 and v4
  {78, 95},   // [4] between v4 and v2  reverse?
  {96, 113}   // [5] betwen v3 and v4
};

// global array of LED pixels for FastLED library:
CRGB leds[NUM_LEDS_ALL];
CRGBPalette16 palette_current;

// Button object for Debounce library:
Bounce button1 = Bounce();
Bounce button2 = Bounce();

// prototypes of all functions:
void setup_buttons();
void setup_leds();
void loop_buttons();
void loop_blink();
void loop_topSpot(uint8_t time);
void loop_fourPoints(uint8_t timeCounter);
void topSpot_setPalette(uint8_t index);

void setup() {
  // 3 second delay for recovery, slowly blinking build in LED
  digitalWrite(LED_BUILTIN, true);
  delay(1000);
  digitalWrite(LED_BUILTIN, false);
  delay(1000);
  digitalWrite(LED_BUILTIN, true);
  delay(1000);
  digitalWrite(LED_BUILTIN, false);

  setup_buttons();
  setup_leds();
  palette_current = RainbowColors_p;

  v1 = {  0, 59, 60, CHSV(HUE_GREEN, 255, 255)};  // ground point near the connector
  v2 = { 23, 24, 95, CHSV(HUE_RED, 255, 255)};    // ground point
  v3 = { 41, 42, 96, CHSV(HUE_BLUE, 255, 255)};   // ground point
  v4 = { 77, 78,113, CHSV(HUE_YELLOW, 255, 255)}; // pyramid top
}


void loop()
{
  static uint8_t timeCounter=0;

  loop_buttons();

  EVERY_N_MILLISECONDS(20)
  {
    //loop_topSpot(timeCounter);
    loop_fourPoints(timeCounter);
    timeCounter++;
    FastLED.show();
  }

  EVERY_N_MILLISECONDS(500) { loop_blink(); }
/*
  digitalWrite(LED_BUILTIN, true);
  delay(1000);
  digitalWrite(LED_BUILTIN, false);
  delay(1000);
*/
}

void setup_buttons()
{
  // Setup both buttons with an internal pull-up :
  pinMode(BUTTON_PIN_1,INPUT_PULLUP);
  button1.attach(BUTTON_PIN_1);
  button1.interval(5); // interval in ms
  pinMode(BUTTON_PIN_2,INPUT_PULLUP);
  button2.attach(BUTTON_PIN_2);
  button2.interval(5); // interval in ms
}

void setup_leds()
{
  // init default variables
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS_ALL).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);

  // Show boot-up animation:
  for(uint8_t i=0; i<255; i++)
  {
   fill_rainbow( leds, NUM_LEDS_ALL, i, 1);
   FastLED.show();
  }

  // fade all leds to black slowly:
  for( int i=0; i<255; i++)
  {
    fadeToBlackBy(leds, NUM_LEDS_ALL, 1);
    FastLED.show();
  }
}

void loop_buttons()
{
  static uint8_t palette_index=0;
  // Update the Bounce instances :
  button1.update();
  button2.update();
  if( button1.fell() )
  {
    palette_index++;
    topSpot_setPalette(palette_index);
  }
  if( button2.fell() )
  {
    palette_index--;
    topSpot_setPalette(palette_index);
  }
  // Turn on the LED if either button is pressed :
  if ( button1.read() == LOW || button2.read() == LOW ) {
    digitalWrite(LED_BUILTIN, HIGH );
  }
}

void loop_blink()
{
  static bool on = true;
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, on);
  on = !on;
}

void paint_edge(type_edge e)
{
  fill_gradient_RGB(leds, e.start, leds[e.start],
                          e.end, leds[e.end]);
}


void loop_fourPoints(uint8_t timeCounter)
{
  CHSVPalette16 YellowAndOrange = CHSVPalette16(
    CHSV(HUE_YELLOW-15, 255, 255),CHSV(HUE_YELLOW-10, 255, 255),CHSV(HUE_YELLOW-5, 255, 255),CHSV(HUE_YELLOW, 255, 255),
    CHSV(HUE_ORANGE-15,255, 255), CHSV(HUE_ORANGE-10,255, 255), CHSV(HUE_ORANGE-5,255, 255), CHSV(HUE_ORANGE,255, 255),
    CHSV(HUE_YELLOW-15, 255, 255),CHSV(HUE_YELLOW-10, 255, 255),CHSV(HUE_YELLOW-5, 255, 255),CHSV(HUE_YELLOW, 255, 255),
    CHSV(HUE_ORANGE-15,255, 255), CHSV(HUE_ORANGE-10,255, 255), CHSV(HUE_ORANGE-5,255, 255), CHSV(HUE_ORANGE,255, 255)
  );
  CHSVPalette16 RedAndPurple = CHSVPalette16(
    CHSV(HUE_RED-15, 255, 255),CHSV(HUE_RED-10, 255, 255),CHSV(HUE_RED-5, 255, 255),CHSV(HUE_RED, 255, 255),
    CHSV(HUE_PINK-15,255, 255), CHSV(HUE_PINK-10,255, 255), CHSV(HUE_PINK-5,255, 255), CHSV(HUE_PINK,255, 255),
    CHSV(HUE_RED-15, 255, 255),CHSV(HUE_RED-10, 255, 255),CHSV(HUE_RED-5, 255, 255),CHSV(HUE_RED, 255, 255),
    CHSV(HUE_PINK-15,255, 255), CHSV(HUE_PINK-10,255, 255), CHSV(HUE_PINK-5,255, 255), CHSV(HUE_PINK,255, 255)
  );
  v4.color = ColorFromPalette(RedAndPurple, timeCounter+0);
  v2.color = ColorFromPalette(YellowAndOrange, timeCounter+20);
  v3.color = ColorFromPalette(YellowAndOrange, timeCounter+20);
  v1.color = ColorFromPalette(RedAndPurple, timeCounter+20);

  leds[v1.pixel1] = v1.color;
  leds[v1.pixel2] = v1.color;
  leds[v1.pixel3] = v1.color;

  leds[v2.pixel1] = v2.color;
  leds[v2.pixel2] = v2.color;
  leds[v2.pixel3] = v2.color;

  leds[v3.pixel1] = v3.color;
  leds[v3.pixel2] = v3.color;
  leds[v3.pixel3] = v3.color;

  leds[v4.pixel1] = v4.color;
  leds[v4.pixel2] = v4.color;
  leds[v4.pixel3] = v4.color;

  for( uint8_t index=0; index<6; index++)
    paint_edge(edges[index]);
}


void loop_topSpot(uint8_t index)
{
  // paint the palette across the ground plane:
  for( int i = 0; i < NUM_LEDS_GROUNDPLANE; i++)
  {
    leds[i] = ColorFromPalette( palette_current, index+i, 255, LINEARBLEND);
  }

  // take a color from the corners of the ground plane
  // paint this color in a gradient to the top of the pyramide
  CRGB topColor = CHSV(HUE_PURPLE,255,255);
  fill_gradient_RGB(leds, NUM_LEDS_GROUNDPLANE, leds[NUM_LEDS_GROUNDPLANE-1], // from ground color to
                          NUM_LEDS_GROUNDPLANE+18-1, topColor);               // top color
  fill_gradient_RGB(leds, NUM_LEDS_GROUNDPLANE+18, topColor,                  // from top color to
                          NUM_LEDS_GROUNDPLANE+18+18-1, leds[24-1]);          // ground color (24 because long side of ground is 24)
  fill_gradient_RGB(leds, NUM_LEDS_GROUNDPLANE+18+18, leds[24+18-1],          //
                          NUM_LEDS_GROUNDPLANE+18+18+18-1, topColor);
}

void topSpot_setPalette(uint8_t index)
{
  // changing the current palette
  index = index%7;

  // fade to black everytime a palette change happens:
  for( int i=0; i<64; i++)
  {
    fadeToBlackBy(leds, NUM_LEDS_ALL, 4);
    FastLED.show();
  }

  // really change the palette:
  switch(index)
  {
    case 0:
      palette_current = RainbowColors_p;
      break;
    case 1:
      palette_current = PartyColors_p;
      break;
    case 2:
      palette_current = HeatColors_p ;
      break;
    case 3:
      palette_current = ForestColors_p ;
      break;
    case 4:
      palette_current = OceanColors_p ;
      break;
    case 5:
      palette_current = LavaColors_p ;
      break;
    case 6:
      palette_current = HeatColors_p  ;
      break;
    case 7:
      palette_current = Rainbow_gp;
    default:
      palette_current = RainbowColors_p;
      break;
  }
}
