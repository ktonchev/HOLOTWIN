#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "MQTTAsync.h"
#include "msgReader.h"
#include "msgWriter.h"
#include "processRequest.h"
#include "databank.hpp"
#include "StreamClient.hpp"
#include "connection.hpp"
#include <thread>

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif


#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
              

std::string &clientId = *get_clientIdPtr();

dataBank &db = *get_dbPtr();
StreamClient client;
unsigned int arr[4][4];

//Function that reads the client ID and topic from the input stream and saves them into the parameter variables 
void inputParams() {

	std::cout << "Please enter client ID\n";
	std::cin >> clientId;


}

void serializeArray(char* buf, int& size, unsigned int *arr) {

	unsigned int dims[2] = { 4,4 };

	Encoder dataenc;
	dataenc.Reset(buf);




}

void _stream() {
	uint32_t val = 0;
	int rc = db.getDataPairById(2, val);
	while (1) {
		rc = db.getDataPairById(2, val);
		if (!rc && val > 500) {
			client.SendData(const_cast<unsigned char*>((const unsigned char*)"data"), 3, true);
		}
		Sleep(5000);
	}
}

void initArray() {
	for(int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++) {
			arr[i][j] = j;
		}
	}
}

int main(int argc, char* argv[]) {

	initArray();


	int rc = 0;

	//Declaring 2 data pairs for testing
	dataPair dp1(1, 101, GETTABLE);
	dataPair dp2(2, 102, BOTH);

	//Adding datapairs to dataBank
	db.addDataPair(dp1);
	db.addDataPair(dp2);

	//Reading user input for client ID and topic
	//inputParams();
	clientId = "dc1";

	InitMQTT_dc(ADDRESS);
	//Wait while session is formed
	while (!get_connected());

	//Subscribe to own topic
	std::cout << "Waiting for subscribtion\n";
	subscribe_dc(clientId);
	//Wait until the client is subscribed
	while (!get_subscribed());

	//Timestamp
	const auto p1 = std::chrono::system_clock::now();

	//Constructing a "New DC connected message"
	msgstruct alert;
	alert.msgType = 3;
	alert.timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
	alert.token = 1;
	alert.newDeviceId = clientId;
	//Publishing the message on the raise topic
	sendMsg(alert,"raise");


	//Init StreamClient
	std::string message;
	client.Init(const_cast<char*>("127.0.0.1"), const_cast<char*>("23"), const_cast<char*>("client"));
	std::thread* _SendThread;
	_SendThread = new std::thread(_stream);


	//Wait for requests
	std::cout << "Press 'q' to quit\n";
	char c;
	while (1) {
		std::cin >> c;
		if (c == 'q') {
			break;
		}
		else {
		
			

		}
	}

	if (get_connected()) {
		disconnect();//Disconnect the client from the MQTT broker
	}

	DeinitMQTT();
	client.~StreamClient();
	return rc;
}

