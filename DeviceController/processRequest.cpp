#include "processRequest.h"

//Function that is used to generate response to a get request
int processGetRequest(msgstruct &msg, msgstruct &response, dataBank &db) {

	uint32_t val = 0;
	int rc;

	//timestamp
	const auto p1 = std::chrono::system_clock::now();

	//try to get data pair from databank
	if (!(rc=db.getDataPairById(msg.dataPairId, val))) {
		response.msgType = 0;
		response.timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count(); 
		response.token = msg.token;
		response.dataPairId = msg.dataPairId;
		response.payload = val;
		return rc;
	}
	//if unsuccesful construct a response which has a dataPairId of 0 and a payload representing the error code
	else {
		response.msgType = 0;
		response.timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count(); 
		response.token = msg.token;
		response.dataPairId = 0;
		response.payload = rc;
		return rc;
	}

}

//Function that is used to generate response to a set request
int processSetRequest(msgstruct& msg, msgstruct& response, dataBank& db) {

	int rc;

	//timestamp
	const auto p1 = std::chrono::system_clock::now();

	//Try to set the value of a data pair
	if (!(rc = db.setDataPairById(msg.dataPairId, msg.payload))) {
		response.msgType = 0;
		response.timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
		response.token = msg.token;
		response.dataPairId = msg.dataPairId;
		response.payload = msg.payload;
		return rc;
	}
	//if unsuccesful construct a response which has a dataPairId of 0 and a payload representing the error code
	else {
		response.msgType = 0;
		response.timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
		response.token = msg.token;
		response.dataPairId = 0;
		response.payload = rc;
		return rc;
	}

}