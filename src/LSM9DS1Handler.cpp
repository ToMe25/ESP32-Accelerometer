/*
 * LSM9DS1Handler.cpp
 *
 * Created on: 27.01.2021
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

#include "LSM9DS1Handler.h"
#include <sstream>
#include <Adafruit_AHRS_NXPFusion.h>

LSM9DS1Handler::LSM9DS1Handler(uint32_t max_measurements) :
		measurements_max(max_measurements), data_size(
				max_measurements * VALUES_PER_MEASUREMENT * sizeof(float)) {

}

LSM9DS1Handler::~LSM9DS1Handler() {

}

void LSM9DS1Handler::setupSensor() {
	lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);

	lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

	lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

void LSM9DS1Handler::begin() {
	data = (float*) ps_malloc(data_size);

	while (!lsm.begin()) {
		Serial.println("Failed to initialize the LSM9DS1. Check your wiring!");
		delay(1000);
	}

#ifdef LSM9DS1_I2C
	Serial.println("Connected to the lsm9ds1 using I2C.");
#endif

#ifdef LSM9DS1_SPI
	Serial.println("Connected to the lsm9ds1 using SPI.");
#endif

	setupSensor();

#ifdef LSM9DS1_I2C
	Wire.setClock(400000);
#endif
}

void LSM9DS1Handler::loop() {
	if (!measuring) {
		delay(1);
		return;
	} else if (measurements_stored >= measurements) {
		measuring = false;
		measurement_duration = millis() - measurement_start;
		measuring_time = measurement_duration * 1000 / measurements;
		return;
	}

	uint64_t start_us = micros();

	lsm.read();

	sensors_event_t a, m, g, t;

	lsm.getEvent(&a, &m, &g, &t);

	if (measurements_stored > 0
			&& a.timestamp - measurement_start
					== uint32_t(
							data[(measurements_stored - 1)
									* VALUES_PER_MEASUREMENT])) {
		return;
	}

	data[measurements_stored * VALUES_PER_MEASUREMENT] = a.timestamp
			- measurement_start;

	data[measurements_stored * VALUES_PER_MEASUREMENT + 1] = a.acceleration.x;
	data[measurements_stored * VALUES_PER_MEASUREMENT + 2] = a.acceleration.y;
	data[measurements_stored * VALUES_PER_MEASUREMENT + 3] = a.acceleration.z;

	data[measurements_stored * VALUES_PER_MEASUREMENT + 4] = g.gyro.x;
	data[measurements_stored * VALUES_PER_MEASUREMENT + 5] = g.gyro.y;
	data[measurements_stored * VALUES_PER_MEASUREMENT + 6] = g.gyro.z;

	data[measurements_stored * VALUES_PER_MEASUREMENT + 7] = m.magnetic.x;
	data[measurements_stored * VALUES_PER_MEASUREMENT + 8] = m.magnetic.y;
	data[measurements_stored * VALUES_PER_MEASUREMENT + 9] = m.magnetic.z;

	measurements_stored++;

	if (measurements_stored > 1) {
		measuring_time = round(
				(data[(measurements_stored - 1) * VALUES_PER_MEASUREMENT]
						- data[(measurements_stored
								- min(measurements_stored, uint32_t(10)))
								* VALUES_PER_MEASUREMENT])
						/ (min(measurements_stored, uint32_t(10)) - 1));
	}

	delayMicroseconds(
			measuring_time_target
					- min(measuring_time_target, uint32_t(micros() - start_us)));
}

void LSM9DS1Handler::measure(uint32_t measurements, uint16_t freq) {
	measurements = min(measurements_max, measurements);
	freq = max((uint16_t) 1, freq);

	measurements_stored = 0;
	frequency = freq;
	measuring_time_target = measuring_time = 1000000 / freq;
	LSM9DS1Handler::measurements = measurements;
	measuring = true;

	measurement_start = millis();
}

std::function<char* (uint8_t, uint32_t)> LSM9DS1Handler::getAllGenerator() const {
	const std::function<char* (uint8_t, uint32_t)> accel_gen =
			getDataContentGenerator(1);
	const std::function<char* (uint8_t, uint32_t)> linear_accel_gen =
			getLinearAccelerationGenerator();
	const std::function<char* (uint8_t, uint32_t)> gyromag_gen =
			getDataContentGenerator(4, 6);
	return [this, accel_gen, linear_accel_gen, gyromag_gen](
			uint8_t separator_char, uint32_t position) {
		char *content = new char[156] { 0 };
		char *buffer = accel_gen(separator_char, position);
		strcat(content, buffer);
		delete[] buffer;

		buffer = linear_accel_gen(separator_char, position);
		strcat(content, buffer);
		delete[] buffer;

		buffer = gyromag_gen(separator_char, position);
		strcat(content, buffer);
		delete[] buffer;

		return content;
	};
}

std::function<char* (uint8_t, uint32_t)> LSM9DS1Handler::getLinearAccelerationGenerator() const {
	Adafruit_NXPSensorFusion filter;
	filter.begin(round(1000000.0 / measuring_time));

	return [this, filter](uint8_t separator_char, uint32_t position) mutable {
		char *line = new char[39] { };

		const float gx = data[position * VALUES_PER_MEASUREMENT + 4]
				* SENSORS_RADS_TO_DPS;
		const float gy = data[position * VALUES_PER_MEASUREMENT + 5]
				* SENSORS_RADS_TO_DPS;
		const float gz = data[position * VALUES_PER_MEASUREMENT + 6]
				* SENSORS_RADS_TO_DPS;

		const float ax = data[position * VALUES_PER_MEASUREMENT + 1]
				/ SENSORS_GRAVITY_EARTH;
		const float ay = data[position * VALUES_PER_MEASUREMENT + 2]
				/ SENSORS_GRAVITY_EARTH;
		const float az = data[position * VALUES_PER_MEASUREMENT + 3]
				/ SENSORS_GRAVITY_EARTH;

		filter.update(gx, gy, gz, ax, ay, az,
				data[position * VALUES_PER_MEASUREMENT + 7],
				data[position * VALUES_PER_MEASUREMENT + 8],
				data[position * VALUES_PER_MEASUREMENT + 9]);

		float lin_x, lin_y, lin_z;
		filter.getLinearAcceleration(&lin_x, &lin_y, &lin_z);

		sprintf(line, "%c%f%c%f%c%f", separator_char,
				lin_x * SENSORS_GRAVITY_EARTH, separator_char,
				lin_y * SENSORS_GRAVITY_EARTH, separator_char,
				lin_z * SENSORS_GRAVITY_EARTH);

		return line;
	};
}

std::function<char* (uint8_t, uint32_t)> LSM9DS1Handler::getDataContentGenerator(
		const uint8_t index, uint8_t channels) const {
	channels = min(VALUES_PER_MEASUREMENT, channels);

	return [this, index, channels](uint8_t separator_char, uint32_t position) {
		char *line = new char[channels * 13] { };

		uint16_t length = 0;
		for (uint8_t i = 0; i < channels; i++) {
			length += sprintf(line + length, "%c%f", separator_char,
					data[position * VALUES_PER_MEASUREMENT + index + i]);
		}

		return line;
	};
}

void LSM9DS1Handler::sendMeasurementsCsv(AsyncWebServerRequest *request,
		const std::function<char* (uint8_t, uint32_t)> content_generator,
		const std::vector<const char*> headers) const {

	uint8_t separator_char = ',';
	if (request->hasArg("separator")) {
		separator_char = request->arg("separator")[0];
	}

	uint32_t position = 0;
	std::string buf = "";
	request->sendChunked("text/csv",
			[this, separator_char, position, buf, content_generator, headers](
					uint8_t *buffer, const size_t maxlen,
					const size_t idx) mutable {
				return generateMeasurementCsv(separator_char, position, buf,
						content_generator, headers, buffer, maxlen);
			});
}

void LSM9DS1Handler::sendMeasurementsJson(AsyncWebServerRequest *request) const {
	char *measurements = new char[50];
	uint32_t measuring_time = measurement_duration;
	if (measuring) {
		measuring_time = uint32_t(millis() - measurement_start);
	}
	sprintf(measurements, "{\"measurements\": %u, \"time\": %u}", measurements_stored, measuring_time);
	request->send(200, "application/json", measurements);
}

size_t LSM9DS1Handler::generateMeasurementCsv(const uint8_t separator_char, uint32_t &position,
			std::string &buf,
			const std::function<char* (uint8_t, uint32_t)> content_generator,
			const std::vector<const char*> headers, uint8_t *buffer,
			size_t maxlen) const {
	maxlen = min(maxlen, (size_t) 1200);
	size_t length = 0;
	char *response = new char[maxlen + 200] { }; // include some buffer in case the line doesn't end exactly at the end of the packet size.

	length = sprintf(response, buf.c_str());
	buf.clear();

	if (position == 0) {
		strcat(response, "Time(ms)");
		length = 8;

		for (uint8_t i = 0; i < headers.size(); i++) {
			length += sprintf(response + length, "%c%s", separator_char, headers[i]);
		}

		length += sprintf(response + length, "%c", '\n');
	}

	if (measurements_stored == 0) {
		position++;
	}

	while (length < maxlen && position < measurements_stored) {
		char *content = content_generator(separator_char, position);

		length += sprintf(response + length, "%d%s%c",
				uint32_t(data[position * VALUES_PER_MEASUREMENT]),
				content, '\n');

		delete[] content;
		position++;
	}

	if (length > maxlen) {
		buf.append(std::string(response).substr(maxlen));
		length = maxlen;
	}

	for (size_t i = 0; i < length && i < maxlen; i++) {
		buffer[i] = response[i];
	}
	delete[] response;

	return min(length, maxlen);
}
