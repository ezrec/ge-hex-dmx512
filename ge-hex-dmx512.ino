// Copyright (C) 2023, Jason S. McMullan <jason.mcmullan@gmail.com>
// All rights reserved.
//
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// GE Hex LED (as sold by Lowes Hardware in 2022) control.
//
// Wire-up:
//
// Remove GE Hex LED control board from housing, and desolder and remove the
// 8051 controller (U2).
//
// From ESP32:
//  Pin 3.3v to 3v to 5v level converter VA
//  Pin 3.3v to 3v to 5v level converter OE
//  Pin GND to 3v to 5v level converter GND
//  Pin PIN_D to 3v to 5v level converter A1
//  Pin PIN_S to 3v to 5v level converter A2
// From GE Hex LED control board:
//  Test point S to 3v to 5v level converter B1
//  Test point DOUT to 3v to 5v level converter B2
//  Test point GND to 3v to 5v level converter GND
//  Test point V5 to 3v to 5v level converter VB
//
// NOTE: The GE Hex LED will not supply enough power for the ESP-32 wifi,
//       make sure to power the ESP-32 separately via USB!
//
// NOTE: If running more than 6 Hex LEDs, use the GE Hex Led power cable
//       included in the set to 'jumper' the first Hex LED to the last one.
//
// Usage:
//  - Flash firmware to ESP-32
//  - After power-on, connect to the "hex-dmx512" AP (no password), and
//   configure the ESP-32 wifi to connect to your local Wifi network.
//  - Use any DMX-512 Ethernet controller to send DMX-512 packets to your
//  lights!
//

const int PIN_D = 25;
const int PIN_S = 26;
const char *WIFI_AUTOCONNECT_SSID = "hex-dmx512";
const char *WIFI_AUTOCONNECT_PSK = "";

#include <ArtnetWifi.h>
#include <AutoConnect.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

AutoConnect portal;
AutoConnectConfig portal_config;

#define NUM_LEDS 12
CRGB _leds[NUM_LEDS];
CRGB leds[NUM_LEDS + 1];

void set_s(bool value) { digitalWrite(PIN_S, value ? HIGH : LOW); }

const time_t led_show_interval = 10;
time_t next_led_show;

void led_show_now() { next_led_show = 0; }

void led_show() {
  if (time(nullptr) < next_led_show) {
    return;
  }
  // The GE Hex LED has inverted color bits
  for (int i = 0; i < NUM_LEDS; i++) {
    _leds[i] = -leds[i];
  }
  set_s(false);
  FastLED.show();
  delay(50);
  set_s(true);

  next_led_show = time(nullptr) + led_show_interval;
}

ArtnetWifi artnet;
const int startUniverse = 0;
bool sendFrame = 1;
int previousDataLength = 0;

// Process LEDs when receiving a DMX-512 frame.
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence,
                uint8_t *data) {
  sendFrame = 1;
  // set brightness of the whole strip
  if (universe == 15) {
    FastLED.setBrightness(data[0]);
  }
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++) {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < NUM_LEDS) {
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
  previousDataLength = length;

  led_show_now();
}

void onBrightnessChange() {
  // Nothing to do!
}

void setup() {
  Serial.begin(115200);

  portal_config.ota = AC_OTA_BUILTIN;
  portal_config.apid = WIFI_AUTOCONNECT_SSID;
  portal_config.psk = WIFI_AUTOCONNECT_PSK;
  portal.config(portal_config);
  portal.begin();

  artnet.begin();

  pinMode(PIN_S, OUTPUT);
  pinMode(PIN_D, OUTPUT);
  FastLED.addLeds<GS1903, PIN_D>(_leds, NUM_LEDS);

  // onDmxFrame will execute every time a packet is received by the ESP32
  artnet.setArtDmxCallback(onDmxFrame);

  led_show();
}

void ocUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
}

void loop() {
  // we call the read function inside the loop
  artnet.read();
  portal.handleClient();
  ocUpdate();

  led_show();
}
