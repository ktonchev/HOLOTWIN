#pragma once
#include "MQTTAsync.h"
#include "msgReader.h"
#include "msgWriter.h"
#include "processRequest.h"
#include "databank.hpp"
#include <cstring>
#include <functional>


//template <typename T>
//struct Callback;
//
//template <typename Ret, typename... Params>
//struct Callback<Ret(Params...)> {
//	template <typename... Args>
//	static Ret msgarrvd_cb(Args... args) {
//		return _msgarrvd(args...);
//	}
//	static std::function<Ret(Params...)> _msgarrvd;
//
//};
//
//template <typename Ret, typename... Params>
//std::function<Ret(Params...)> Callback<Ret(Params...)>::_msgarrvd;



//typedef int (*callback_t)(void*, char*, int, MQTTAsync_message*);

class MQTTDeviceController {
public:
	MQTTAsync client;//MQTT client handle
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;//Connection options structure
	dataBank db;
	std::string clientId;
	int qos = 1;

	//Declaring flags
	bool rdyToSend = false;
	bool subscribed = false;
	bool connected = false;

	volatile MQTTAsync_token deliveredtoken;

	MQTTAsync_messageArrived* msgarrvd_ptr;
	MQTTAsync_connectionLost* connlost_ptr;
	MQTTAsync_onSuccess* onConnect_ptr;
	MQTTAsync_onSuccess* onDisconnect_ptr;
	MQTTAsync_onSuccess* onSend_ptr;
	MQTTAsync_onSuccess* onSubscribe_ptr;
	MQTTAsync_onFailure* onSubscribeFailure_ptr;
	MQTTAsync_onFailure* onConnectFailure_ptr;



	MQTTDeviceController(std::string address, std::string clientId, dataBank _db) {

		int rc;

		//Creating the client
		MQTTAsync_create(&client, address.c_str(), clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
		//Setting callback functions
//		MQTTAsync_setCallbacks(client, client, &this->connlost, &this->msgarrvd, NULL);
		//void (MQTTDeviceController::*pmf)(void*, char*);
		//pmf = &MQTTDeviceController::connlost;
		//*pmf()

		//MQTTAsync_setCallbacks(client, client, this->connlost, &MQTTDeviceController::msgarrvd, NULL);
		//Callback<int(void*, char*, int, MQTTAsync_message*)>::msgarrvd_cb = std::bind(&MQTTDeviceController::msgarrvd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		//callback_t msgarrvd_ptr = static_cast<callback_t>(Callback<int(int, int)>::msgarrvd_cb);
		//MQTTAsync_setCallbacks(client, client, NULL, msgarrvd_ptr, NULL);


		std::function<int(void*, char*, int, MQTTAsync_message*)> msgarrvdBinder = std::bind(&MQTTDeviceController::msgarrvd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		std::function<void(void*, char*)> connlostBinder = std::bind(&MQTTDeviceController::connlost, this, std::placeholders::_1, std::placeholders::_2);
		std::function<void(void* , MQTTAsync_successData*)> onDisconnectBinder = std::bind(&MQTTDeviceController::onDisconnect, this, std::placeholders::_1, std::placeholders::_2);
		std::function<void(void*, MQTTAsync_successData*)> onConnectBinder = std::bind(&MQTTDeviceController::onConnect, this, std::placeholders::_1, std::placeholders::_2);
		std::function<void(void*, MQTTAsync_successData*)> onSendBinder = std::bind(&MQTTDeviceController::onSend, this, std::placeholders::_1, std::placeholders::_2);
		std::function<void(void*, MQTTAsync_successData*)> onSubscribeBinder = std::bind(&MQTTDeviceController::onSubscribe, this, std::placeholders::_1, std::placeholders::_2);
		std::function<void(void*, MQTTAsync_failureData*)> onSubscribeFailureBinder = std::bind(&MQTTDeviceController::onSubscribeFailure, this, std::placeholders::_1, std::placeholders::_2);
		std::function<void(void*, MQTTAsync_failureData*)> onConnectFailureBinder = std::bind(&MQTTDeviceController::onConnectFailure, this, std::placeholders::_1, std::placeholders::_2);


		msgarrvd_ptr = (MQTTAsync_messageArrived*)*(long*)(char*)(&msgarrvdBinder);
		connlost_ptr = (MQTTAsync_connectionLost*)*(long*)(char*)(&connlostBinder);
		onDisconnect_ptr = (MQTTAsync_onSuccess*)*(long*)(char*)(&onDisconnectBinder);
		onConnect_ptr = (MQTTAsync_onSuccess*)*(long*)(char*)(&onConnectBinder);
		onSend_ptr = (MQTTAsync_onSuccess*)*(long*)(char*)(&onSendBinder);
		onSubscribe_ptr = (MQTTAsync_onSuccess*)*(long*)(char*)(&onSubscribeBinder);
		onSubscribeFailure_ptr = (MQTTAsync_onFailure*)*(long*)(char*)(&onSubscribeFailureBinder);
		onConnectFailure_ptr = (MQTTAsync_onFailure*)*(long*)(char*)(&onConnectFailureBinder);


		MQTTAsync_setCallbacks(client, client, connlost_ptr, msgarrvd_ptr, NULL);

		conn_opts.keepAliveInterval = 20;//Specifying the interval at which the "keep alive" messages are sent
		conn_opts.cleansession = 1;//Specifying whether to discard or keep the state information at connect and disconnect
		conn_opts.onSuccess = onConnect_ptr;//Specifying the event function for a successful connection
		conn_opts.onFailure = onConnectFailure_ptr;//Specifying the event function for an usuccessful connection
		conn_opts.context = client;

		
		//Attemting to connect to MQTT broker
		std::cout << "Waiting for connection\n";
		if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {

			std::cout << "Failed to start connect, return code " << rc << "\n";
			exit(EXIT_FAILURE);
		}
		//Wait while session is formed
		while (!connected);
		
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

		opts.onSuccess = onSend_ptr;//Specifying event function for succesful send event
		opts.context = client;

		char msgbuf[160];
		char* bufptr = msgbuf;

		//Serializing the message
		writeMsg(bufptr, msg);


		pubmsg.payload = (void*)bufptr;//Setting message payload
		pubmsg.payloadlen = 160;//Setting message payload length
		pubmsg.qos = qos;//Setting message quality of survice parameter 
		pubmsg.retained = 0;//Specifying whether a copy of the message should be retained or not
		deliveredtoken = 0;

		//Attempting to send the message
		if ((rc = MQTTAsync_sendMessage(client, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
			std::cout << "Failed to start sendMessage, return code " << rc << "\n";
			exit(EXIT_FAILURE);
		}

	}

	//Function used to subscribe a client to a topic
	void subscribe(void* context, std::string _topic) {

		//Constructing client handle from context
		MQTTAsync client = (MQTTAsync)context;
		MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
		int rc;

		std::cout << "Subscribing to topic " << _topic << " for client " << clientId << " using qos" << qos << "\n\n";
		opts.onSuccess = onSubscribe_ptr;//Specifying the event function for a succesful subscribtion
		opts.onFailure = onSubscribeFailure_ptr;//Specifying the event function for an unsuccesfull subscribtion 
		opts.context = client;

		deliveredtoken = 0;

		//Attempting to subscribe to a topic
		if ((rc = MQTTAsync_subscribe(client, _topic.c_str(), qos, &opts)) != MQTTASYNC_SUCCESS) {

			std::cout << "Failed to start subscribe, return code " << rc << "\n";
			exit(EXIT_FAILURE);
			subscribed = false;

		}
	}

	//Function used to disconnect the client from the MQTT broker
	void disconnect() {


		MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
		int rc;

		opts.onSuccess = onDisconnect_ptr;//Specifying event function for succesful disconnection
		opts.context = client;

		//Attemoting to disconnect from the MQTT broker
		if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
			std::cout << "Failed to start sendMessage, return code " << rc << "\n";
			exit(EXIT_FAILURE);
		}

	}

private:





	//Connection lost event function
    //This is a callback function, which is called when the conection is lost. Callback functions are specified using MQTTAsync_setCallbacks()
	void connlost(void* context, char* cause) {

		//Set connected flag to false
		connected = false;


		MQTTAsync client = (MQTTAsync)context;//Construct client handle from context
		MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;//Connection options
		int rc;//Return code

		std::cout << "\nConnection lost\n";
		std::cout << "     cause: "<< cause <<"\n";
		std::cout << "Reconnecting\n";

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

	//Function used to recieve messages
	//This function is a callback function, that is called when there is a new message available on the subscribed topic.
	//The callback functions are specified using MQTTAsync_setCallbacks()
	int msgarrvd(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {

		MQTTAsync client = (MQTTAsync)context;
		char* payloadptr;//Mesagge payload pointer
		payloadptr = (char*)message->payload;

		//Deseriallizing the message
		msgstruct msg;
		int rc = readMsg(payloadptr, msg);

		std::cout << "\n msgtype: " << (int)msg.msgType << " timestamp: " << msg.timestamp << " token: " << msg.token << " datapairid: " << msg.dataPairId << " payload: " << msg.payload << " return code: " << rc << " end" << "\n";

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
			sendMsg(client, response, clientId);
			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}
		//Performing a set request if the message is of type set
		if (msg.msgType == 2) {
			msgstruct response;
			processSetRequest(msg, response, db);
			sendMsg(client, response, clientId);

			MQTTAsync_freeMessage(&message);
			MQTTAsync_free(topicName);
			return 1;
		}

		//Freeing allocated memory
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		return 1;

	}

};