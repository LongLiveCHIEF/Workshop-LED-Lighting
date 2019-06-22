#include <Arduino.h>
#include <FastLED.h>

#define LED_DATA 1                         // Data pin to connect to the strip
#define COLOR_ORDER RGB                     // Color Order for chip
#define LED_TYPE WS2812B                    // LED Type
#define NUM_LEDS 32*8                       // Number of LED's

uint8_t max_bright = 128;

struct CRGB leds[NUM_LEDS];

//

void setup() {
  delay(2000); // prevent crashy things
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  LEDS.addLeds<LED_TYPE, LED_DATA, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);

}

void loop() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}