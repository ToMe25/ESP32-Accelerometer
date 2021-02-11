/*
 * WebserverHandler.cpp
 *
 *  Created on: 23.01.2021
 *      Author: ToMe25
 */

#include <WebserverHandler.h>

using namespace std;

WebserverHandler::WebserverHandler(int port, LSM9DS1Handler *handler) :
		server(port), lsm9ds1(handler) {
	// register website pages
	server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
		on_get_index(request);
	});

	server.on("/index.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
		on_get_index(request);
	});

	server.on("/index.html", HTTP_POST, [this](AsyncWebServerRequest *request) {
		on_post_index(request);
	});

	server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/css", index_css);
	});

	server.onNotFound([this](AsyncWebServerRequest *request) {
		on_not_found(request);
	});

	server.on("/all.csv", HTTP_ANY,
			[this](AsyncWebServerRequest *request) {
				lsm9ds1->sendAllCsv(request);
			});

	server.on("/accelerometer.csv", HTTP_ANY,
			[this](AsyncWebServerRequest *request) {
				lsm9ds1->sendAccelerometerCsv(request);
			});

	server.on("/linear_accelerometer.csv", HTTP_ANY,
			[this](AsyncWebServerRequest *request) {
				lsm9ds1->sendLinearAccelerometerCsv(request);
			});

	server.on("/gyroscope.csv", HTTP_ANY,
			[this](AsyncWebServerRequest *request) {
				lsm9ds1->sendGyroscopeCsv(request);
			});

	server.on("/magnetometer.csv", HTTP_ANY,
			[this](AsyncWebServerRequest *request) {
				lsm9ds1->sendMagnetometerCsv(request);
			});
}

WebserverHandler::~WebserverHandler() {

}

void WebserverHandler::on_not_found(AsyncWebServerRequest *request) {
	string response = string(not_found_html);
	replace_all(response, "$requested",
			string(request->url().c_str()).substr(1));
	request->send(404, "text/html", response.c_str());
}

void WebserverHandler::on_get_index(AsyncWebServerRequest *request) {
	if (!lsm9ds1->isMeasuring() && lsm9ds1->getStoredMeasurements() == 0) {
		request->send(200, "text/html", settings_html);
	} else if (lsm9ds1->isMeasuring()) {
		string response = string(recording_html);
		ostringstream converter;
		converter << lsm9ds1->getStoredMeasurements();
		replace_all(response, "$recorded", converter.str());

		converter.str("");
		converter << lsm9ds1->getMeasurements();
		replace_all(response, "$recordings", converter.str());
		replace_all(response, "$eta",
				format_time(
						uint(lsm9ds1->getMeasurements()
								- lsm9ds1->getStoredMeasurements())
								* lsm9ds1->getMeasuringTime()));

		uint64_t now_ms = millis();
		replace_all(response, "$time",
				format_time(now_ms - lsm9ds1->getMeasurementStart()));
		request->send(200, "text/html", response.c_str());
	} else {
		string response = string(results_html);
		ostringstream converter;
		converter << lsm9ds1->getStoredMeasurements();
		replace_all(response, "$measurements", converter.str());
		replace_all(response, "$time",
				format_time(lsm9ds1->getMeasurementDuration()));
		request->send(200, "text/html", response.c_str());
	}
}

void WebserverHandler::on_post_index(AsyncWebServerRequest *request) {
	if (request->hasParam("start", true)
			&& request->hasParam("measurements", true)
			&& request->hasParam("rate", true)) {
		istringstream converter(request->arg("measurements").c_str());
		uint measurements = 0;
		converter >> measurements;
		converter.clear();
		converter.str(request->arg("rate").c_str());
		uint16_t rate = 0;
		converter >> rate;
		lsm9ds1->measure(measurements, rate);
	}

	if (lsm9ds1->getStoredMeasurements() > 0
			&& (request->hasParam("back", true)
					|| request->hasParam("cancel", true))) {
		lsm9ds1->resetMeasurements();
	}

	on_get_index(request);
}

void WebserverHandler::replace_all(string &str, const string &from,
		const string &to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

const char* WebserverHandler::format_time(uint64_t time_ms) {
	char *result = new char[10] {};

	uint8_t printed = 0;
	uint8_t length = 0;

	uint8_t h = time_ms / 3600000;

	uint8_t m;
	if (h == 0) {
		m = time_ms / 60000 % 60;
	} else {
		length = sprintf(result, "%dh", h);
		m = uint(round(time_ms / 60000.0)) % 60;
		printed++;
	}

	uint8_t s;
	if (m == 0) {
		s = time_ms / 1000 % 60;
	} else {
		length += sprintf(result + length, printed == 1 ? " %dm" : "%dm", m);
		s = uint(round(time_ms / 1000.0)) % 60;
		printed++;
	}

	if (s != 0 && printed < 2) {
		length += sprintf(result + length, printed == 1 ? " %ds" : "%ds", s);
		printed++;
	}

	uint16_t ms = time_ms % 1000;

	if (ms != 0 && printed < 2) {
		sprintf(result + length, printed == 1 ? " %dms" : "%dms", ms);
	}

	return result;
}
