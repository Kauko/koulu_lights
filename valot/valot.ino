//Jami mode V1 – Multi color; never goes into completely bright (always black spots). Slowish

/* Title: inoise8_pal_demo.ino
 *
 * By: Andrew Tuline
 *
 * Date: August, 2016
 *
 * This short sketch demonstrates some of the functions of FastLED, including:
 *
 * Perlin noise
 * Palettes
 * Palette blending
 * Alternatives to blocking delays
 * Beats (and not the Dr. Dre kind, but rather the sinewave kind)
 *
 * Refer to the FastLED noise.h and lib8tion.h routines for more information on these functions.
 *
 *
 * Recommendations for high performance routines:
 *
 *  Don't use blocking delays, especially if you plan to use buttons for input.
 *  Keep loops to a minimum, and don't use nested loops.
 *  Don't use floating point math. It's slow. Use 8 bit where possible.
 *  Let high school and not elementary school math do the work for you, i.e. don't just count pixels; use sine waves or other math functions instead.
 *  FastLED math functions are faster than built in math functions.
 *
 */


#include "FastLED.h"

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define LED_PIN     4
//#define CLK_PIN     11
#define BRIGHTNESS  230
#define LED_TYPE    WS2812      // Only use the LED_PIN for WS2812's
#define COLOR_ORDER GRB

#define NUM_LEDS 225
#define MAX_MODES 3 // with 4, the modulo will get values of 0, 1, 2, and 3 // https://www.google.com/search?q=0+%25+4&oq=0+%25+4

struct CRGB leds[NUM_LEDS];

static uint16_t dist;         // A random number for our noise generator.
uint16_t scale = 10;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky. ORIGINAALI 30
uint8_t maxChanges = 3;      // Value for blending between palettes. ORIGINAALI 15

CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(ForestColors_p);

uint8_t modeSelect = 0;
const byte interruptPin = 2; // CHANGE to interruptable pin

unsigned long modeChangeTime = 0;

void setup() {
  Serial.begin(57600);
  delay(3000); // 5000

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), flipMode, RISING);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
//  LEDS.addLeds<LED_TYPE,LED_PIN,CLK_PIN, COLOR_ORDER>(leds,NUM_LEDS);

// All modes will have the same brightness at reboot
  LEDS.setBrightness(BRIGHTNESS);

  dist = random16(12345);          // A semi-random number for our noise generator
  allToColor(CRGB::Red);
  delay(300);
  allToColor(CRGB::Yellow);
  delay(300);
  allToColor(CRGB::Green);
  delay(800);
  allToColor(CRGB::Black);
  delay(300);
} // setup()

void loop() {

  unsigned long currentTime = millis();
  
  switch(modeSelect) {
    case 0:   
      if(!maybeFlashColor(currentTime, CRGB::Red)){
        fadingPalette(currentTime);  
      }
      break;
    case 1:
      if(!maybeFlashColor(currentTime, CRGB::Pink)){
        singleColorPulse(currentTime);
      }
      break;
    case 2:
      if(!maybeFlashColor(currentTime, CRGB::Green)){
        rainbowPulse(currentTime);
      }
      break;
    /*case 3:
      if(!maybeFlashCOlor(currentTime, CRGB::Green)){
        stayingPalette(currentTime, CRGBPalette16::ForestColors_p);
      }*/
  }
} // loop()

unsigned long previousReset = 0;
unsigned long RESET_TIMER = 900000; // 900000ms = 15min

void fadingPalette(unsigned long curr) {
//New loop for mode 1
  EVERY_N_MILLISECONDS(20) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // Blend towards the target palette
    fillnoise8();                                                           // Update the LED array with noise at the new location
  }

  EVERY_N_SECONDS(2) {             // Change the target palette to a random one every XX seconds.
    if(curr - previousReset > RESET_TIMER){
      targetPalette = CRGBPalette16(CRGB::Black);
    }else{
      targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
    }
  }

  LEDS.show();
  // Display the LED's at every loop cycle.
}

/*void stayingPalette(unsigned long curr, CRGBPalette16::palette) {

  EVERY_N_MILLISECONDS(20) {
    nblendPaletteTowardPalette(currentPalette, palette, maxChanges);  // Blend towards the target palette
    fillnoise8();                                                           // Update the LED array with noise at the new location
  }

  EVERY_N_SECONDS(10) {             // Change the target palette to a random one every XX seconds.
    targetPalette = CRGBPalette16(CRGB::Black);
  }

  LEDS.show();
  // Display the LED's at every loop cycle.
}*/

unsigned long previousSingleColorPulse = 0;
int PULSE_START_INDEX = 0;
int previousSingleColorIndex = PULSE_START_INDEX;
int singleColorPulseSaturation = 240;
int singleColorPulseHue = 240;  

void singleColorPulse(unsigned long curr) {
  long UPDATE_DELAY = random8(20, 150);

  if (PULSE_START_INDEX == previousSingleColorIndex) {
    singleColorPulseHue = random8(255);
    singleColorPulseSaturation = random8(255);
  }
  
  if (curr - previousSingleColorPulse > UPDATE_DELAY) {

    leds[previousSingleColorIndex - 8] = CRGB::Black;
    leds[previousSingleColorIndex - 7] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 15);
    leds[previousSingleColorIndex - 6] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 45);
    leds[previousSingleColorIndex - 5] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 95);
    leds[previousSingleColorIndex - 4] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 155);
    leds[previousSingleColorIndex - 3] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 205);
    
    leds[previousSingleColorIndex - 2] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 255);
    leds[previousSingleColorIndex - 1] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 255);
    leds[previousSingleColorIndex + 0] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 255);
    leds[previousSingleColorIndex + 1] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 255);
    leds[previousSingleColorIndex + 2] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 255);
    
    leds[previousSingleColorIndex + 3] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 205);
    leds[previousSingleColorIndex + 4] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 155);
    leds[previousSingleColorIndex + 5] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 95);
    leds[previousSingleColorIndex + 6] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 45);
    leds[previousSingleColorIndex + 7] = CHSV(singleColorPulseHue, singleColorPulseSaturation, 14);

    previousSingleColorPulse = curr;
    previousSingleColorIndex++;
    previousSingleColorIndex = previousSingleColorIndex % (NUM_LEDS - 7);
  }

  LEDS.show();
}

unsigned long previousRainbowPulse = 0;
int previousRainbowIndex = PULSE_START_INDEX;  

void rainbowPulse(unsigned long curr) {
  long UPDATE_DELAY = random8(20, 150);

  int rainbowPulseHue = random8(255);
  int rainbowPulseSaturation = random8(255);
  
  if (curr - previousRainbowPulse > UPDATE_DELAY) {

    leds[previousRainbowIndex - 8] = CRGB::Black;
    leds[previousRainbowIndex - 7] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 15);
    leds[previousRainbowIndex - 6] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 45);
    leds[previousRainbowIndex - 5] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 95);
    leds[previousRainbowIndex - 4] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 155);
    leds[previousRainbowIndex - 3] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 205);
    
    leds[previousRainbowIndex - 2] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 255);
    leds[previousRainbowIndex - 1] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 255);
    leds[previousRainbowIndex + 0] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 255);
    leds[previousRainbowIndex + 1] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 255);
    leds[previousRainbowIndex + 2] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 255);
    
    leds[previousRainbowIndex + 3] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 205);
    leds[previousRainbowIndex + 4] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 155);
    leds[previousRainbowIndex + 5] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 95);
    leds[previousRainbowIndex + 6] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 45);
    leds[previousRainbowIndex + 7] = CHSV(rainbowPulseHue, rainbowPulseSaturation, 14);

    previousRainbowPulse = curr;
    previousRainbowIndex++;
    previousRainbowIndex = previousRainbowIndex % (NUM_LEDS - 4);
  }

  LEDS.show();
}

void fillnoise8() {
  for(int i = 0; i < NUM_LEDS; i++) {                                      // Just ONE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i*scale, dist+i*scale) % 255;                  // Get a value from the noise function. I'm using both x and y axis.
    leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  dist += beatsin8(10,1, 4);                                               // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
                                                                           // In some sketches, I've used millis() instead of an incremented counter. Works a treat.
} // fillnoise8()

void flipMode() {
  // Without debounce you have to be inhumanely fast to change the mode just once.
  while(digitalRead(interruptPin) == LOW) { delay(200); }

  //next mode
  modeSelect++;
  //dont go over MAX_MODES -1 and wrap back to 0 for value of modeSelect
  modeSelect = modeSelect % MAX_MODES;

  modeChangeTime = millis();
}

void allToColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i]=color;
  }
  LEDS.show();
}

void allToBlack(){
  //LEDS.clear();

  allToColor(CRGB::Black);
}

boolean isBetween(unsigned long minimi, unsigned long value, unsigned long maximum) {
  if(minimi <= value && maximum > value) { return true; } else { return false; }
}

boolean maybeFlashColor(unsigned long currentTime, CRGB color) {
  unsigned long diff = currentTime - modeChangeTime;
  if(isBetween(0, diff, 1000)) {
    allToColor(color);
    return true;
  } else if (isBetween(1000, diff, 1500)) {
    allToBlack();
    return true;
  } else if (isBetween(1500, diff, 2000)) {
    allToColor(color);
    return true;
  } else if (isBetween(2000, diff, 2500)) {
    allToBlack();
    return true;
  } else if (isBetween(2500, diff, 3000)) {
    allToColor(color);
    return true;
  } else if (isBetween(3000, diff, 3500)) {
    allToBlack();
    return true;
  } else {
    return false;
  }
}

