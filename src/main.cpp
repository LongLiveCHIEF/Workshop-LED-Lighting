#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

#define LED_DATA 1                         // Data pin to connect to the strip
#define COLOR_ORDER RGB                     // Color Order for chip
#define LED_TYPE WS2812B                    // LED Type
#define NUM_LEDS 32*8                       // Number of LED's

uint8_t max_bright = 128;

struct CRGB leds[NUM_LEDS];
Ticker ticker;

void tick() {
  int state = digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, !state);
}

void configModeCallback (WiFiManager *myWiFiManager){
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  Serial.begin(115200);

  //set builtin led pin as output
  pinMode(LED_BUILTIN, OUTPUT);

  // start ticker with 0.5 because we start in AP mode and try to connect
  digitalWrite(LED_BUILTIN, LOW);
  ticker.attach(0.6, tick);

  //Local Initialization
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you got here, you're connected to WiFi
  Serial.println("connected to configured access point");
  ticker.detach();
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);

  // Setup LED panels
  LEDS.addLeds<LED_TYPE, LED_DATA, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);

}

void loop() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}