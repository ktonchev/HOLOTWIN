#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include "MQTTAsync.h"
#include "msgReader.h"
#include "msgWriter.h"
#include "processRequest.h"
#include "databank.hpp"
#include <chrono>
#include "StreamServer.hpp"
#include "connection.hpp"



#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
              
/*
#define ADDRESS     "tcp://192.168.0.113:1883" //Address of MQTT broker
#define QOS         1 //Setting the Quality of Service level
volatile MQTTAsync_token deliveredtoken;


//Declaring variables for the clientId and topic
std::string clientId;

//Declaring flags
bool rdyToSend = false;
bool subscribed = false;
bool subscribedToClientTopic = false;
bool connected = false;
int curToken = 0;

//vector used for storing the deviceId of evry connected DC
std::vector<std::string> deviceControllers;

 //Connection lost event function
 //This is a callback function, which is called when the conection is lost. Callback functions are specified using MQTTAsync_setCallbacks()
void connlost(void* context, char* cause) {

	//Set connected flag to false
	connected = false;


	MQTTAsync client = (MQTTAsync)context;//Construct client handle from context
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;//Connection options
	int rc;//Return code

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	//Attempting reconection
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		std::cout << "Failed to start connect, return code " << rc << "\n";
		subscribed = false;
	}
	else {
		connected = true;
	}

}

//Disconnect event function
void onDisconnect(void* context, MQTTAsync_successData* response) {

	std::cout << "Successful disconnection\n";
	connected = false;
	subscribed = false;

}

//Message sent event function
void onSend(void* context, MQTTAsync_successData* response) {

	//Set the "ready to send" flag to true, as soon as the message is send
	rdyToSend = true;
	//Print out token number
	std::cout << "Message with token value " << response->token << " delivery confirmed\n";

}

//Connection failed event function
void onConnectFailure(void* context, MQTTAsync_failureData* response) {

	//Print out error code, if any
	std::cout << "Connect failed, rc " << (response ? response->code : 0) << "\n";
	connected = false;//Set connected flag to false

}

//Connection success event function
void onConnect(void* context, MQTTAsync_successData* response) {

	std::cout << "Successful connection\n";
	rdyToSend = true;
	connected = true;

}

//Subscribtion success event function
void onSubscribe(void* context, MQTTAsync_successData* response) {
	std::cout << "Subscribe succeeded\n";
	subscribed = true;

}

//Subscribtion failed event function
void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {

	std::cout << "Subscribe failed, rc " << (response ? response->code : 0) << "\n";

}


//Function used to send a message with a specific payload
void sendMsg(void* context, msgstruct msg, std::string topic) {

	//Setting the "ready to send" flag to false
	rdyToSend = false;

	//Constructing the MQTT client from context
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	opts.onSuccess = onSend;//Specifying event function for succesful send event
	opts.context = client;

	char msgbuf[160];
	char* bufptr = msgbuf;

	//Serializing the message
	writeMsg(bufptr, msg);


	pubmsg.payload = (void*)bufptr;//Setting message payload
	pubmsg.payloadlen = 160;//Setting message payload length
	pubmsg.qos = QOS;//Setting message quality of survice parameter 
	pubmsg.retained = 0;//Specifying whether a copy of the message should be retained or not
	deliveredtoken = 0;

	//Attempting to send the message
	if ((rc = MQTTAsync_sendMessage(client, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		std::cout << "Failed to start sendMessage, return code " << rc << "\n";
		exit(EXIT_FAILURE);
	}

}

//Function used to subscribe a client to a topic
void subscribe(void* context, std::string topic) {

	//Constructing client handle from context
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	std::cout << "Subscribing to topic " << topic << " for client " << clientId << " using QoS" << QOS << "\n\n";
	opts.onSuccess = onSubscribe;//Specifying the event function for a succesful subscribtion
	opts.onFailure = onSubscribeFailure;//Specifying the event function for an unsuccesfull subscribtion 
	opts.context = client;
	deliveredtoken = 0;

	if (topic !="raise") {
		deviceControllers.push_back(topic);
	}

	//Attempting to subscribe to a topic
	if ((rc = MQTTAsync_subscribe(client, topic.c_str(), QOS, &opts)) != MQTTASYNC_SUCCESS) {

		std::cout << "Failed to start subscribe, return code " << rc << "\n";
		exit(EXIT_FAILURE);
		subscribed = false;

	}
}

//Function used to disconnect the client from the MQTT broker
void disconnect(void* context) {

	//Constructing client handle from context
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	opts.onSuccess = onDisconnect;//Specifying event function for succesful disconnection
	opts.context = client;

	//Attemoting to disconnect from the MQTT broker
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
		std::cout << "Failed to start sendMessage, return code " << rc << "\n";
		exit(EXIT_FAILURE);
	}

}

//Function used to recieve messages
//This function is a callback function, that is called when there is a new message available on the subscribed topic.
//The callback functions are specified using MQTTAsync_setCallbacks()
int msgarrvd(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {

	MQTTAsync client = (MQTTAsync)context;
	std::string msgTopic(topicName, topicLen);
	char* payloadptr;//Mesagge payload pointer
	payloadptr = (char*)message->payload;

	//Deseriallize message
	msgstruct msg;
	int rc = readMsg(payloadptr, msg);

	//std::cout << "\n msgtype: " << (int)msg.msgType << " timestamp: " << msg.timestamp << " token: " << msg.token << " datapairid: " << msg.dataPairId << " payload: " << msg.payload << " return code: " << rc <<" end" << "\n";

	//if the message is on the raise topic check if the message is a "new DC connected" message...
	if (msgTopic == "raise") {
		if (msg.msgType == 3) {
			//... if so subscribe to the DC's topic
			subscribe(client, msg.newDeviceId);
			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}
	}
	else {
		//Check if the msgType is valid
		if (msg.msgType > 2) {
			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}
		//if the message is a response check if the token coresponds with a previously sent request
		if (msg.msgType == 0) {

			if (msg.token != (uint8_t)curToken) {
				std::cout << " Bad token!\n";
			}
			//if the token is ok, check if the dataPairId is zero (which means there was an error) and if not print the response payload
			else {
				if (msg.dataPairId == 0) {
					if (msg.payload == 1) {
						std::cout << "Databank is empty!\n";
					}
					else if (msg.payload == 2) {
						std::cout << "No such datapair!\n";
					}
					else if (msg.payload == 3) {
						std::cout << "Access denied!\n";
					}
				}
				else {
					std::cout << "Response payload: " << msg.payload << "\n";
				}
			}
		

			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}
		if (msg.msgType == 1) {
			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}
		if (msg.msgType == 2) {
			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}
	}

	//Freeing allocated memory
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;

}


*/


//vector used for storing the deviceId of evry connected DC
std::vector<std::string>& deviceControllers = *get_deviceControllersPtr();


std::string& clientId = *get_clientIdPtr();

//Function that reads the client ID and topic from the input stream and saves them into the parameter variables 
void inputParams() {

	std::cout << "Please enter client ID\n";
	std::cin >> clientId;

}

int main(int argc, char* argv[]) {

	int rc = 0;

	//Reading user input for client ID and topic
	//inputParams();
	clientId = "central";

	InitMQTT_cc(ADDRESS);
	//Wait while session is formed
	while (!get_connected());

	//Subscribe to own topic
	std::cout << "Waiting for subscribtion\n";
	subscribe_cc("raise");
	//Wait until the client is subscribed
	while (!get_subscribed());

	//Timestamp
	const auto p1 = std::chrono::system_clock::now();

	//Constructing a "New DC connected message"
	msgstruct msg;
	int msgtype = 0;
	uint32_t timestamp = 0;
	uint8_t token = 0;
	uint32_t datapairid = 0;
	uint32_t data = 0;
	int dc;
	std::string dcid;

	// Create ethernet server and check for errors
	std::thread* _ReceiveThread;
	StreamServer* server = new StreamServer(23);
	if (int rc = server->GetLatestError()) {
		std::cout << "Error encountered rc = " << rc << "\n";
	}
	else {
		// Create data receive thread
		server->SetReceiveThread(true);
		_ReceiveThread = new std::thread(&StreamServer::Receive, server);
	}






	//LOOP------------------------------------------------------------------------------------------->
	while (1) {
		if (get_connected() && get_subscribed()) {
			//only attempt to send message if there are dcs connected
			if (deviceControllers.size() >0) {
				//only attempt to send a message if the previous message has been sent
				if (get_rdyToSend() == true) {
					std::cout << "Enter msgtype (1 = get, 2 = set, 3 = quit): \n";
					std::cin >> msgtype;
					if (msgtype == 3) {
						break;
					}
					else {
						//handle user input and send a request

						std::cout << "Choose which device to send message to: \n";
						for (int i = 0; i < deviceControllers.size(); i++) {
							std::cout << i << ". " << deviceControllers.at(i) << "\n";
						}
						std::cin >> dc;
						dcid = deviceControllers[dc];
						msg.msgType = msgtype;
						std::cout << "enter data pair id : \n";
						std::cin >> datapairid;
						std::cout << "enter data: \n";
						std::cin >> data;

						set_curToken(rand());
						
						msg.token = get_curToken();

						const auto p1 = std::chrono::system_clock::now();
						msg.timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();

						msg.dataPairId = datapairid;
						msg.payload = data;

						sendMsg(msg, dcid);
					}
				}
			}
			else {
				//std::cout << "no device controllers connected!" << "\n";
			}
		}
		else {
			break;
		}
	}
	//LOOP------------------------------------------------------------------------------------------->





	if (get_connected()) {
		disconnect();//Disconnect the client from the MQTT broker
	}

	DeinitMQTT();
	return rc;
}

