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

#define NUM_LEDS 150
#define MAX_MODES 2 // with 4, the modulo will get values of 0, 1, 2, and 3 // https://www.google.com/search?q=0+%25+4&oq=0+%25+4

struct CRGB leds[NUM_LEDS];

static uint16_t dist;         // A random number for our noise generator.
uint16_t scale = 10;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky. ORIGINAALI 30
uint8_t maxChanges = 3;      // Value for blending between palettes. ORIGINAALI 15

CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(OceanColors_p);

uint8_t modeSelect = 0;
const byte interruptPin = 2; // CHANGE to interruptable pin

void setup() {
  Serial.begin(57600);
  delay(5000); //Originaali 3000

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), flipMode, RISING);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
//  LEDS.addLeds<LED_TYPE,LED_PIN,CLK_PIN, COLOR_ORDER>(leds,NUM_LEDS);

// All modes will have the same brightness at reboot
  LEDS.setBrightness(BRIGHTNESS);

  dist = random16(12345);          // A semi-random number for our noise generator
  allToColor(CRGB::Red, false);
  delay(300);
  allToColor(CRGB::Yellow, false);
  delay(300);
  allToColor(CRGB::Green, false);
  delay(300);
  allToColor(CRGB::Black, true);
  delay(500);
} // setup()

//Instead of having just one loop, we make optional "main loops" that can be selected.
void loop() {

  switch(modeSelect) {
    case 0:
      fadingPalette();
      break;
    case 1:
      stepToGreen();
      break;
  }
} // loop()

void fadingPalette() {
//New loop for mode 1
  EVERY_N_MILLISECONDS(20) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // Blend towards the target palette
    fillnoise8();                                                           // Update the LED array with noise at the new location
  }

  EVERY_N_SECONDS(2) {             // Change the target palette to a random one every XX seconds.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
  }
  LEDS.show();
  // Display the LED's at every loop cycle.
}

void stepToBlack() {
  allToColor(CRGB::Black, true);
}


void stepToGreen() {
  allToColor(CRGB::Green, true);
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
  //debounce button to avoid double, comment out next line if you do not want delay in animation. The delay is supposed to last about as long as a normal button press.
  while(digitalRead(interruptPin) == LOW) { delay(200); }

  //next mode
  modeSelect++;
  //dont go over MAX_MODES -1 and wrap back to 0 for value of modeSelect
  modeSelect = modeSelect % MAX_MODES;

  //fadeAllToColor(CRGB::Black);
}

/*void fadeAllToColor(CRGB color) {
  targetPalette=color;
  nblendPaletteTowardPalette(currentPalette, targetPalette, NUM_LEDS);
  LEDS.show();
  delay(5);
}*/

void allToColor(CRGB color, boolean showIndividual) {
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i]=color;
    if(showIndividual) {
      LEDS.show();
      delay(30);
      }
  }
  if (!showIndividual) { 
    LEDS.show(); 
    }
}

