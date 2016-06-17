#include "MQTTApp.h"
#include <iostream>
#include "signal.h"
//#include <vcclr.h>
//#include <atlstr.h>


int toStop = 0;
void handleSignal(int sig)
{
	toStop = 1;
}

//Initialize connection and subscribe to topic
MQTTApp::MQTTApp(int opt, String^ top, String^ mess)
{
	int rc = 0;
	char url[256];
	char clientId[24];
	char message[20] = { 0 };
	char topic[17];

	sprintf(topic, "%s", top);
	
	// Default settings:
	char* action = "publish";
	if (mess->Length < sizeof(topic)) // make sure it fits & allow space for null terminator
		sprintf(message, "%s", mess);

	int qos = 2;
	char* broker = "85.119.83.194"; //IP of Mosquitto broker
	char* port = "1883"; // standard port
	int i = 0;


	typedef void(*SignalHandlerPointer)(int);

	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	
	// Validate the provided arguments
	if (opt > 1)
	{
		std::cout <<"Invalid action:";
	}

	// Construct the full broker URL and clientId
	sprintf(url, "tcp://%s:%s", broker, port);
	sprintf(clientId, "SampleCV3_%s", action);

	if (opt == 0)
	{
		std::cout << "publish";
		//rc = Publish(url, clientId, topic, qos, message);

	}
	else if (opt == 1)
	{
		std::cout << "subscribe";

		rc = Subscribe(url, clientId, topic, qos);
	}

}

MQTTApp::~MQTTApp()
{

}

char* MQTTApp::getMessage()
{
	return Mssg;
}

//Susbscribe method called from the constructor
// Subscribes to general topic "AD/" 
//On message receive, take action depending on message topic
int MQTTApp::Subscribe(char* brokerUrl, char* clientId, char* topicName, int qos)
{
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;
	char mbedValuesTopic[30] = "AD/mbed-server/values"; // live values sent from Desk Egg
	char mbedFeedbackTopic[30] = "AD/mbed-server/feedback"; // on feedback response from Desk Egg
	char mbedRequestHistory[30] = "AD/UI-server/request";// on request of history values from UI
	char * pointerValues;
	char * pointerFeedback;
	char * pointerHistory;

	// Create the client instance
	rc = MQTTClient_create(&client, brokerUrl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create MQTT Client instance\n");
		return rc;
	}

	// Create the connection options
	conn_opts.keepAliveInterval = 100;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 1;

	// Connect the client
	rc = MQTTClient_connect(client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		MQTTClient_destroy(&client);
		return rc;
	}

	if (!quietMode)
		printf("Connected to %s\n", brokerUrl);

	// Subscribe to the topic
	if (!quietMode)
		printf("Subscribing to topic \"%s\" qos %d\n", topicName, qos);

	rc = MQTTClient_subscribe(client, topicName, 2);

	if (rc != MQTTCLIENT_SUCCESS)
		printf("Failed to subscribe, return code %d\n", rc);
	else
	{
	
		long receiveTimeout = 100l;

		while (!toStop)
		{
			char* receivedTopicName = NULL;
			int topicLen;
			MQTTClient_message* message = NULL;
			// Check to see if a message is available
			rc = MQTTClient_receive(client, &receivedTopicName, &topicLen, &message, receiveTimeout);

			if (rc == MQTTCLIENT_SUCCESS && message != NULL)
			{
				array< String^>^ dataArray = gcnew array<String^>(5);
				// A message has been received
				printf("Topic:\t\t%s\n", receivedTopicName);
				printf("Message:\t%.*s\n", message->payloadlen, (char*)message->payload);
				printf("QoS:\t\t%d\n", message->qos);
		
				Mssg = (char*)message->payload;
				String^ mssgString = gcnew String(Mssg);
				Console::WriteLine(mssgString);
				Char delimiter = ',';
				dataArray = mssgString->Split(delimiter);
				pointerValues = mbedValuesTopic;
				pointerFeedback = mbedFeedbackTopic;
				pointerHistory = mbedRequestHistory;
				if (strcmp(receivedTopicName, pointerValues) == 0) // if message topic is "AD/mbed-server/values"
				{		//Insert all values to database
						database.insertValues(dataArray[0], dataArray[1], dataArray[2], dataArray[3], dataArray[4]);
				}
				else if (strcmp(receivedTopicName, pointerFeedback) == 0) // if message topic "AD/mbed-server/feedback"
				{
					database.insertFeedback(dataArray[0], dataArray[1]); // insert to feedback database table
					int parameter = Convert::ToInt32(dataArray[0]); // string to int
					float value;
					char message[10];
					char *fbTopic = "AD/server-UI/feedback";
					char* pointerMessag;

					System::Single::TryParse(dataArray[1], value);
					sprintf(message, "%d, %.2f",parameter, value);
					pointerMessag = message;		
					Console::Write("Message :");
					Console::Write(message);
					// Publish to user interface
					Publish(client, fbTopic, pointerMessag);
						
				}
				else if (strcmp(receivedTopicName, pointerHistory) == 0) // if topic message is "AD/mbed-server/history"
				{
					MySQL database;
					double tempArr[10];
					array<String^>^ ToQuery = gcnew array<String^>{"temperature", "dew_point", "dust", "noise", "ambient_light"};;
					char message[40];
					std::string topicDir[5] = { "AD/server-UI/history/temp","AD/server-UI/history/dew_point","AD/server-UI/history/dust","AD/server-UI/history/noise","AD/server-UI/history/light", };
					char* ptrmssg;
					for (int i = 0; i < 5; i++)
					{
						Console::Write("message");
						database.queryValues(ToQuery[i], tempArr); // get values from database
						sprintf(message, "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f", tempArr[0], tempArr[1], tempArr[2], tempArr[3], tempArr[4], tempArr[5], tempArr[6], tempArr[7]);
						ptrmssg= message;

						//arrange database values to *char values
						char * writable = new char[topicDir[i].size() + 1];
						std::copy(topicDir[i].begin(), topicDir[i].end(), writable);
						writable[topicDir[i].size()] = '\0'; 

						//Publish to user interface
						Publish(client, writable, ptrmssg);

						// free char string
						delete[] writable;
					}
				}

				MQTTClient_freeMessage(&message);
				MQTTClient_free(receivedTopicName);
				printf("count%d", count);
				count++;
			}
			else if (rc != MQTTCLIENT_SUCCESS)
			{
				printf("Failed to received message, return code %d\n", rc);
				toStop = 1;
			}
		  }
		}
		// Disconnect the client
		MQTTClient_disconnect(client, 0);

		if (!quietMode)
			printf("Disconnected\n");

		// Free the memory used by the client instance
		MQTTClient_destroy(&client);

		return rc;
}

// Publish message 
//Paramin-
//MQTTClient: client which initiated connection.
//topicName = topic to be published in
// message: Message to be published
int MQTTApp::Publish(MQTTClient client, char*topicName, char * message)
{
	int rc;
	// Construct the message to publish
	MQTTClient_message mqttMessage = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	mqttMessage.payload = message;
	mqttMessage.payloadlen = strlen(message);
	mqttMessage.qos = 1;
	mqttMessage.retained = 0;
	//MQTTClient client_ = *client;

	// Publish the message
	if (!quietMode)
		printf("Publishing to topic \"%s\" qos %d\n", topicName, 1);

	rc = MQTTClient_publishMessage(client, topicName, &mqttMessage, &token);

	if (rc != MQTTCLIENT_SUCCESS)
		printf("Message not accepted for delivery, return code %d\n", rc);
	else
	{
		// Wait until the message has been delivered to the server
		rc = MQTTClient_waitForCompletion(client, token, 10000L);

		if (rc != MQTTCLIENT_SUCCESS)
			printf("Failed to deliver message, return code %d\n", rc);
	}
	return rc;

}

