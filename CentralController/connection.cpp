#include "connection.hpp"

volatile MQTTAsync_token deliveredtoken;

//Declaring variables for the clientId and topic
static std::string clientId;

//Declaring flags
static bool rdyToSend = false;
static bool subscribed = false;
static bool connected = false;
static dataBank db;

static int curToken;

static MQTTAsync client;//MQTT client handle
std::vector<std::string> deviceControllers;


bool get_rdyToSend() {
	return rdyToSend;
}

bool get_subscribed() {
	return subscribed;
}

bool get_connected() {
	return connected;
}
int get_curToken() {
	return curToken;
}

void set_rdyToSend(bool val) {
	rdyToSend = val;
}

void set_subscribed(bool val) {
	subscribed = val;
}

void set_connected(bool val) {
	connected = val;
}

void set_curToken(int val) {
	curToken = val;
}


dataBank* get_dbPtr() {
	return std::addressof(db);
}

std::string* get_clientIdPtr() {
	return std::addressof(clientId);
}

std::vector<std::string>* get_deviceControllersPtr() {
	return std::addressof(deviceControllers);
}

//Initialize communication for a device controller
int InitMQTT_dc(std::string address) {
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;//Connection options structure
	int rc;//Return code

    //Creating the client
	MQTTAsync_create(&client, address.c_str(), clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	//Setting callback functions
	MQTTAsync_setCallbacks(client, client, connlost, msgarrvd_dc, NULL);

	conn_opts.keepAliveInterval = 20;//Specifying the interval at which the "keep alive" messages are sent
	conn_opts.cleansession = 1;//Specifying whether to discard or keep the state information at connect and disconnect
	conn_opts.onSuccess = onConnect;//Specifying the event function for a successful connection
	conn_opts.onFailure = onConnectFailure;//Specifying the event function for an usuccessful connection
	conn_opts.context = client;

	//Attemting to connect to MQTT broker
	std::cout << "Waiting for connection\n";
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {

		std::cout << "Failed to start connect, return code " << rc << "\n";
		return rc;
	}
	return rc;
}


//Initialize communication for a central controller
int InitMQTT_cc(std::string address) {
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;//Connection options structure
	int rc;//Return code

	//Creating the client
	MQTTAsync_create(&client, address.c_str(), clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	//Setting callback functions
	MQTTAsync_setCallbacks(client, client, connlost, msgarrvd_cc, NULL);

	conn_opts.keepAliveInterval = 20;//Specifying the interval at which the "keep alive" messages are sent
	conn_opts.cleansession = 1;//Specifying whether to discard or keep the state information at connect and disconnect
	conn_opts.onSuccess = onConnect;//Specifying the event function for a successful connection
	conn_opts.onFailure = onConnectFailure;//Specifying the event function for an usuccessful connection
	conn_opts.context = client;

	//Attemting to connect to MQTT broker
	std::cout << "Waiting for connection\n";
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {

		std::cout << "Failed to start connect, return code " << rc << "\n";
		return rc;
	}
	return rc;
}

void DeinitMQTT() {

	if (connected) {
		disconnect();
	}
	MQTTAsync_destroy(&client);//Deallocate the client handle

}

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

//Subscribtion success event function for central controller
void onSubscribe_cc(void* context, MQTTAsync_successData* response) {

	std::cout << "Subscribe succeeded\n";
	subscribed = true;

}

//Subscribtion failed event function for central controller
void onSubscribeFailure_cc(void* context, MQTTAsync_failureData* response) {

	std::cout << "Subscribe failed, rc " << (response ? response->code : 0) << "\n";

}

//Subscribtion success event function for device controller
void onSubscribe_dc(void* context, MQTTAsync_successData* response) {

	std::cout << "Subscribe succeeded\n";
	subscribed = true;

}

//Subscribtion failed event function for device controller
void onSubscribeFailure_dc(void* context, MQTTAsync_failureData* response) {

	std::cout << "Subscribe failed, rc " << (response ? response->code : 0) << "\n";

}

//Function used to send a message with a specific payload
void sendMsg(msgstruct msg, std::string topic) {

	//Setting the "ready to send" flag to false
	rdyToSend = false;

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

//Function used to recieve messages on device controller
//This function is a callback function, that is called when there is a new message available on the subscribed topic.
//The callback functions are specified using MQTTAsync_setCallbacks()
int msgarrvd_dc(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {

	MQTTAsync client = (MQTTAsync)context;
	char* payloadptr;//Mesagge payload pointer
	payloadptr = (char*)message->payload;

	//Deseriallizing the message
	msgstruct msg;
	
	int rc = readMsg(payloadptr, msg);

	std::cout << "\n msgtype: " << (int)msg.msgType << " timestamp: " << msg.timestamp << " token: " << (unsigned int)msg.token << " datapairid: " << msg.dataPairId << " payload: " << msg.payload << " return code: " << rc << " end" << "\n";

	//Checking if msgType is valid
	if (msg.msgType > 2) {
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		return 1;
	}
	//Discarding the message if it is a response
	if (msg.msgType == 0) {
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		return 1;
	}
	//Performing a get request if the message is of type get
	if (msg.msgType == 1) {
		msgstruct response;
		processGetRequest(msg, response, db);
		sendMsg(response, clientId);
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		return 1;
	}
	//Performing a set request if the message is of type set
	if (msg.msgType == 2) {
		msgstruct response;
		processSetRequest(msg, response, db);
		sendMsg(response, clientId);
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		return 1;
	}

	//Freeing allocated memory
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;

}


//Function used to recieve messages on centrall controller
//This function is a callback function, that is called when there is a new message available on the subscribed topic.
//The callback functions are specified using MQTTAsync_setCallbacks()
int msgarrvd_cc(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {

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
			subscribe_cc(msg.newDeviceId);
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

//Function used to subscribe a client to a topic
void subscribe_cc(std::string _topic) {

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	std::cout << "Subscribing to topic " << _topic << " for client " << clientId << " using QoS" << QOS << "\n\n";
	opts.onSuccess = onSubscribe_cc;//Specifying the event function for a succesful subscribtion
	opts.onFailure = onSubscribeFailure_cc;//Specifying the event function for an unsuccesfull subscribtion 
	opts.context = client;
	deliveredtoken = 0;

	if (_topic != "raise") {
		deviceControllers.push_back(_topic);
	}

	//Attempting to subscribe to a topic
	if ((rc = MQTTAsync_subscribe(client, _topic.c_str(), QOS, &opts)) != MQTTASYNC_SUCCESS) {

		std::cout << "Failed to start subscribe, return code " << rc << "\n";
		exit(EXIT_FAILURE);
		subscribed = false;

	}
}

//Function used to subscribe a client to a topic
void subscribe_dc(std::string _topic) {

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	std::cout << "Subscribing to topic " << _topic << " for client " << clientId << " using QoS" << QOS << "\n\n";
	opts.onSuccess = onSubscribe_dc;//Specifying the event function for a succesful subscribtion
	opts.onFailure = onSubscribeFailure_dc;//Specifying the event function for an unsuccesfull subscribtion 
	opts.context = client;

	deliveredtoken = 0;

	//Attempting to subscribe to a topic
	if ((rc = MQTTAsync_subscribe(client, _topic.c_str(), QOS, &opts)) != MQTTASYNC_SUCCESS) {

		std::cout << "Failed to start subscribe, return code " << rc << "\n";
		exit(EXIT_FAILURE);
		subscribed = false;

	}
}

//Function used to disconnect the client from the MQTT broker
int disconnect() {

	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	opts.onSuccess = onDisconnect;//Specifying event function for succesful disconnection
	opts.context = client;

	//Attemoting to disconnect from the MQTT broker
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
		std::cout << "Failed to start sendMessage, return code " << rc << "\n";
		return rc;
	}
	return rc;
}