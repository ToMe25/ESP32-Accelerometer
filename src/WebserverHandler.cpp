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
#include <regex>

WebserverHandler::WebserverHandler(LSM9DS1Handler *handler, uint16_t port) :
		lsm9ds1(handler), server(port) {
	using namespace std::placeholders;

	// register website pages
	std::function<void(AsyncWebServerRequest*)> on_get_index = bind(&WebserverHandler::on_get_index, this, _1);
	register_url(HTTP_GET, "/", on_get_index);
	register_url(HTTP_GET, "/index.html", on_get_index);

	register_url(HTTP_POST, "/index.html",
			bind(&WebserverHandler::on_post_index, this, _1));

	register_static_handler(HTTP_GET, "/index.css", "text/css", index_css);

	register_static_handler(HTTP_GET, "/recording.js", "text/javascript", recording_js);

	register_static_handler(HTTP_GET, "/calculating.js", "text/javascript", calculating_js);

	server.onNotFound(on_not_found);

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

void WebserverHandler::register_url(const uint8_t http_code, const char *uri,
		std::function<void(AsyncWebServerRequest*)> callback) {
	server.on(uri, http_code, [this, callback](AsyncWebServerRequest *request) {
		callback(request);
	});
}

void WebserverHandler::register_static_handler(const uint8_t http_code,
		const char *uri, const char *content_type, const char *content) {
	server.on(uri, http_code, [content_type, content](AsyncWebServerRequest *request) {
		request->send_P(200, content_type, content);
	});
}

void WebserverHandler::on_not_found(AsyncWebServerRequest *request) {
	std::string response = std::string(not_found_html);
	response = std::regex_replace(response, std::regex("\\$requested"),
			std::string(request->url().c_str()).substr(1));
	request->send(404, "text/html", response.c_str());
}

void WebserverHandler::on_get_index(AsyncWebServerRequest *request) {
	if (!lsm9ds1->isMeasuring() && lsm9ds1->getStoredMeasurements() == 0) {
		request->send_P(200, "text/html", settings_html);
	} else if (lsm9ds1->isMeasuring()) {
		std::string response(recording_html);
		std::ostringstream converter;

		converter << lsm9ds1->getStoredMeasurements();
		response = std::regex_replace(response, std::regex("\\$recorded"),
				converter.str());

		converter.str("");
		converter.clear();
		converter << lsm9ds1->getMeasurements();
		response = std::regex_replace(response, std::regex("\\$recordings"),
				converter.str());

		const uint32_t eta = (lsm9ds1->getMeasurements()
				- lsm9ds1->getStoredMeasurements())
				* lsm9ds1->getMeasuringTime() / 1000;
		response = std::regex_replace(response, std::regex("\\$eta"),
				format_time(eta));

		response = std::regex_replace(response, std::regex("\\$time"),
				format_time(millis() - lsm9ds1->getMeasurementStart()));

		request->send(200, "text/html", response.c_str());
	} else if (lsm9ds1->isCalculating()) {
		std::string response(calculating_html);
		std::ostringstream converter;

		converter << (uint16_t) lsm9ds1->getFilesCalculated();
		response = std::regex_replace(response, std::regex("\\$calculated"),
				converter.str());

		converter.str("");
		converter.clear();
		converter << (uint16_t) lsm9ds1->MEASUREMENT_CSVS;
		response = std::regex_replace(response, std::regex("\\$files"),
				converter.str());

		response = std::regex_replace(response, std::regex("\\$file"),
				lsm9ds1->getFileCalculating());

		response = std::regex_replace(response, std::regex("\\$time"),
				format_time(millis() - lsm9ds1->getCalculationStart()));

		request->send(200, "text/html", response.c_str());
	} else {
		std::string response(results_html);
		std::ostringstream converter;

		converter << lsm9ds1->getStoredMeasurements();
		response = std::regex_replace(response, std::regex("\\$measurements"),
				converter.str());
		response = std::regex_replace(response, std::regex("\\$time"),
				format_time(lsm9ds1->getMeasurementDuration()));

		request->send(200, "text/html", response.c_str());
	}
}

void WebserverHandler::on_post_index(AsyncWebServerRequest *request) {
	if (request->hasParam("start", true)
			&& request->hasParam("measurements", true)
			&& request->hasParam("rate", true)) {
		const uint32_t measurements = atoi(request->arg("measurements").c_str());
		const uint16_t rate = atoi(request->arg("rate").c_str());
		lsm9ds1->measure(measurements, rate);
	}

	if (lsm9ds1->getStoredMeasurements() > 0
			&& (request->hasParam("back", true)
					|| request->hasParam("cancel", true))) {
		lsm9ds1->resetMeasurements();
	}

	on_get_index(request);
}

const std::string WebserverHandler::format_time(uint64_t time_ms) {
	std::ostringstream result;
	uint8_t printed = 0;

	const uint8_t h = time_ms / 3600000;

	uint8_t m;
	if (h == 0) {
		m = time_ms / 60000 % 60;
	} else {
		result << (uint16_t) h << 'h';
		m = uint32_t(round(time_ms / 60000.0)) % 60;
		printed++;
	}

	uint8_t s;
	if (m == 0) {
		s = time_ms / 1000 % 60;
	} else {
		if (printed == 1) {
			result << ' ';
		}
		result << (uint16_t) m << 'm';
		s = uint32_t(round(time_ms / 1000.0)) % 60;
		printed++;
	}

	if (s != 0 && printed < 2) {
		if (printed == 1) {
			result << ' ';
		}
		result << (uint16_t) s << 's';
		printed++;
	}

	const uint16_t ms = time_ms % 1000;

	if (ms != 0 && printed < 2) {
		if (printed == 1) {
			result << ' ';
		}
		result << ms << "ms";
	}

	return result.str();
}
