#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <windows.h>

#define SCR_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define SCR_HIGHT GetSystemMetrics(SM_CYSCREEN)

std::string status = "", status_start = "0", status_end = "1";
std::vector<std::string>status_list = { "0", "1", "00", "01", "10", "11" };
bool run_status = true;
bool flag = false;
std::vector<time_t>vec_time_stamp;
std::vector<int>vec_data;

std::string dumpData()
{
	std::string s = "";
	for (int i = 0; i < vec_time_stamp.size(); i++)
	{
		s += std::to_string(vec_time_stamp[i]);
		s += ",";
		s += std::to_string(vec_data[2 * i]);
		s += ",";
		s += std::to_string(vec_data[2 * i + 1]);
		s += ",";
	}
	return s;
}

time_t getTimeStampTobii()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
		std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	time_t timestamp = tmp.count();

	return timestamp;
}

void gaze_point_callback(tobii_gaze_point_t const* gaze_point, void* user_data)
{
	vec_time_stamp.emplace_back(getTimeStampTobii());

	if (gaze_point->validity == TOBII_VALIDITY_VALID)
	{
		vec_data.emplace_back(static_cast<int>(gaze_point->position_xy[0] * SCR_WIDTH));
		vec_data.emplace_back(static_cast<int>(gaze_point->position_xy[1] * SCR_HIGHT));
	}
	else
		for (int i = 0; i < 2; i++) vec_data.emplace_back(0);

	if (!flag)
	{
		vec_time_stamp.clear();
		vec_data.clear();
	}
}

void url_receiver(char const* url, void* user_data)
{
	char* buffer = (char*)user_data;
	if (*buffer != '\0') return;

	if (strlen(url) < 256)
		strcpy(buffer, url);
}

void workTobii()
{
	tobii_api_t* api = NULL;
	tobii_error_t result = tobii_api_create(&api, NULL, NULL);

	char url[256] = { 0 };
	result = tobii_enumerate_local_device_urls(api, url_receiver, url);
	if (*url == '\0')
	{
		std::cout << "Error: No device found" << std::endl;
		exit(-1);
	}

	tobii_device_t* device = NULL;
	result = tobii_device_create(api, url, TOBII_FIELD_OF_USE_INTERACTIVE, &device);

	result = tobii_gaze_point_subscribe(device, gaze_point_callback, NULL);

	while (true)
	{
		if (!run_status) break;

		result = tobii_wait_for_callbacks(1, &device);
		result = tobii_device_process_callbacks(device);
	}

	result = tobii_gaze_point_unsubscribe(device);
	result = tobii_device_destroy(device);
	result = tobii_api_destroy(api);
}