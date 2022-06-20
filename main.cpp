#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_seesaw.h"
#include "WiFi.h"

//------------ Definitions ------------
#define led_pin 13          // PCB LED
#define us_to_s_factor 1000000ULL  // factor according to example
#define time_to_sleep  10         // seconds to put device in deep sleep 


//------------ Global Variables -------
int baudrate = 115200;
const float battery_level_max = 4.2;
Adafruit_seesaw ss;


// ubidots and esp32 https://help.ubidots.com/en/articles/748067-connect-an-esp32-devkitc-to-ubidots-over-mqtt



void setup() {
  // ---------- pinModes ---------------
  pinMode(led_pin, OUTPUT);                                                               // Set led as output

  // -----------------------------------
  Serial.begin(baudrate);                                                                 // Start serial at baudrate-monitor-speed

  digitalWrite(led_pin, HIGH);                                                            // Turn on PCB LED (will probably be removed due to excessive current draw)


  esp_sleep_enable_timer_wakeup(time_to_sleep * us_to_s_factor);                          // Set deep sleep duration
  Serial.println("EDK will measure every " + String(time_to_sleep) + " seconds");

  // ----------- Measurements ------------
  // Check if moisture sensor is connected and print ss version (from example code)
  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    while(1) delay(1);
  } 
  else {
    Serial.print("seesaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
  }

  // Measure temp and moisture (from example)
  float tempC = ss.getTemp();
  uint16_t capread = ss.touchRead(0);

  Serial.print("Temperature: "); Serial.print(tempC); Serial.println("*C");
  Serial.print("Capacitive: "); Serial.println(capread);
  // ---------------------------------------

  
  // Measure battery_level
  float battery_level_raw = analogRead(A13);
  float battery_level_real = (battery_level_raw / 4095.0) * 2 * 1.1 * 3.3;
  float battery_level_percent = (battery_level_real / battery_level_max) * 100;
  Serial.print("Battery level: "); Serial.print(battery_level_percent); Serial.println(" %");


  digitalWrite(led_pin, LOW);             // Turn off PCB LED

  Serial.println("Entering deep sleep");
  // delay(1000);      // maybe not necessary?
  Serial.flush();
  esp_deep_sleep_start();


}




void loop() {
  // put your main code here, to run repeatedly:
}




// ------------- Functions -------------
