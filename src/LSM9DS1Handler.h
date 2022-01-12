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

const uint8_t LSM9DS1_XGCS = 27;
const uint8_t LSM9DS1_MCS = 26;

//#define LSM9DS1_I2C // uncomment to use I2C to connect to the lsm9ds1

#ifndef LSM9DS1_I2C
#define LSM9DS1_SPI
#else
#undef LSM9DS1_SPI
#endif

/**
 * The number of floats to be recorder per measurement.
 * Default is 1x timestamp + 3x accelerometer + 3x magnetometer + 3x gyroscope.
 */
const uint8_t VALUES_PER_MEASUREMENT = 10;

class LSM9DS1Handler {
public:
	LSM9DS1Handler(uint32_t max_measurements);
	virtual ~LSM9DS1Handler();

	void begin();
	void setupSensor();
	void loop();

	/**
	 * Starts a new recording session storing the given number of measurements.
	 *
	 * @param measurements	The number of measurements to take.
	 * @param freq			The target measurement frequency. In measurements per second.
	 */
	void measure(uint32_t measurements, uint16_t freq);

	void sendAllCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Acceleration X(m/s^2)",
				"Acceleration Y(m/s^2)", "Acceleration Z(m/s^2)",
				"Linear Acceleration X(m/s^2)", "Linear Acceleration Y(m/s^2)",
				"Linear Acceleration Z(m/s^2)", "Rotation X(rad/s)",
				"Rotation Y(rad/s)", "Rotation Z(rad/s)", "Magnetic X(uT)",
				"Magnetic Y(uT)", "Magnetic Z(uT)" };
		sendMeasurementsCsv(request, getAllGenerator(), headers);
	}

	void sendAccelerometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Acceleration X(m/s^2)",
				"Acceleration Y(m/s^2)", "Acceleration Z(m/s^2)" };
		sendMeasurementsCsv(request, headers, 1);
	}

	void sendLinearAccelerometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Linear Acceleration X(m/s^2)",
				"Linear Acceleration Y(m/s^2)", "Linear Acceleration Z(m/s^2)" };
		sendMeasurementsCsv(request, getLinearAccelerationGenerator(), headers);
	}

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

	void sendMeasurementsJson(AsyncWebServerRequest *request) const;

	const uint32_t getStoredMeasurements() const {
		return measurements_stored;
	}

	const uint32_t getMeasurements() const {
		return measurements;
	}

	const bool isMeasuring() const {
		return measuring;
	}

	/**
	 * Get the time for one measurement in microseconds.
	 *
	 * @return	The time for one measurement in microseconds.
	 */
	const uint16_t getMeasuringTime() const {
		return measuring_time;
	}

	/**
	 * Get the measurement start time in milliseconds.
	 *
	 * @return	The time when the current measurement series was started.
	 */
	const uint64_t getMeasurementStart() const {
		return measurement_start;
	}

	/**
	 * Get the time spent measuring in milliseconds.
	 *
	 * @return	The time spent measuring in milliseconds.
	 */
	const uint64_t getMeasurementDuration() const {
		return measurement_duration;
	}

	void resetMeasurements() {
		measurements_stored = 0;
		measuring = false;
	}

private:
#ifdef LSM9DS1_I2C
	Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(); // Use I2C to connect to the lsm9ds1
#endif
#ifdef LSM9DS1_SPI
	Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(22, 32, 21, LSM9DS1_XGCS, LSM9DS1_MCS); // Use Software SPI to connect to the lsm9ds1
#endif

	const uint32_t measurements_max;
	const uint32_t data_size;

	float *data = NULL;

	uint32_t measurements_stored = 0;
	uint32_t measurements = 0;

	uint16_t frequency = 50;
	uint32_t measuring_time = 0;
	uint32_t measuring_time_target = 20000;

	uint64_t measurement_start = 0;
	uint32_t measurement_duration = 0;

	bool measuring = false;

	std::function<char* (uint8_t, uint32_t)> getAllGenerator() const;
	std::function<char* (uint8_t, uint32_t)> getLinearAccelerationGenerator() const;
	std::function<char* (uint8_t, uint32_t)> getDataContentGenerator(const uint8_t index, uint8_t channels = 3) const;

	/**
	 * Generates a measurements csv and sends it to a client.
	 *
	 * @param request	The HTTP web request requesting the measurements csv.
	 * @param headers	A vector containing the headers for the generated measurements csv.
	 * @param index		The index of the first value to to write to the csv in a measurement.
	 */
	void sendMeasurementsCsv(AsyncWebServerRequest *request,
			const std::vector<const char*> headers, const uint8_t index) const {
		sendMeasurementsCsv(request,
				getDataContentGenerator(index, headers.size()), headers);
	}

	/**
	 * Sends a measurements csv to a client.
	 *
	 * @param request			The HTTP web request requesting the measurements csv.
	 * @param content_generator	The function generating a single line of the measurements csv.
	 * @param headers			A vector containing the headers for the generated measurements csv.
	 */
	void sendMeasurementsCsv(AsyncWebServerRequest *request,
			const std::function<char* (uint8_t, uint32_t)> content_generator,
			const std::vector<const char*> headers) const;

	/**
	 * Generates a part of the a measurements csv.
	 *
	 * @param separator_char	The character to be used to separate values in the csv.
	 * @param position			The index of the last value that was already written to the csv.
	 * 							Used for the next call to continue where the last one left off.
	 * @param buf				A buffer string to store a small amount of data between calls.
	 * @param content_generator	The function generating a single line of the csv.
	 * @param headers			A vector containing the headers for the generated measurements csv.
	 * @param buffer			The output buffer to write the content to.
	 * @param maxlen			The max length of the content to generate.
	 * @return	The number of bytes generated.
	 */
	size_t generateMeasurementCsv(const uint8_t separator_char, uint32_t &position,
			std::string &buf,
			const std::function<char* (uint8_t, uint32_t)> content_generator,
			const std::vector<const char*> headers, uint8_t *buffer,
			size_t maxlen) const;
};

#endif /* SRC_LSM9DS1HANDLER_H_ */
