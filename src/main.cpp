/*
 * main.cpp
 *
 *  Created on: 23.01.2021
 *      Author: ToMe25
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

	WiFi.begin(SSID, PASSWORD);
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
