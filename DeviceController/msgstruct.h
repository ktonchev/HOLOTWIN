#pragma once
#include <stdint.h>
#include <cstring>

//A struct that contains the fields of a message
typedef struct {

	uint8_t msgType = 4;// 0 = response, 1 = get, 2 = set, 3 = new DC connected
	uint32_t timestamp = 0;
	uint8_t token = 0;
	uint32_t dataPairId = 0;
	uint32_t payload = 0;
	std::string newDeviceId = "";

} msgstruct;