#include "msgWriter.h"

//Function used to seriallize
int writeMsg(char* msgBuffer, msgstruct msg) {
	Encoder dataenc;

	//set buffer pointer
	dataenc.Reset(msgBuffer);
	dataenc.IndexUpdate(0);

	//encode msgType
	if (dataenc.encode_uint8(msg.msgType) != dataenc.OK) {
		return 1;
	}
	//encode timestamp
	if (dataenc.encode_uint32(msg.timestamp) != dataenc.OK) {
		return 1;
	}
	//encode token
	if (dataenc.encode_uint8(msg.token) != dataenc.OK) {
		return 1;
	}

	//if message is of type "new DC connected", encode the deviceId"
	if (msg.msgType == 3) {
		char* cstr = new char[msg.newDeviceId.length()];
		strcpy(cstr, msg.newDeviceId.c_str());
		if (dataenc.encode_byte_string_n255(cstr, msg.newDeviceId.size()) != dataenc.OK) {
			return 1;
		}
	}
	//Otherways encode the dataPairId and payload
	else {
		if (dataenc.encode_uint32(msg.dataPairId) != dataenc.OK) {
			return 1;
		}
		if (dataenc.encode_uint32(msg.payload) != dataenc.OK) {
			return 1;
		}
	}
	return 0;
}