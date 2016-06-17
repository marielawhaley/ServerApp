#include "data.h"
#include <map>

data::data()
{
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);
	curDay = now->tm_mday;
	curHour = now->tm_hour;

	//std::map<int, int> tempMap;
	//temperatureMap =&tempMap;

	int day = curDay;
	int hour = curHour;
	int dayHour = ((day * 100) + hour);

	tempMap.insert(std::pair<int, double>(dayHour, 0));             // version 3
	for (int i = 0; i < 7; i++)
	{
		hour--;
		if (hour < 0)
		{
			hour = 23;
			day = day - 1;
		}
		dayHour = (day * 100) + hour;
		tempMap.insert(std::map<int, double>::value_type(dayHour, 0));             // version 3
	}
	//Console::Write("ceros");
//	printMap();
}
//data::temperatureMap data::tempMap = {
//};

void data::setArray(double value, int dayHour)
{
	std::map<int, double>::iterator it = tempMap.find(dayHour);
	if (it != tempMap.end())
	{
		it->second = value;
	}
}


void data::printMap()
{
	std::map<int, double>::iterator it = tempMap.begin();
	for (it = tempMap.begin(); it != tempMap.end(); it++)
	{
		Console::Write(it->first + " => " + it->second + "\n");
	}
}

double* data::getMap(double arr[])
{
	int i=0;
	 std::map<int, double>::iterator it = tempMap.begin();
	 for (it = tempMap.begin(); it != tempMap.end(); it++)
	 {
		 arr[i] = it->second;
		 i++;
	 }
	 return arr;

}

data::~data()
{
}


