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

#include <ESPAsyncWebServer.h>
#include <LSM9DS1Handler.h>

extern const char settings_html[] asm("_binary_src_html_settings_html_start");
extern const char recording_html[] asm("_binary_src_html_recording_html_start");
extern const char calculating_html[] asm("_binary_src_html_calculating_html_start");
extern const char results_html[] asm("_binary_src_html_results_html_start");
extern const char unavailable_html[] asm("_binary_src_html_unavailable_html_start");
extern const char not_found_html[] asm("_binary_src_html_not_found_html_start");
extern const char index_css[] asm("_binary_src_html_index_css_start");
extern const char recording_js[] asm("_binary_src_html_recording_js_start");
extern const char calculating_js[] asm("_binary_src_html_calculating_js_start");

class WebserverHandler {
public:
	WebserverHandler(int port, LSM9DS1Handler *handler);
	virtual ~WebserverHandler();

	void begin() {
		server.begin();
	}

	void on_not_found(AsyncWebServerRequest *request);
	void on_get_index(AsyncWebServerRequest *request);
	void on_post_index(AsyncWebServerRequest *request);

	const char* format_time(uint64_t time_ms);

private:
	AsyncWebServer server;

	LSM9DS1Handler *lsm9ds1;

	void register_url(const uint8_t http_code, const char* url, std::function<void(AsyncWebServerRequest*)> callback);
	void replace_all(std::string &str, const std::string &from, const std::string &to);
};

#endif
