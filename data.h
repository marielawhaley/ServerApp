#pragma once
#include <time.h>
#include <map>
using namespace System;
class data
{
private:
	int curDay;
	int curHour;

public:
	data();
	~data();
	double* getMap(double arr[]);
	void printMap();
	typedef std::map<int, double> temperatureMap;
	temperatureMap tempMap;
	void setArray(double value, int dayHour);

};

