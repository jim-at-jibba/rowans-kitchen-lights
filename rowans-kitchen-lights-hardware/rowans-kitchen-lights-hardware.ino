/*
 * Rowans kitchen Lights
 * 
 * This sketch controls the lights in my daughters play kitchen using mqtt.
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include "settings.h"
 

// This is the onboard led for nodemcu
#define BUILTIN_LED 2

#define PIN 2
#define BUTTON 0
#define NUMPIXELS 25

#define LIGHTOFFDELAY 5000

// Set Up NeoPixel Ring
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

 // MQTT settings
const char* mqtt_server = "192.168.1.217";
const char* rowansKitchenTopic = "rowansKitchen/lights";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

// Button Set Up
int buttonState = LOW;
int thisButtonState = LOW;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;  // the time the button state last switched
unsigned long debounceDelay = 50;    // the state must remain the same for this many millis to register the button press

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  // Turn off LED
//  digitalWrite(BUILTIN_LED, HIGH);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Setting up....");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  turnAllOn();
}

void setup_wifi() {
  delay(10);

  // Start connecting to wifi
  Serial.println();
  Serial.print("Connecting to....");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wifi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Turns on all pixels in ring, setting then to white
void turnAllOn() {
  Serial.print("In All On!!");
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setBrightness(40);
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(255,255,255)); 
    pixels.show(); // This sends the updated pixel color to the hardware.
  }

  // This might not work as it is blocking. Will leave here for the moment.
//  delay(LIGHTOFFDELAY);
//  turnAllOff();
}

// Turns off all pixels in ring
void turnAllOff() {
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); 
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (int i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for(int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }

  Serial.println();

  // Switch on LED if a 1 was received as first character
  if((char)payload[0] == '1') {
    Serial.print("Hello");
    turnAllOn();
  } else if ((char)payload[0] == '2') {
    theaterChaseRainbow(50);  
  } else {
    // Strange bug where all but one led turns off
    // Maybe because of it being a fake ring.
    turnAllOff();
    turnAllOff();
  }
}

void reconnect() {
  // Loop until we reconnect
  while(!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement
      client.publish(rowansKitchenTopic, "Connected to the home mqqt server");
      // and resubscribe
      client.subscribe(rowansKitchenTopic);
    } else {
      Serial.print("failed rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}
void loop() {

  // Debouncing the mother fucking button
  thisButtonState = digitalRead(BUTTON);
  if (thisButtonState != lastButtonState) {
    // reset the timer
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // if the button state has changed
    if (thisButtonState != buttonState) {
      buttonState = thisButtonState;

      if (buttonState == HIGH) {
       Serial.print("Buttton Pushed");
       turnAllOn();
      }
    }
  }

  // persist for next loop iteration
  lastButtonState = thisButtonState;
  
  if(!client.connected()) {
    reconnect();
  }
  client.loop();
}






