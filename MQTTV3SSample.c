/*
 * -----------------------------------------------------------------
 * IBM Websphere MQ Telemetry
 * MQTTV3SSample MQTT v3 SSL Client application
 *
 * Version: @(#) MQMBID sn=p000-L130522.1 su=_M3QBMsMbEeK31Ln-reX3cg pn=com.ibm.mq.mqxr.listener/SDK/clients/c/samples/MQTTV3SSample.c
 *
 *   <copyright 
 *   notice="lm-source-program" 
 *   pids="5724-H72," 
 *   years="2010,2012" 
 *   crc="218290716" > 
 *   Licensed Materials - Property of IBM  
 *    
 *   5724-H72, 
 *    
 *   (C) Copyright IBM Corp. 2010, 2012 All Rights Reserved.  
 *    
 *   US Government Users Restricted Rights - Use, duplication or  
 *   disclosure restricted by GSA ADP Schedule Contract with  
 *   IBM Corp.  
 *   </copyright> 
 * -----------------------------------------------------------------
 */

/**
 * This sample application demonstrates basic usage
 * of the MQTT v3 SSL Client api.
 *
 * It can be run in one of two modes:
 *  - as a publisher, sending a single message to a topic on the server
 *  - as a subscriber, listening for messages from the server
 *
 */


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
#include <sys/time.h>
#endif

//Default settings:
struct Options
{
	char* action;
	char* topic;
	char* message ;
	int qos;
	char* broker;
	char* port;
	char* clientKey;
	char* clientPass;
	char* serverKey;
	int enableServerCertAuth;
} options =
{
	"publish",
	NULL,
	"Message from MQTTv3 SSL C client",
	2,
	"localhost",
	"8883",
	NULL,
	NULL,
	NULL,
	0
};


int subscribe(char* brokerUrl, char* clientId, struct Options options);
int publish(char* brokerUrl, char* clientId, struct Options options);

volatile int toStop = 0;
volatile int quietMode = 0;

void printHelp()
{
	printf("Syntax:\n\n");
	printf("    MQTTV3SSample [-h] [-a publish|subscribe] [-t <topic>] [-m <message text>]\n");
	printf("         [-s 0|1|2] [-b <hostname|IP address>] [-p <brokerport>] \n\n");
	printf("    -h  Print this help text and quit\n");
	printf("    -q  Quiet mode (default is false)\n");
	printf("    -a  Perform the relevant action (default is publish)\n");
	printf("    -t  Publish/subscribe to <topic> instead of the default\n");
	printf("            (publish: \"MQTTV3SSample/C/v3\", subscribe: \"MQTTV3SSample/#\")\n");
	printf("    -m  Use this message instead of the default (\"Message from MQTTv3 SSL C client\")\n");
	printf("    -s  Use this QoS instead of the default (2)\n");
	printf("    -b  Use this name/IP address instead of the default (localhost)\n");
	printf("    -p  Use this port instead of the default (8883)\n");
	printf("    -k  Use this PEM format keyfile as the client certificate (default none)\n");
	printf("    -w  Passphrase to unlock client certificate\n");
	printf("    -r  Use this PEM format keyfile to verify the server\n");
	printf("    -v  Set server certificate verification; 0 - off (default), 1 - on\n");
	printf("\nDelimit strings containing spaces with \"\"\n");
	printf("\nPublishers transmit a single message then disconnect from the broker.\n");
	printf("Subscribers remain connected to the broker and receive appropriate messages\n");
	printf("until Control-C (^C) is received from the keyboard.\n\n");
}


void handleSignal(int sig)
{
	toStop = 1;
}


/**
 * The main entry point of the sample.
 *
 * This method handles parsing the arguments specified on the
 * command-line before performing the specified action.
 */
int main(int argc, char** argv)
{
	int rc = 0;
	char url[256];
	char clientId[24];

	int i=0;

	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	quietMode = 0;
	// Parse the arguments -
	for (i=1; i<argc; i++)
	{
		// Check this is a valid argument
		if (strlen(argv[i]) == 2 && argv[i][0] == '-')
		{
			char arg = argv[i][1];
			// Handle the no-value arguments
			if (arg == 'h' || arg == '?')
			{
				printHelp();
				return 255;
			}
			else if (arg == 'q')
			{
				quietMode = 1;
				continue;
			}

			// Validate there is a value associated with the argument
			if (i == argc - 1 || argv[i+1][0] == '-')
			{
				printf("Missing value for argument: %s\n", argv[i]);
				printHelp();
				return 255;
			}
			switch(arg)
			{
			case 'a': options.action = argv[++i];      break;
			case 't': options.topic = argv[++i];       break;
			case 'm': options.message = argv[++i];     break;
			case 's': options.qos = atoi(argv[++i]);   break;
			case 'b': options.broker = argv[++i];      break;
			case 'p': options.port = argv[++i];        break;
			case 'k': options.clientKey = argv[++i];   break;
			case 'w': options.clientPass = argv[++i];  break;
			case 'r': options.serverKey = argv[++i];   break;
			case 'v': options.enableServerCertAuth = atoi(argv[++i]);  break;
			default:
				printf("Unrecognised argument: %s\n", argv[i]);
				printHelp();
				return 255;
			}
		}
		else
		{
			printf("Unrecognised argument: %s\n", argv[i]);
			printHelp();
			return 255;
		}
	}


	// Validate the provided arguments
	if (strcmp(options.action, "publish") != 0 && strcmp(options.action, "subscribe") != 0)
	{
		printf("Invalid action: %s\n", options.action);
		printHelp();
		return 255;
	}
	if (options.qos < 0 || options.qos > 2)
	{
		printf("Invalid QoS: %d\n", options.qos);
		printHelp();
		return 255;
	}
	if (options.topic == NULL || ( options.topic != NULL && strlen(options.topic) == 0) )
	{
		// Set the default topic according to the specified action
		if (strcmp(options.action, "publish") == 0)
			options.topic = "MQTTV3SSample/C/v3";
		else
			options.topic = "MQTTV3SSample/#";
	}

	// Construct the full broker URL and clientId
	sprintf(url, "ssl://%s:%s", options.broker, options.port);
	sprintf(clientId, "SampleCV3S_%s", options.action);

	if (strcmp(options.action, "publish") == 0)
		rc = publish(url, clientId, options);
	else
		rc = subscribe(url, clientId, options);

	return rc;
}


/**
 * Subscribes to a topic and blocks until Ctrl-C is pressed,
 * or the connection is lost.
 *
 * The sample demonstrates synchronous subscription, which
 * does not use callbacks.
 */
int subscribe(char* brokerUrl, char* clientId, struct Options options)
{
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
	int rc;

	// Create the client instance
	rc = MQTTClient_create(&client, brokerUrl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create MQTT Client instance\n");
		return rc;
	}

	// Create the connection options
	conn_opts.keepAliveInterval = 20;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 1;

	conn_opts.ssl = &ssl_opts;
	if (options.serverKey != NULL) conn_opts.ssl->trustStore = options.serverKey;
	if (options.clientKey != NULL) conn_opts.ssl->keyStore = options.clientKey;
	if (options.clientPass != NULL) conn_opts.ssl->privateKeyPassword = options.clientPass;
	conn_opts.ssl->enableServerCertAuth = options.enableServerCertAuth;

	// Connect the client
	rc = MQTTClient_connect(client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		MQTTClient_destroy(&client);
		return rc;
	}

	if (!quietMode)
		printf("Connected to %s\n",brokerUrl);

	// Subscribe to the topic
	if (!quietMode)
		printf("Subscribing to topic \"%s\" qos %d\n", options.topic, options.qos);

	rc = MQTTClient_subscribe(client, options.topic, options.qos);

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
				// A message has been received
				printf("Topic:\t\t%s\n", receivedTopicName);
				printf("Message:\t%.*s\n", message->payloadlen, (char*)message->payload);
				printf("QoS:\t\t%d\n", message->qos);
				MQTTClient_freeMessage(&message);
				MQTTClient_free(receivedTopicName);
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

/**
 * Performs a single publish
 */
int publish(char* brokerUrl, char* clientId, struct Options options)
{
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
	MQTTClient_message mqttMessage = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	int rc;

	// Create the client instance
	rc = MQTTClient_create(&client, brokerUrl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create MQTT Client instance\n");
		return rc;
	}

	// Create the connection options
	conn_opts.keepAliveInterval = 20;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 1;

	conn_opts.ssl = &ssl_opts;
	if (options.serverKey != NULL) conn_opts.ssl->trustStore = options.serverKey;
	if (options.clientKey != NULL) conn_opts.ssl->keyStore = options.clientKey;
	if (options.clientPass != NULL) conn_opts.ssl->privateKeyPassword = options.clientPass;
	conn_opts.ssl->enableServerCertAuth = options.enableServerCertAuth;

	// Connect the client
	rc = MQTTClient_connect(client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		MQTTClient_destroy(&client);
		return rc;
	}

	if (!quietMode)
		printf("Connected to %s\n",brokerUrl);

	// Construct the message to publish
	mqttMessage.payload = options.message;
	mqttMessage.payloadlen = strlen(options.message);
	mqttMessage.qos = options.qos;
	mqttMessage.retained = 0;

	// Publish the message
	if (!quietMode)
		printf("Publishing to topic \"%s\" qos %d\n", options.topic, options.qos);

	rc = MQTTClient_publishMessage(client, options.topic, &mqttMessage, &token);

	if ( rc != MQTTCLIENT_SUCCESS )
		printf("Message not accepted for delivery, return code %d\n", rc);
	else
	{
		// Wait until the message has been delivered to the server
		rc = MQTTClient_waitForCompletion(client, token, 10000L);

		if ( rc != MQTTCLIENT_SUCCESS )
			printf("Failed to deliver message, return code %d\n",rc);
	}

	// Disconnect the client
	MQTTClient_disconnect(client, 0);

	if (!quietMode)
		printf("Disconnected\n");

	// Free the memory used by the client instance
	MQTTClient_destroy(&client);

	return rc;
}
