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

#include <LSM9DS1Handler.h>
#include <sstream>
#include <Adafruit_AHRS_NXPFusion.h>

using namespace std;

LSM9DS1Handler::LSM9DS1Handler(uint max_measurements) :
		measurements_max(max_measurements), data_size(
				max_measurements * values_per_measurement * sizeof(float)) {

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

	if (!lsm.begin()) {
		while (1) {
			Serial.println(
					"Failed to initialize the LSM9DS1. Check your wiring!");
			delay(1000);
		}
	}

	setupSensor();
}

void LSM9DS1Handler::loop() {
	if (!measuring) {
		delay(1);
		return;
	} else if (measurements_stored >= measurements) {
		measuring = false;
		measurement_duration = millis() - measurement_start;
		return;
	}

	uint64_t start_ms = millis();

	lsm.read();

	sensors_event_t a, m, g, t;

	lsm.getEvent(&a, &m, &g, &t);

	data[measurements_stored * values_per_measurement] = a.timestamp
			- measurement_start;

	data[measurements_stored * values_per_measurement + 1] = a.acceleration.x;
	data[measurements_stored * values_per_measurement + 2] = a.acceleration.y;
	data[measurements_stored * values_per_measurement + 3] = a.acceleration.z;

	data[measurements_stored * values_per_measurement + 4] = g.gyro.x;
	data[measurements_stored * values_per_measurement + 5] = g.gyro.y;
	data[measurements_stored * values_per_measurement + 6] = g.gyro.z;

	data[measurements_stored * values_per_measurement + 7] = m.magnetic.x;
	data[measurements_stored * values_per_measurement + 8] = m.magnetic.y;
	data[measurements_stored * values_per_measurement + 9] = m.magnetic.z;

	measurements_stored++;

	if (measurements_stored > 1) {
		measuring_time =
				(measuring_time * 2
						+ uint(
								data[(measurements_stored - 1)
										* values_per_measurement])
						- uint(
								data[(measurements_stored - 2)
										* values_per_measurement])) / 3;
	}

	delay(
			measuring_time_target
					- min(measuring_time_target,
							uint16_t(millis() - start_ms)));
}

void LSM9DS1Handler::measure(uint measurements, uint16_t freq) {
	measurements = min(measurements_max, measurements);
	freq = max(uint16_t(1), freq);

	measurements_stored = 0;
	frequency = freq;
	measuring_time_target = measuring_time = 1000 / freq;
	LSM9DS1Handler::measurements = measurements;
	measuring = true;

	measurement_start = millis();
}

function<char* (byte, uint)> LSM9DS1Handler::getLinearAccelerationGenerator() const {
	Adafruit_NXPSensorFusion filter;
	filter.begin(round(1000.0 / measuring_time));

	return [this, filter](byte separator_char, uint position) mutable {
		char *line = new char[39] { };

		const float gx = data[position * values_per_measurement + 4]
				* SENSORS_RADS_TO_DPS;
		const float gy = data[position * values_per_measurement + 5]
				* SENSORS_RADS_TO_DPS;
		const float gz = data[position * values_per_measurement + 6]
				* SENSORS_RADS_TO_DPS;

		const float ax = data[position * values_per_measurement + 1]
				/ SENSORS_GRAVITY_EARTH;
		const float ay = data[position * values_per_measurement + 2]
				/ SENSORS_GRAVITY_EARTH;
		const float az = data[position * values_per_measurement + 3]
				/ SENSORS_GRAVITY_EARTH;

		filter.update(gx, gy, gz, ax, ay, az,
				data[position * values_per_measurement + 7],
				data[position * values_per_measurement + 8],
				data[position * values_per_measurement + 9]);

		float lin_x, lin_y, lin_z;
		filter.getLinearAcceleration(&lin_x, &lin_y, &lin_z);

		sprintf(line, "%c%f%c%f%c%f", separator_char,
				lin_x * SENSORS_GRAVITY_EARTH, separator_char,
				lin_y * SENSORS_GRAVITY_EARTH, separator_char,
				lin_z * SENSORS_GRAVITY_EARTH);

		return line;
	};
}

function<char* (byte, uint)> LSM9DS1Handler::getDataContentGenerator(
		const uint8_t index, uint8_t channels) const {
	channels = min(values_per_measurement, channels);

	return [this, index, channels](byte separator_char, uint position) {
		char *line = new char[channels * 13] { };

		uint16_t length = 0;
		for (uint8_t i = 0; i < channels; i++) {
			length += sprintf(line + length, "%c%f", separator_char,
					data[position * values_per_measurement + index + i]);
		}

		return line;
	};
}

void LSM9DS1Handler::sendAllCsv(AsyncWebServerRequest *request) const {
	const vector<const char*> headers = { "Acceleration X(m/s^2)",
			"Acceleration Y(m/s^2)", "Acceleration Z(m/s^2)",
			"Linear Acceleration X(m/s^2)", "Linear Acceleration Y(m/s^2)",
			"Linear Acceleration Z(m/s^2)", "Rotation X(rad/s)",
			"Rotation Y(rad/s)", "Rotation Z(rad/s)", "Magnetic X(uT)",
			"Magnetic Y(uT)", "Magnetic Z(uT)" };

	const uint8_t channels = headers.size();
	const function<char* (byte, uint)> accel_gen = getDataContentGenerator(1);
	const function<char* (byte, uint)> linear_accel_gen =
			getLinearAccelerationGenerator();
	const function<char* (byte, uint)> gyromag_gen = getDataContentGenerator(4,
			6);
	sendMeasurementsCsv(request,
			[this, channels, accel_gen, linear_accel_gen, gyromag_gen](
					byte separator_char, uint position) {
				char *content = new char[channels * 13] { };
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
			},
			headers);
}

void LSM9DS1Handler::sendLinearAccelerometerCsv(
		AsyncWebServerRequest *request) const {
	const vector<const char*> headers = { "Linear Acceleration X(m/s^2)",
			"Linear Acceleration Y(m/s^2)", "Linear Acceleration Z(m/s^2)" };

	sendMeasurementsCsv(request, getLinearAccelerationGenerator(), headers);
}

void LSM9DS1Handler::sendMeasurementsCsv(AsyncWebServerRequest *request,
		const vector<const char*> headers, const uint8_t index) const {

	const uint8_t channels = headers.size();
	sendMeasurementsCsv(request, getDataContentGenerator(index, channels),
			headers);
}

void LSM9DS1Handler::sendMeasurementsCsv(AsyncWebServerRequest *request,
		const function<char* (byte, uint)> content_generator,
		const vector<const char*> headers) const {

	byte separator_char = ',';
	if (request->hasArg("separator")) {
		separator_char = request->arg("separator")[0];
	}

	uint position = 0;
	string buf = "";
	request->sendChunked("text/csv",
			[this, separator_char, position, buf, content_generator, headers](
					uint8_t *buffer, size_t maxlen, size_t idx) mutable {
				maxlen = min(int(maxlen), 1200);
				uint16_t length = 0;
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
							uint(data[position * values_per_measurement]),
							content, '\n');

					delete[] content;
					position++;
				}

				if (length > maxlen) {
					buf.append(string(response).substr(maxlen));
					length = maxlen;
				}

				for (uint i = 0; i < length && i < maxlen; i++) {
					buffer[i] = response[i];
				}
				delete[] response;

				return min(size_t(length), maxlen);
			});
}
