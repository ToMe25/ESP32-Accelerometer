/*
 * main.h
 *
 *  Created on: 23.01.2021
 *      Author: ToMe25
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <Arduino.h>
#include <WebserverHandler.h>
#include <LSM9DS1Handler.h>

extern const char WIFI_SSID[] asm("_binary_wifissid_txt_start");
extern const char WIFI_PASS[] asm("_binary_wifipass_txt_start");

static IPAddress localhost;
static const IPAddress GATEWAY(192, 168, 2, 1);

static constexpr char HOSTNAME[] = "esp-accelerometer";

static const uint MAX_MEASUREMENTS = 90000; // 90000 measurements * 10 values * 4 bytes per float

static LSM9DS1Handler sensorHandler(MAX_MEASUREMENTS);

static const uint PORT = 80;

static WebserverHandler server(PORT, &sensorHandler);

#endif
