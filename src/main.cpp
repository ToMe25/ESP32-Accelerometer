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

#include <main.h>

void setup() {
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true);

	if (!WiFi.config(INADDR_NONE, GATEWAY, INADDR_NONE)) { // setting the local ip here causes setHostname to fail.
		Serial.println("Configuring WiFi failed!");
		return;
	}

	WiFi.setHostname(HOSTNAME);

	WiFi.begin(WIFI_SSID, WIFI_PASS);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Establishing a WiFi connection failed!");
		return;
	}

	localhost = WiFi.localIP();

	if (!WiFi.enableIpV6()) {
		Serial.print("Couldn't enable IPv6!");
	}

	Serial.print("IP Address: ");
	Serial.println(localhost);

	if (!psramInit()) {
		while(1) {
			Serial.println("Failed to initialize psram!");
			delay(1000);
		}
	}

	server.begin();

	sensorHandler.begin();
}

void loop() {
	sensorHandler.loop();
}
