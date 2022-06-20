#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_seesaw.h"
#include "WiFi.h"
#include "UbidotsEsp32Mqtt.h"

//------------ Definitions ------------
#define led_pin 13          // PCB LED
#define us_to_s_factor 1000000ULL  // factor according to example
#define time_to_sleep  900         // seconds to put device in deep sleep 


//------------ Global Variables -------
int baudrate = 115200;
Adafruit_seesaw ss;

const float battery_level_max = 4.2;

const char *UBIDOTS_TOKEN = "";
const char *WIFI_SSID = "";
const char *WIFI_PASS = "";
const char *DEVICE_LABEL = "EDK_ESP32";
const char *VARIABLE_LABEL1 = "EDK_moisture";
const char *VARIABLE_LABEL2 = "EDK_voltage";
const char *VARIABLE_LABEL3 = "EDK_temp";

bool buffer_sent = false;

Ubidots ubidots(UBIDOTS_TOKEN);

// ubidots and esp32 https://help.ubidots.com/en/articles/748067-connect-an-esp32-devkitc-to-ubidots-over-mqtt


// ------------- Functions -------------
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}




void setup() {
  // ---------- pinModes ---------------
  pinMode(led_pin, OUTPUT);                                                               // Set led as output

  // -----------------------------------
  Serial.begin(baudrate);                                                                 // Start serial at baudrate-monitor-speed

  digitalWrite(led_pin, HIGH);                                                          // Turn on PCB LED (will probably be removed due to excessive current draw)

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


  //  --- Connect to Wifi and Ubidots---
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);   //connect to wifi using wifi ssid and pass
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  if (ubidots.connected()) {
    Serial.println("Connected to wifi");
  }

  while (!buffer_sent) {
    while (!ubidots.connected()) {
      ubidots.reconnect();
      }
    if (ubidots.connected()) {
      ubidots.add(VARIABLE_LABEL1, capread);                // Send moisture data
      ubidots.add(VARIABLE_LABEL2, battery_level_percent);  // Send battery voltage data
      ubidots.add(VARIABLE_LABEL3, tempC);                  // Send temperature data
      ubidots.publish(DEVICE_LABEL);

      buffer_sent = ubidots.publish();
    ubidots.loop();
   }
  }

  if (buffer_sent) {
    Serial.println("Data sent to Ubidots");
  }
  ubidots.disconnect();
  // --------------------------------------
  digitalWrite(led_pin, LOW);             // Turn off PCB LED

  Serial.println("Entering deep sleep");
  // delay(1000);      // maybe not necessary?
  Serial.flush();
  esp_deep_sleep_start();


}


void loop() {
  // put your main code here, to run repeatedly:
}
