#pragma once
#include "data.h"
using namespace System;
//using namespace std;
using namespace MySql::Data::MySqlClient;

class MySQL
{
private:

protected:


public:
	MySQL();
	~MySQL();
	void insertValues(String^ temperature, String^ dewPoint, String^ dust, String^ humidity, String^ light);
	void insertFeedback(String^ parameter, String^ value);
	double* queryValues(String^ cond, double result[]);
};

