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

class LSM9DS1Handler {
public:
	/**
	 * The number of floats to be recorder per measurement.
	 * Default is 1x timestamp + 3x accelerometer + 3x magnetometer + 3x gyroscope.
	 */
	const uint8_t VALUES_PER_MEASUREMENT = 10;

	/**
	 * The number of different measurement csvs that can be downloaded after recording measurements.
	 */
	const uint8_t MEASUREMENT_CSVS = 5;

	/**
	 * The bit to be set in the EventGroup when starting a measurement.
	 */
	const EventBits_t MEASURE_START_BIT = 1;

	/**
	 * Creates a new LSM9DS1Handler for the given maximum amount of measurements.
	 *
	 * @param max_measurements	The max amount of measurements to be stored in this LSM9DS1Handler.
	 */
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
		sendMeasurementsCsv(request, getAllGenerator(), headers, all_csv_size);
	}

	void sendAccelerometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Acceleration X(m/s^2)",
				"Acceleration Y(m/s^2)", "Acceleration Z(m/s^2)" };
		sendMeasurementsCsv(request, headers, 1, acc_csv_size);
	}

	void sendLinearAccelerometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = {
				"Linear Acceleration X(m/s^2)", "Linear Acceleration Y(m/s^2)",
				"Linear Acceleration Z(m/s^2)" };
		sendMeasurementsCsv(request, getLinearAccelerationGenerator(), headers,
				lin_acc_csv_size);
	}

	void sendGyroscopeCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Rotation X(rad/s)",
				"Rotation Y(rad/s)", "Rotation Z(rad/s)" };
		sendMeasurementsCsv(request, headers, 4, gyro_csv_size);
	}

	void sendMagnetometerCsv(AsyncWebServerRequest *request) const {
		const std::vector<const char*> headers = { "Magnetic X(uT)",
				"Magnetic Y(uT)", "Magnetic Z(uT)" };
		sendMeasurementsCsv(request, headers, 7, mag_csv_size);
	}

	void sendMeasurementsJson(AsyncWebServerRequest *request) const;

	void sendCalculationsJson(AsyncWebServerRequest *request) const;

	const uint32_t getStoredMeasurements() const {
		return measurements_stored;
	}

	const uint32_t getMeasurements() const {
		return measurements;
	}

	/**
	 * Checks whether the esp is currently recording measurements.
	 *
	 * @return	True if a measurement is in progress.
	 */
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

	/**
	 * Checks whether the esp is currently generating the output files to determine their size.
	 *
	 * @return	True if the esp is generating the output files for this purpose.
	 */
	const bool isCalculating() const {
		return calculating;
	}

	/**
	 * Gets the name of the measurements csv for which the size is currently being calculated.
	 *
	 * @return	The name of the csv being generated.
	 */
	const std::string getFileCalculating() const {
		return file_calculating;
	}

	/**
	 * Gets the number of measurement csvs for which the file size has already be determined.
	 *
	 * @return	The number of measurement csvs for which the file size has already be determined.
	 */
	const uint8_t getFilesCalculated() const {
		return calculated;
	}

	/**
	 * Get the calculation start time in milliseconds.
	 *
	 * @return	The calculation start time in milliseconds.
	 */
	const uint64_t getCalculationStart() const {
		return calculation_start;
	}

	/**
	 * Resets the LSM0DS1Handler to a state where it is neither recording measurements nor calculating csv sizes.
	 */
	void resetMeasurements() {
		measurements_stored = 0;
		measuring = false;
		file_calculating = "";
		calculated = 0;
		calculating = false;
	}

private:
#ifdef LSM9DS1_I2C
	Adafruit_LSM9DS1 lsm; // Use I2C to connect to the lsm9ds1
#elif defined(LSM9DS1_SPI)
	Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(22, 32, 21, LSM9DS1_XGCS,
			LSM9DS1_MCS); // Use Software SPI to connect to the lsm9ds1
#endif

	const uint32_t measurements_max;
	const uint32_t data_size;

	float *data = NULL;
	uint32_t last_timestamp = 0;

	uint32_t measurements_stored = 0;
	uint32_t measurements = 0;

	uint16_t frequency = 50;
	uint32_t measuring_time = 0;
	uint32_t measuring_time_target = 20000;

	uint64_t measurement_start = 0;
	uint32_t measurement_duration = 0;

	bool measuring = false;
	EventGroupHandle_t eventGroup;

	std::string file_calculating;
	uint8_t calculated = 0;
	uint64_t calculation_start = 0;
	bool calculating = false;

	size_t all_csv_size = 0;
	size_t lin_acc_csv_size = 0;
	size_t acc_csv_size = 0;
	size_t gyro_csv_size = 0;
	size_t mag_csv_size = 0;

	std::function<char* (uint8_t, uint32_t)> getAllGenerator() const;
	std::function<char* (uint8_t, uint32_t)> getLinearAccelerationGenerator() const;
	std::function<char* (uint8_t, uint32_t)> getDataContentGenerator(
			const uint8_t index, uint8_t channels = 3) const;

	/**
	 * Generates a measurements csv and sends it to a client.
	 *
	 * @param request		The HTTP web request requesting the measurements csv.
	 * @param headers		A vector containing the headers for the generated measurements csv.
	 * @param index			The index of the first value to to write to the csv in a measurement.
	 * @param content_len	The total size of the measurements csv in bytes.
	 */
	void sendMeasurementsCsv(AsyncWebServerRequest *request,
			const std::vector<const char*> headers, const uint8_t index,
			const size_t content_len) const {
		sendMeasurementsCsv(request,
				getDataContentGenerator(index, headers.size()), headers,
				content_len);
	}

	/**
	 * Sends a measurements csv to a client.
	 *
	 * @param request			The HTTP web request requesting the measurements csv.
	 * @param content_generator	The function generating a single line of the measurements csv.
	 * @param headers			A vector containing the headers for the generated measurements csv.
	 * @param content_len		The total size of the measurements csv in bytes.
	 */
	void sendMeasurementsCsv(AsyncWebServerRequest *request,
			const std::function<char* (uint8_t, uint32_t)> content_generator,
			const std::vector<const char*> headers,
			const size_t content_len) const;

	/**
	 * Generates a part of the a measurements csv.
	 *
	 * @param separator_char	The character to be used to separate values in the csv.
	 * @param position			The index of the last value that was already written to the csv.
	 * 							Used for the next call to continue where the last one left off.
	 * @param content_generator	The function generating a single line of the csv.
	 * @param headers			A vector containing the headers for the generated measurements csv.
	 * @param buffer			The output buffer to write the content to.
	 * @param maxlen			The max length of the content to generate.
	 * @return	The number of bytes generated.
	 */
	size_t generateMeasurementCsv(const uint8_t separator_char,
			uint32_t &position,
			const std::function<char* (uint8_t, uint32_t)> content_generator,
			const std::vector<const char*> headers, char *buffer,
			size_t maxlen) const;

	/**
	 * Generates a part of the a measurements csv.
	 *
	 * @param separator_char	The character to be used to separate values in the csv.
	 * @param position			The index of the last value that was already written to the csv.
	 * 							Used for the next call to continue where the last one left off.
	 * @param content_generator	The function generating a single line of the csv.
	 * @param headers			A vector containing the headers for the generated measurements csv.
	 * @param buffer			The output buffer to write the content to.
	 * @param maxlen			The max length of the content to generate.
	 * @return	The number of bytes generated.
	 */
	size_t generateMeasurementCsv(const uint8_t separator_char,
			uint32_t &position,
			const std::function<char* (uint8_t, uint32_t)> content_generator,
			const std::vector<const char*> headers, uint8_t *buffer,
			size_t maxlen) const;

	/**
	 * The function running in a new task generating the measurement csv to send to a client.
	 *
	 * @param parameter	A pointer to a CsvGeneratorParameter containing all the required variables.
	 */
	static void csvGenerator(void *parameter);
};

/**
 * A Stream implementation that writes to and reads from a cbuf.
 * Can also optionally notify an EventGroup when more then 1000 bytes are free.
 */
class BufferStream : public Stream {
public:
	/**
	 * Creates a new BufferStream with the given initial buffer size.
	 * The buffer size can be increased by writing more bytes to it.
	 *
	 * @param buffer_size	The initial buffer size.
	 */
	BufferStream(size_t buffer_size = 10000);
	virtual ~BufferStream();

	/**
	 * Gets the size of the internal buffer.
	 *
	 * @return	The size of the internal buffer.
	 */
	size_t size() {
		return content->size();
	}

	int available();
	int peek();
	int read();
	size_t readBytes(char *buffer, size_t length);
	size_t readBytes(uint8_t *buffer, size_t length);
	String readString();

	int availableForWrite();
	size_t write(uint8_t b);
	size_t write(const uint8_t *buffer, size_t size);
	using Print::write;

	void flush() {
	}

	/**
	 * Sets the EventGroup to notify when something happens.
	 * There is one notification for when 1000 or more bytes are free in the internal buffer.
	 * And another one for when the stream is empty.
	 *
	 * @param group	The EventGroup to notify.
	 */
	void setEventGroup(EventGroupHandle_t group) {
		eventGroup = group;
	}

	/**
	 * Gets the EventGroup to notify when something happens.
	 * There is one notification for when a tenth or more of internal buffer is free.
	 * And another one for when the stream is empty.
	 *
	 * @return	The EventGroup that gets notified.
	 */
	EventGroupHandle_t getEventGroup() const {
		return eventGroup;
	}

	/**
	 * The bit that is set in the EventGroup when a tenth of the internal buffer or more is free.
	 */
	const EventBits_t FREE_SPACE_BIT = 1;

	/**
	 * The bit that is set in the EventGroup when the buffer is completely empty.
	 */
	const EventBits_t EMPTY_BIT = 1 << 1;
private:
	cbuf *content;
	EventGroupHandle_t eventGroup;
	bool empty = false;
};

struct CsvGeneratorParameter {
	CsvGeneratorParameter(const LSM9DS1Handler *handler, BufferStream *stream,
			size_t buffer_len, char *buffer,
			const std::function<char* (uint8_t, uint32_t)> content_gen,
			const std::vector<const char*> headers, const size_t content_len,
			const uint8_t separator_char) :
			handler(handler), stream(stream), buffer_len(buffer_len), buffer(
					buffer), content_gen(content_gen), headers(headers), content_len(
					content_len), separator_char(separator_char) {
	}

	~CsvGeneratorParameter() {
		delete[] buffer;
		delete stream;
	}

	const LSM9DS1Handler *handler;
	BufferStream *stream;
	size_t buffer_len;
	char *buffer;
	const std::function<char* (uint8_t, uint32_t)> content_gen;
	const std::vector<const char*> headers;
	const size_t content_len;
	const uint8_t separator_char;
};

#endif /* SRC_LSM9DS1HANDLER_H_ */
