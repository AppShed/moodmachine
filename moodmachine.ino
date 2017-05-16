/*
  This sketch modifies the aREST Library for the ESP8266 WiFi chip.
  This example illustrate the cloud part of aREST that makes the board accessible from anywhere
  See the README file for more details.

  Written in 2016 by Marco Schwartz under a GPL license.
  Modified by Torsten Stauch in 2017
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>
#include <Adafruit_NeoPixel.h>
#include <Math.h>


#define PIN 14
int numLEDs = 24;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLEDs, PIN, NEO_GRB + NEO_KHZ800);




// Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Create aREST instance
aREST rest = aREST(client);

// aREST Pro key (that you can get at dashboard.arest.io)
char * key = "your_pro_key";

// WiFi parameters
const char* ssid = "AppShed";
const char* password = "password";

// Variables to be exposed to the API
int count_red = 0;
int count_orange = 0;
int count_green = 0;


// Declare functions to be exposed to the API
int red(String command);
int orange(String command);
int green(String command);

// Colors
uint32_t cRed = strip.Color(255, 0, 0);
uint32_t cOrange = strip.Color(208, 59, 0);
uint32_t cGreen = strip.Color(0, 255, 0);
uint32_t cWaiting = strip.Color(53, 53, 53);

// waiting
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 50;           // interval at which to blink (milliseconds)
int ledNum = 0;             // ledState used to set the LED


// Functions
void callback(char* topic, byte* payload, unsigned int length);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  Serial.println("Sketch: capetalk");
  Serial.println("");
  Serial.println("INSTRUCTIONS");
  Serial.println("");
  Serial.println("NeoPixel on GPIO 14, D5");
  Serial.println("Requires a WiFi netowrk called AppShed, password appshedrocks");
  Serial.println("");
  Serial.println("aREST ID");
  Serial.println("101798");



  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  
  // Set aREST key
  rest.setKey(key, client);

  // Set callback
  client.setCallback(callback);

  rest.variable("count_red",&count_red);
  rest.variable("count_orange",&count_orange);
  rest.variable("count_green",&count_green);


  rest.function("red",red);  
  rest.function("orange",orange);  
  rest.function("green",green);  


  // Give name to device
  rest.set_name("capetalk");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Set output topic
  char* out_topic = rest.get_topic();

  colorWipe(cRed, 50);
  colorWipe(cOrange, 50);
  colorWipe(cGreen, 50);


}

void loop() {

  unsigned long currentMillis = millis();

  // if no values, show the waiting animation
  if (currentMillis - previousMillis >= interval) {

    // save the last time you blinked the LED
    previousMillis = currentMillis;
    
    if(count_red == 0 && count_orange == 0 && count_green == 0){
      showWaiting();
    } else{
      mood("");
    }
  }
    
  // Connect to the cloud
  rest.handle(client);

}


// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  rest.handle_callback(client, topic, payload, length);

}



void showWaiting(){

      // turn all off
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0);
      }  
      // turn one on
      strip.setPixelColor(ledNum, cWaiting);

      strip.show();
  
      ledNum++;
      if(ledNum == numLEDs)
        ledNum = 0;
  
}

int red(String command){
 count_red++;
 return count_red;
}

int orange(String command){
 count_orange++;
 return count_orange;
}

int green(String command){
 count_green++;
 return count_green;
}




// show mood
int mood(String command){

  int total_votes = count_red + count_orange + count_green;

  int num_red = numberToShow(count_red,total_votes);
  int num_orange = numberToShow(count_orange,total_votes);
  int num_green = numberToShow(count_green,total_votes);


  // make sure 24 are shown
  int num_total = num_red + num_orange + num_green;
  if(num_total < 24){
    if(num_red > num_orange && num_red > num_green)
      num_red += (24-num_total);
    else if (num_orange > num_green)
      num_orange += (24-num_total);
    else
      num_green += (24-num_total);      
  }


  for(uint16_t i=0; i<num_red; i++) {
    strip.setPixelColor(i, cRed);
  }  
  for(uint16_t i=num_red; i<(num_red+num_orange); i++) {
    strip.setPixelColor(i, cOrange);
  }  
  for(uint16_t i=(num_red+num_orange); i<(num_red+num_orange+num_green); i++) {
    strip.setPixelColor(i, cGreen);
  }  

  strip.show();

  return 1;
}


int numberToShow(float c, float t){
  int num = 0;
  // show at least one led if there are votes for this color
  if(c > 0){

    float p = (c / t * 100) + 0.5;

    num = (int)((p/100*24) + 0.5);

    if(num == 0)
      num = 1;      
  }

  return num;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}




