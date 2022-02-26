/*
 * WebserverHandler.h
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

#ifndef SRC_WEBSERVERHANDLER_H_
#define SRC_WEBSERVERHANDLER_H_

#include "LSM9DS1Handler.h"
#include <ESPAsyncWebServer.h>

extern const char calculating_html[] asm("_binary_src_html_calculating_html_start");
extern const char not_found_html[] asm("_binary_src_html_not_found_html_start");
extern const char recording_html[] asm("_binary_src_html_recording_html_start");
extern const char results_html[] asm("_binary_src_html_results_html_start");
extern const char settings_html[] asm("_binary_src_html_settings_html_start");
extern const char unavailable_html[] asm("_binary_src_html_unavailable_html_start");
extern const char main_css[] asm("_binary_src_html_main_css_start");
extern const char calculating_js[] asm("_binary_src_html_calculating_js_start");
extern const char recording_js[] asm("_binary_src_html_recording_js_start");

class WebserverHandler {
public:
	WebserverHandler(LSM9DS1Handler *handler, uint16_t port = 80);
	virtual ~WebserverHandler() {
	}

	void begin() {
		server.begin();
	}

	static void on_not_found(AsyncWebServerRequest *request);
	void on_get_index(AsyncWebServerRequest *request);
	void on_post_index(AsyncWebServerRequest *request);

private:
	LSM9DS1Handler *lsm9ds1;
	AsyncWebServer server;

	void register_url(const uint8_t http_code, const char *uri,
			std::function<void(AsyncWebServerRequest*)> callback);

	/**
	 * Registers a request handler responding to requests on the given uri.
	 * Responds with the given c string.
	 *
	 * @param http_code		The HTTP request type to which to respond.
	 * @param uri			The uri for which to register a request handler.
	 * @param content_type	The content type for the response.
	 * @param content		The content to respond with when the given uri is requested.
	 */
	void register_static_handler(const uint8_t http_code, const char *uri,
			const char *content_type, const char *content);

	/**
	 * Formats the given time in milliseconds as a human readable string.
	 * The resulting string only contains the two biggest time units which have a non zero value.
	 *
	 * @param time_ms	The time to print as a string in ms.
	 * @return	The formatted string.
	 */
	const std::string format_time(uint64_t time_ms);
};

#endif
