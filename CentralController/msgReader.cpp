#include "msgReader.h"


//function used to deseriallize
int readMsg(char* msgBuffer, msgstruct& msg) {
	Encoder dataenc;

	//set buffer pointer
	dataenc.Reset(msgBuffer);
	dataenc.IndexUpdate(0);

	//declare variables to read into
	uint8_t msgType = 4;//by default msg is invalid 
	uint32_t timestamp = 0;
	uint8_t token = 0;
	uint32_t dataPairId = 0;
	uint32_t payload = 0;
	std::string newDeviceId = "";
	char cstr[50];
	uint8_t size = 0;

	//initializing msg's fields
	msg.msgType = msgType;
	msg.timestamp = timestamp;
	msg.token = token;
	msg.dataPairId = dataPairId;
	msg.payload = payload;
	
	//decode msgType
	if (dataenc.decode_uint8(msgType) != dataenc.OK) {
		return 1;
	}

	//decode timestamp
	if (dataenc.decode_uint32(timestamp) != dataenc.OK) {
		return 2;
	}

	//decode token
	if (dataenc.decode_uint8(token) != dataenc.OK) {
		return 3;
	}

	//if message is of type "new DC connected", save the Device Id to newDeviceId"
	if (msgType == 3) {
		if (dataenc.decode_byte_string_n255(cstr, size) != dataenc.OK) {
			return 4;
		}
		else {
			newDeviceId.assign(cstr, size);
		}
	}
	//if its of another type read only the dataPairId and payload
	else {

		if (dataenc.decode_uint32(dataPairId) != dataenc.OK) {
			return 5;
		}

		if (dataenc.decode_uint32(payload) != dataenc.OK) {
			return 6;
		}
	}
	//Assign the deseriallized values to the fields of the struct
	msg.msgType = msgType;
	msg.timestamp = timestamp;
	msg.token = token;
	msg.dataPairId = dataPairId;
	msg.payload = payload;
	msg.newDeviceId = newDeviceId;

	return 0;



}