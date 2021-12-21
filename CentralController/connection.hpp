#ifndef conn
#define conn

#include "databank.hpp"
#include <cstring>
#include <iostream>
#include "MQTTAsync.h"
#include "msgstruct.h"
#include "msgReader.h"
#include "msgWriter.h"
#include "processRequest.h"


#define ADDRESS     "tcp://192.168.0.113:1883" //Address of MQTT broker
#define QOS         1 //Setting the Quality of Service level

bool get_rdyToSend();

bool get_subscribed();

bool get_connected();

int get_curToken();

void set_rdyToSend(bool val);

void set_subscribed(bool val);

void set_connected(bool val);

void set_curToken(int val);



dataBank* get_dbPtr();

std::string* get_clientIdPtr();

std::vector<std::string>* get_deviceControllersPtr();

int InitMQTT_dc(std::string address);

int InitMQTT_cc(std::string address);

void DeinitMQTT();

//Connection lost event function
 //This is a callback function, which is called when the conection is lost. Callback functions are specified using MQTTAsync_setCallbacks()
void connlost(void* context, char* cause);

//Disconnect event function
void onDisconnect(void* context, MQTTAsync_successData* response);

//Message sent event function
void onSend(void* context, MQTTAsync_successData* response);

//Connection failed event function
void onConnectFailure(void* context, MQTTAsync_failureData* response);

//Connection success event function
void onConnect(void* context, MQTTAsync_successData* response);

//Subscribtion success event function for central controller
void onSubscribe_cc(void* context, MQTTAsync_successData* response);

//Subscribtion failed event function for central contoller
void onSubscribeFailure_cc(void* context, MQTTAsync_failureData* response);

//Subscribtion failed event function for device controller
void onSubscribe_dc(void* context, MQTTAsync_successData* response);

//Subscribtion failed event function for device controller
void onSubscribeFailure_dc(void* context, MQTTAsync_failureData* response);


//Function used to send a message with a specific payload
void sendMsg(msgstruct msg, std::string topic);

//Function used to recieve messages on device controller
//This function is a callback function, that is called when there is a new message available on the subscribed topic.
//The callback functions are specified using MQTTAsync_setCallbacks()
int msgarrvd_dc(void* context, char* topicName, int topicLen, MQTTAsync_message* message);

//Function used to recieve messages on centrall controller
//This function is a callback function, that is called when there is a new message available on the subscribed topic.
//The callback functions are specified using MQTTAsync_setCallbacks()
int msgarrvd_cc(void* context, char* topicName, int topicLen, MQTTAsync_message* message);

//Function used to subscribe a centrall controller to a topic
void subscribe_cc(std::string _topic);

//Function used to subscribe a device controller to a topic
void subscribe_dc(std::string _topic);

//Function used to disconnect the client from the MQTT broker
int disconnect();

#endif