/*
 * LSM9DS1Handler.h
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

#ifndef SRC_LSM9DS1HANDLER_H_
#define SRC_LSM9DS1HANDLER_H_

#include <Adafruit_LSM9DS1.h>
#include <ESPAsyncWebServer.h>

#define LSM9DS1_XGCS 27
#define LSM9DS1_MCS 26

class LSM9DS1Handler {
public:
	static const uint8_t values_per_measurement = 10;

	LSM9DS1Handler(uint max_measurements);
	virtual ~LSM9DS1Handler();

	void begin();
	void setupSensor();
	void loop();

	void measure(uint measurements, uint16_t freq);

	void sendAllCsv(AsyncWebServerRequest *request) const;

	void sendAccelerometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Acceleration X(m/s^2)",
				"Acceleration Y(m/s^2)", "Acceleration Z(m/s^2)" };
		sendMeasurementsCsv(request, headers, 1);
	}

	void sendLinearAccelerometerCsv(AsyncWebServerRequest *request) const;

	void sendGyroscopeCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Rotation X(rad/s)",
				"Rotation Y(rad/s)", "Rotation Z(rad/s)" };
		sendMeasurementsCsv(request, headers, 4);
	}

	void sendMagnetometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Magnetic X(uT)",
				"Magnetic Y(uT)", "Magnetic Z(uT)" };
		sendMeasurementsCsv(request, headers, 7);
	}

	const uint getStoredMeasurements() const {
		return measurements_stored;
	}

	const uint getMeasurements() const {
		return measurements;
	}

	const bool isMeasuring() const {
		return measuring;
	}

	const uint16_t getMeasuringTime() const {
		return measuring_time;
	}

	const uint64_t getMeasurementStart() const {
		return measurement_start;
	}

	const uint64_t getMeasurementDuration() const {
		return measurement_duration;
	}

	void resetMeasurements() {
		measurements_stored = 0;
		measuring = false;
	}

private:
	Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

	const uint measurements_max;
	const uint data_size;

	float *data = NULL;

	uint measurements_stored = 0;
	uint measurements = 0;

	uint16_t frequency = 50;
	uint16_t measuring_time = 0;
	uint16_t measuring_time_target = 20;

	uint64_t measurement_start = 0;
	uint measurement_duration = 0;

	bool measuring = false;

	std::function<char* (byte, uint)> getLinearAccelerationGenerator() const;
	std::function<char* (byte, uint)> getDataContentGenerator(const uint8_t index, uint8_t channels = 3) const;

	void sendMeasurementsCsv(AsyncWebServerRequest *request,
			const std::vector<const char*> headers, const uint8_t index) const;

	void sendMeasurementsCsv(AsyncWebServerRequest *request,
			const std::function<char* (byte, uint)> content_generator,
			const std::vector<const char*> headers) const;
};

#endif
