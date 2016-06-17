
#include "AmbientData.h"
#include "MQTTApp.h"
#include "MySQL.h"

using namespace System;
using namespace std;
int main() {

	MQTTApp* mqttSubscriberData = new MQTTApp(1, "AD/#", ""); //subscribe values
}