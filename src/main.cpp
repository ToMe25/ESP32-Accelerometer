/*
 * main.cpp
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

#include "main.h"
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		delay(10);
	}

	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true);
	WiFi.onEvent(onWiFiEvent);
	WiFi.setAutoReconnect(true);

	if (!WiFi.config(INADDR_NONE, GATEWAY, INADDR_NONE)) { // setting the local ip here causes setHostname to fail.
		Serial.println("Configuring WiFi failed!");
		return;
	}

	WiFi.begin(WIFI_SSID, WIFI_PASS);

	setupOTA();

	while (!psramInit()) {// Fails in debug mode for some reason
		Serial.println("Failed to initialize psram!");
		delay(1000);
	}

	server.begin();

	MDNS.addService("http", "tcp", 80);

	sensorHandler.begin();
}

void setupOTA() {
	ArduinoOTA.setHostname(HOSTNAME);

	ArduinoOTA.onStart([]() {
		Serial.println("Start updating sketch.");
	});

	ArduinoOTA.onProgress([](uint progress, uint total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});

	ArduinoOTA.onEnd([]() {
		Serial.println("\nUpdate Done.");
	});

	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("OTA Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println("Auth Failed.");
		} else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed.");
		} else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed.");
		} else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed.");
		} else if (error == OTA_END_ERROR) {
			Serial.println("End Failed.");
		}
	});

	ArduinoOTA.begin();

	xTaskCreate([](void *params) {
		while (true) {
			ArduinoOTA.handle();
			delay(10);
		}
	}, "OTA handler", 3000, NULL, 1, NULL);
}

void onWiFiEvent(const WiFiEventId_t id, const WiFiEventInfo_t info) {
	switch (id) {
	case SYSTEM_EVENT_STA_START:
		WiFi.setHostname(HOSTNAME);
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		if (!WiFi.enableIpV6()) {
			Serial.print("Couldn't enable IPv6!");
		}
		break;
	case SYSTEM_EVENT_GOT_IP6:
		Serial.print("IPv6 address: ");
		Serial.println(WiFi.localIPv6());
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		Serial.print("IP address: ");
		Serial.println(localhost = WiFi.localIP());
		break;
	default:
		break;
	}
}

void loop() {
	sensorHandler.loop();
}
