/*
 * WebserverHandler.cpp
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

#include "WebserverHandler.h"

using namespace std;

WebserverHandler::WebserverHandler(int port, LSM9DS1Handler *handler) :
		server(port), lsm9ds1(handler) {
	using namespace std::placeholders;
	// register website pages
	function<void(AsyncWebServerRequest*)> on_get_index = bind(&WebserverHandler::on_get_index, this, _1);
	register_url(HTTP_GET, "/", on_get_index);
	register_url(HTTP_GET, "/index.html", on_get_index);

	register_url(HTTP_POST, "/index.html",
			bind(&WebserverHandler::on_post_index, this, _1));

	server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/css", index_css);
	});

	server.on("/recording.js", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/javascript", recording_js);
	});

	server.on("/calculating.js", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/javascript", calculating_js);
	});

	server.onNotFound([this](AsyncWebServerRequest *request) {
		on_not_found(request);
	});

	// register dynamic pages
	register_url(HTTP_ANY, "/all.csv",
			bind(&LSM9DS1Handler::sendAllCsv, lsm9ds1, _1));

	register_url(HTTP_ANY, "/accelerometer.csv",
			bind(&LSM9DS1Handler::sendAccelerometerCsv, lsm9ds1, _1));

	register_url(HTTP_ANY, "/linear_accelerometer.csv",
			bind(&LSM9DS1Handler::sendLinearAccelerometerCsv, lsm9ds1, _1));

	register_url(HTTP_ANY, "/gyroscope.csv",
			bind(&LSM9DS1Handler::sendGyroscopeCsv, lsm9ds1, _1));

	register_url(HTTP_ANY, "/magnetometer.csv",
			bind(&LSM9DS1Handler::sendMagnetometerCsv, lsm9ds1, _1));

	register_url(HTTP_GET, "/measurements.json",
			bind(&LSM9DS1Handler::sendMeasurementsJson, lsm9ds1, _1));

	register_url(HTTP_GET, "/calculations.json",
			bind(&LSM9DS1Handler::sendCalculationsJson, lsm9ds1, _1));
}

WebserverHandler::~WebserverHandler() {

}

void WebserverHandler::register_url(const uint8_t http_code, const char* url, function<void(AsyncWebServerRequest*)> callback) {
	server.on(url, http_code, [this, callback](AsyncWebServerRequest *request) {
		callback(request);
	});
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
		converter.clear();
		converter << lsm9ds1->getMeasurements();
		replace_all(response, "$recordings", converter.str());
		replace_all(response, "$eta",
				format_time(
						(uint32_t) (lsm9ds1->getMeasurements()
								- lsm9ds1->getStoredMeasurements())
								* lsm9ds1->getMeasuringTime() / 1000));

		uint64_t now_ms = millis();
		replace_all(response, "$time",
				format_time(now_ms - lsm9ds1->getMeasurementStart()));
		request->send(200, "text/html", response.c_str());
	} else if (lsm9ds1->isCalculating()) {
		string response = string(calculating_html);
		ostringstream converter;
		converter << (uint16_t) lsm9ds1->getFilesCalculated();
		replace_all(response, "$calculated", converter.str());

		converter.str("");
		converter.clear();
		converter << (uint16_t) lsm9ds1->MEASUREMENT_CSVS;
		replace_all(response, "$files", converter.str());
		replace_all(response, "$file", lsm9ds1->getFileCalculating());

		uint64_t now_ms = millis();
		replace_all(response, "$time",
				format_time(now_ms - lsm9ds1->getCalculationStart()));
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
