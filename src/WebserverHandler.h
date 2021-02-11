/*
 * WebserverHandler.h
 *
 *  Created on: 23.01.2021
 *      Author: ToMe25
 */

#ifndef SRC_WEBSERVERHANDLER_H_
#define SRC_WEBSERVERHANDLER_H_

#include <ESPAsyncWebServer.h>
#include <LSM9DS1Handler.h>

extern const char settings_html[] asm("_binary_src_html_settings_html_start");
extern const char recording_html[] asm("_binary_src_html_recording_html_start");
extern const char results_html[] asm("_binary_src_html_results_html_start");
extern const char not_found_html[] asm("_binary_src_html_404_html_start");
extern const char index_css[] asm("_binary_src_html_index_css_start");

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

	void replace_all(std::string &str, const std::string &from, const std::string &to);
};

#endif
