#include <FS.h>
#include <WiFiManager.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define LED_DATA 1                         // Data pin to connect to the strip
#define COLOR_ORDER RGB                     // Color Order for chip
#define LED_TYPE WS2812B                    // LED Type
#define NUM_LEDS 32*8                       // Number of LED's

char mqtt_server[40];
char mqtt_port[6];
char mqtt_server_token[40];

bool shouldSaveConfig = true;

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
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  WiFiManager wifiManager;
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_server_token("token", "mqtt server token", mqtt_server_token, 40);

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_server_token);

  //Local Initialization
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect("workshop-LED-AP")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you got here, you're connected to WiFi
  Serial.println("connected to configured access point");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_server_token, custom_mqtt_server_token.getValue());

  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_server_token"] = mqtt_server_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

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