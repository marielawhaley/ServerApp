#pragma once

extern "C"
{
#include <memory.h>
#include <MQTTClient.h>
#include <MQTTClientPersistence.h>
#include <signal.h>
#include <stdio.h>
#if defined(WIN32)
#include <Windows.h>
#define sleep Sleep
#else
#include <stdlib.h>
#include <time.h>
#endif
}
#include "MySQL.h"


using namespace System;


class MQTTApp
{
private:
	int toStop = 0;
	int quietMode = 0;
	int count = 0;
	MySQL database;
	char* Mssg;

public:
	MQTTApp(int opt, String^ topic, String^ mess);
	~MQTTApp();
	int Subscribe(char* brokerUrl, char* clientId, char* topicName, int qos); 
	//int Publish(char* brokerUrl, char* clientId, char* topicName, int qos, char* message);

	int MQTTApp::Publish(MQTTClient client, char*topicName, char * message);

	char* getMessage();
};