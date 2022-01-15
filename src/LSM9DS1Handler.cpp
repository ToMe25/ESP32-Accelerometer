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
#include "WebserverHandler.h"
#include <sstream>
#include <Adafruit_AHRS_Madgwick.h>

LSM9DS1Handler::LSM9DS1Handler(uint32_t max_measurements) :
		measurements_max(max_measurements), data_size(
				max_measurements * VALUES_PER_MEASUREMENT * sizeof(float)),
				eventGroup(xEventGroupCreate()) {
}

LSM9DS1Handler::~LSM9DS1Handler() {
	free(data);
	vTaskDelete(eventGroup);
}

void LSM9DS1Handler::begin() {
	data = (float*) ps_malloc(data_size);

	while (!lsm.begin()) {
		Serial.println("Failed to initialize the LSM9DS1. Check your wiring!");
		delay(1000);
	}

#ifdef LSM9DS1_I2C
	Serial.println("Connected to the lsm9ds1 using I2C.");
#elif defined(LSM9DS1_SPI)
	Serial.println("Connected to the lsm9ds1 using SPI.");
#endif

	setupSensor();

#ifdef LSM9DS1_I2C
	Wire.setClock(400000);
#endif
}

void LSM9DS1Handler::setupSensor() {
	lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);

	lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

	lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

void LSM9DS1Handler::loop() {
	if (measuring) {
		if (measurements_stored >= measurements) {
			measuring = false;
			measurement_duration = millis() - measurement_start;
			measuring_time = measurement_duration * 1000 / measurements;
			calculating = true;
			return;
		}

		uint64_t start_us = micros();

		sensors_event_t a, m, g, t;
		lsm.getEvent(&a, &m, &g, &t);

		if (measurements_stored > 0 && a.timestamp == last_timestamp) {
			return;
		}

		last_timestamp = a.timestamp;

		data[measurements_stored * VALUES_PER_MEASUREMENT] = a.timestamp
				- measurement_start;

		data[measurements_stored * VALUES_PER_MEASUREMENT + 1] =
				a.acceleration.x;
		data[measurements_stored * VALUES_PER_MEASUREMENT + 2] =
				a.acceleration.y;
		data[measurements_stored * VALUES_PER_MEASUREMENT + 3] =
				a.acceleration.z;

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
									- min(measurements_stored, (uint32_t) 10))
									* VALUES_PER_MEASUREMENT]) * 1000
							/ (min(measurements_stored, (uint32_t) 10) - 1));
		}

		delayMicroseconds(
				measuring_time_target
						- min(measuring_time_target,
								(uint32_t) (micros() - start_us)));
	} else if (calculating) {
		const uint64_t start = millis();
		const uint8_t separator = ',';
		uint32_t pos = 0;
		size_t size = 0;
		std::function<char* (uint8_t, uint32_t)> content_generator;
		std::vector<const char*> headers;

		switch(calculated) {
		case 0:
			calculation_start = start;
			file_calculating = "all.csv";
			content_generator = getAllGenerator();
			headers = { "Acceleration X(m/s^2)", "Acceleration Y(m/s^2)",
					"Acceleration Z(m/s^2)", "Linear Acceleration X(m/s^2)",
					"Linear Acceleration Y(m/s^2)",
					"Linear Acceleration Z(m/s^2)", "Rotation X(rad/s)",
					"Rotation Y(rad/s)", "Rotation Z(rad/s)", "Magnetic X(uT)",
					"Magnetic Y(uT)", "Magnetic Z(uT)" };
			break;
		case 1:
			file_calculating = "accelerometer.csv";
			content_generator = getDataContentGenerator(1, 3);
			headers = { "Acceleration X(m/s^2)", "Acceleration Y(m/s^2)",
					"Acceleration Z(m/s^2)" };
			break;
		case 2:
			file_calculating = "linear_accelerometer.csv";
			content_generator = getLinearAccelerationGenerator();
			headers = { "Linear Acceleration X(m/s^2)",
					"Linear Acceleration Y(m/s^2)",
					"Linear Acceleration Z(m/s^2)" };
			break;
		case 3:
			file_calculating = "gyroscope.csv";
			content_generator = getDataContentGenerator(4, 3);
			headers = { "Rotation X(rad/s)", "Rotation Y(rad/s)",
					"Rotation Z(rad/s)" };
			break;
		case 4:
			file_calculating = "magnetometer.csv";
			content_generator = getDataContentGenerator(7, 3);
			headers = { "Magnetic X(uT)", "Magnetic Y(uT)", "Magnetic Z(uT)" };
			break;
		default:
			Serial.print("Trying to calculate size for unknown measurements csv with id ");
			Serial.print(calculated);
			Serial.println('.');
			calculating = false;
			return;
		}

		const size_t buffer_size = 10000;
		char *buffer = new char[buffer_size];
		while (pos < measurements) {
			size += generateMeasurementCsv(separator, pos,
					content_generator, headers, buffer, buffer_size);
		}

		delete[] buffer;

		switch(calculated) {
		case 0:
			all_csv_size = size;
			break;
		case 1:
			acc_csv_size = size;
			break;
		case 2:
			lin_acc_csv_size = size;
			break;
		case 3:
			gyro_csv_size = size;
			break;
		case 4:
			mag_csv_size = size;
			break;
		default:
			Serial.print("Trying to calculate size for unknown measurements csv with id ");
			Serial.print(calculated);
			Serial.println('.');
			calculating = false;
			file_calculating = "";
			calculated = 0;
		}

		calculated++;
		if (calculated >= MEASUREMENT_CSVS) {
			calculating = false;
		}
	} else {
		xEventGroupWaitBits(eventGroup, MEASURE_START_BIT, pdTRUE, pdTRUE, 500);
	}
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
	xEventGroupSetBits(eventGroup, MEASURE_START_BIT);
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
		strcpy(content, buffer);
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
	std::shared_ptr<Adafruit_Madgwick> filter = std::make_shared<
			Adafruit_Madgwick>();
	// Tell the filter a lower sample rate to reduce smoothing.
	filter->begin(round(1000000.0 / measuring_time) / 20);

	return [this, filter](uint8_t separator_char, uint32_t position) mutable {
		char *line = new char[39] { };

		const float ax = data[position * VALUES_PER_MEASUREMENT + 1]
				/ SENSORS_GRAVITY_EARTH;
		const float ay = data[position * VALUES_PER_MEASUREMENT + 2]
				/ SENSORS_GRAVITY_EARTH;
		const float az = data[position * VALUES_PER_MEASUREMENT + 3]
				/ SENSORS_GRAVITY_EARTH;

		const float gx = data[position * VALUES_PER_MEASUREMENT + 4]
				* SENSORS_RADS_TO_DPS;
		const float gy = data[position * VALUES_PER_MEASUREMENT + 5]
				* SENSORS_RADS_TO_DPS;
		const float gz = data[position * VALUES_PER_MEASUREMENT + 6]
				* SENSORS_RADS_TO_DPS;

		filter->update(gx, gy, gz, ax, ay, az,
				data[position * VALUES_PER_MEASUREMENT + 7],
				data[position * VALUES_PER_MEASUREMENT + 8],
				data[position * VALUES_PER_MEASUREMENT + 9]);

		float qW, qX, qY, qZ;
		filter->getQuaternion(&qW, &qX, &qY, &qZ);

		// Calculate gravity
		const float gravX = 2.0f * (qX * qZ - qW * qY);
		const float gravY = 2.0f * (qW * qX + qY * qZ);
		const float gravZ = 2.0f * (qW * qW - 0.5f + qZ * qZ);

		// Calculate linear acceleration
		const float linX = ax - gravX;
		const float linY = ay - gravY;
		const float linZ = az - gravZ;

		sprintf(line, "%c%f%c%f%c%f", separator_char,
				linX * SENSORS_GRAVITY_EARTH, separator_char,
				linY * SENSORS_GRAVITY_EARTH, separator_char,
				linZ * SENSORS_GRAVITY_EARTH);

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
		const std::vector<const char*> headers, const size_t content_len) const {
	if (measurements_stored == 0 || measuring || calculating) {
		AsyncWebServerResponse *response = request->beginResponse(503,
				"text/html", unavailable_html);
		if (measuring) {
			std::ostringstream converter;
			converter
					<< ((measurements - measurements_stored) * measuring_time
							/ 1000000);
			response->addHeader("Retry-After", converter.str().c_str());
		} else {
			response->addHeader("Retry-After", "5");
		}
		request->send(response);
		return;
	}

	uint8_t separator_char = ',';
	if (request->hasArg("separator")) {
		separator_char = request->arg("separator")[0];
	}

	BufferStream *stream = new BufferStream(25000);
	EventGroupHandle_t eventGroup = xEventGroupCreate();
	stream->setEventGroup(eventGroup);
	const size_t maxlen = 5000;
	char *buffer = new char[maxlen];
	std::shared_ptr<CsvGeneratorParameter> parameter = std::make_shared<
			CsvGeneratorParameter>(this, stream, maxlen, buffer,
			content_generator, headers, content_len, separator_char);
	TaskHandle_t handle;
	xTaskCreate(csvGenerator, "csv generator", 3000, parameter.get(), 1, &handle);

	request->onDisconnect([handle, parameter]() {
		if (parameter->buffer_len > 0) {
			parameter->buffer_len = 0;
			vTaskDelete(handle);
			vEventGroupDelete(parameter->stream->getEventGroup());
		}
	});
	request->send(*stream, "text/csv", content_len);
}

void LSM9DS1Handler::sendMeasurementsJson(
		AsyncWebServerRequest *request) const {
	char *measurements = new char[50];
	uint32_t measuring_time = measurement_duration;
	if (measuring) {
		measuring_time = uint32_t(millis() - measurement_start);
	}
	sprintf(measurements, "{\"measurements\": %u, \"time\": %u}",
			measurements_stored, measuring_time);
	request->send(200, "application/json", measurements);
	delete[] measurements;
}

void LSM9DS1Handler::sendCalculationsJson(
		AsyncWebServerRequest *request) const {
	char *calculations = new char[70];
	uint32_t calculating_time = millis() - calculation_start;
	sprintf(calculations, "{\"calculated\": %u, \"file\": \"%s\", \"time\": %u}",
			calculated, file_calculating.c_str(), calculating_time);
	request->send(200, "application/json", calculations);
	delete[] calculations;
}

size_t LSM9DS1Handler::generateMeasurementCsv(const uint8_t separator_char,
		uint32_t &position,
		const std::function<char* (uint8_t, uint32_t)> content_generator,
		const std::vector<const char*> headers, char *buffer,
		size_t maxlen) const {
	size_t length = 0;

	if (position == 0) {
		strcpy(buffer, "Time(ms)");
		length = 8;

		for (uint8_t i = 0; i < headers.size(); i++) {
			buffer[length++] = separator_char;
			strcpy(buffer + length, headers[i]);
			length += strlen(headers[i]);
		}

		buffer[length++] = '\n';
	}

	if (measurements_stored == 0) {
		position++;
	}

	while (length < maxlen - 13 * (headers.size() + 1) && position < measurements_stored) {
		char *content = content_generator(separator_char, position);

		length += sprintf(buffer + length, "%d",
				(uint32_t) data[position * VALUES_PER_MEASUREMENT]);
		strcpy(buffer + length, content);
		length += strlen(content);
		buffer[length++] = '\n';

		delete[] content;
		position++;
	}

	return min(length, maxlen);
}

size_t LSM9DS1Handler::generateMeasurementCsv(const uint8_t separator_char,
		uint32_t &position,
		const std::function<char* (uint8_t, uint32_t)> content_generator,
		const std::vector<const char*> headers, uint8_t *buffer,
		size_t maxlen) const {
	return generateMeasurementCsv(separator_char, position,
			content_generator, headers, (char*) buffer, maxlen);
}

void LSM9DS1Handler::csvGenerator(void *parameter) {
	CsvGeneratorParameter *param = (CsvGeneratorParameter*) parameter;
	const LSM9DS1Handler *handler = param->handler;
	BufferStream *stream = param->stream;
	size_t *maxlen = &param->buffer_len;
	char *buffer = param->buffer;
	const std::function<char* (uint8_t, uint32_t)> content_gen =
			param->content_gen;
	const EventGroupHandle_t eventGroup = stream->getEventGroup();
	const std::vector<const char*> headers = param->headers;
	const size_t content_len = param->content_len;
	const uint8_t separator_char = param->separator_char;

	size_t generated = 0;
	uint32_t position = 0;

	while (generated < content_len) {
		int free = min((int) *maxlen, stream->availableForWrite());
		if (free >= 1000) {
			free = handler->generateMeasurementCsv(separator_char, position,
					content_gen, headers, buffer, free);
			generated += stream->write(buffer, free);
		} else {
			xEventGroupWaitBits(eventGroup, stream->FREE_SPACE_BIT, pdTRUE, pdFALSE,
					500 / portTICK_PERIOD_MS);
		}

		if (*maxlen == 0) {
			return;
		}
	}

	*maxlen = 0;
	xEventGroupWaitBits(eventGroup, stream->EMPTY_BIT, pdTRUE, pdFALSE,
			750 / portTICK_PERIOD_MS);
	vEventGroupDelete(eventGroup);
	vTaskDelete(NULL);
}

BufferStream::BufferStream(size_t buffer_size) {
	content = new cbuf(buffer_size);
	free = content->room();
	eventGroup = NULL;
}

BufferStream::~BufferStream() {
	delete content;
}

int BufferStream::available() {
	return content->available();
}

int BufferStream::peek() {
	return content->peek();
}

int BufferStream::read() {
	const bool flag = eventGroup != NULL && free < 1000;
	const int read = content->read();
	free++;
	if (flag && free >= 1000) {
		xEventGroupSetBits(eventGroup, FREE_SPACE_BIT);
		if (content->available() == 0) {
			xEventGroupSetBits(eventGroup, EMPTY_BIT);
			empty = true;
		}
	}
	return read;
}

size_t BufferStream::readBytes(char *buffer, size_t length) {
	const bool flag = eventGroup != NULL && free < 1000;
	const size_t read = content->read(buffer, length);
	free += read;
	if (flag && free >= 1000) {
		xEventGroupSetBits(eventGroup, FREE_SPACE_BIT);
		if (content->available() == 0) {
			xEventGroupSetBits(eventGroup, EMPTY_BIT);
			empty = true;
		}
	}
	return read;
}

size_t BufferStream::readBytes(uint8_t *buffer, size_t length) {
	return readBytes((char*) buffer, length);
}

String BufferStream::readString() {
	const size_t available = content->available();
	char *buf = new char[available];
	readBytes(buf, available);
	return buf;
}

size_t BufferStream::write(uint8_t b) {
	return write(&b, 1);
}

size_t BufferStream::write(const uint8_t *buffer, size_t size) {
	if (size > free) {
		content->resizeAdd(size - free);
		free = content->room();
	}

	const bool flag = eventGroup != NULL && free >= 1000;
	const size_t written = content->write((const char*) buffer, size);
	free -= written;
	if (flag && free < 1000) {
		xEventGroupClearBits(eventGroup, FREE_SPACE_BIT);
		if (empty) {
			xEventGroupClearBits(eventGroup, EMPTY_BIT);
			empty = false;
		}
	}
	return written;
}
