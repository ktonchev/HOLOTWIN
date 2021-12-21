#include "msgReader.h"


//function used to deseriallize
int readMsg(char* msgBuffer, msgstruct& msg) {
	Decoder datadec;

	//set buffer pointer
	datadec.Reset(msgBuffer);
	datadec.IndexUpdate(0);

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
	if (datadec.decode_uint8(msgType) != datadec.OK) {
		return 1;
	}

	//decode timestamp
	if (datadec.decode_uint32(timestamp) != datadec.OK) {
		return 2;
	}

	//decode token
	if (datadec.decode_uint8(token) != datadec.OK) {
		return 3;
	}

	//if message is of type "new DC connected", save the Device Id to newDeviceId"
	if (msgType == 3) {
		if (datadec.decode_byte_string_n255(cstr, size) != datadec.OK) {
			return 4;
		}
		else {
			newDeviceId.assign(cstr, size);
		}
	}
	//if its of another type read only the dataPairId and payload
	else {

		if (datadec.decode_uint32(dataPairId) != datadec.OK) {
			return 5;
		}

		if (datadec.decode_uint32(payload) != datadec.OK) {
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