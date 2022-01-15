/*
 * main.h
 *
 * Created on: 23.01.2021
 *
 * Copyright 2021 ToMe25
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <Arduino.h>
#include <WebserverHandler.h>
#include <LSM9DS1Handler.h>

extern const char WIFI_SSID[] asm("_binary_wifissid_txt_start");
extern const char WIFI_PASS[] asm("_binary_wifipass_txt_start");
extern const char OTA_PASS[] asm("_binary_otapass_txt_start");

static IPAddress localhost;
static const IPAddress GATEWAY(192, 168, 2, 1);

static constexpr char HOSTNAME[] = "esp-accelerometer";

static const uint MAX_MEASUREMENTS = 90000; // 90000 measurements * 10 values * 4 bytes per float

static LSM9DS1Handler sensorHandler(MAX_MEASUREMENTS);

static const uint PORT = 80;

static WebserverHandler server(PORT, &sensorHandler);

/**
 * The first method called when the ESP starts.
 * Initializes everything the program needs.
 */
void setup();

/**
 * Initializes everything required for ArduinoOTA.
 */
void setupOTA();

/**
 * The method handling everything happening with the WiFi interfaces.
 *
 * @param id	The id of the WiFi event that occurred.
 * @param infi	Information about the WiFi event to be handlerd.
 */
void onWiFiEvent(const WiFiEventId_t id, const WiFiEventInfo_t info);

/**
 * The method doing everything that needs to be repeated for as long as the program runs.
 * Gets called repeatedly by the Arduino framework.
 */
void loop();

#endif /* SRC_MAIN_H_ */
