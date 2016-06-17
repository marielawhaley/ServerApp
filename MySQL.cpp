#include <iostream>
#include "MySQL.h"
#include <ctime>


MySQL::MySQL()
{
	

}

MySQL::~MySQL()
{
}
//QUery values from database
//Param in: 
// cond: ambient condition to be queried
//Param out:
// result: Array of values retrieved
double* MySQL::queryValues(String^ cond, double result[])
{
	String^ constring = L"datasource=localhost;port=3306;username=root;password=Light33!";
	MySqlConnection^ connection = gcnew MySqlConnection(constring);
	double condition[13];
	String^ dateTemp;
	array<String^>^ dateTempDelimiter = gcnew array<String^>(5);
	array<String^>^ dayTime = gcnew array<String^>(5);
	array<String^>^ time = gcnew array<String^>(5);
	String^ day; 
	String^	hour;
	int intDay, intHour, dayHour;
	int entry = 0;
	data dataStore;
	MySqlCommand^ cmdDataBase = gcnew MySqlCommand("SELECT AVG("+ cond+ ") , DATE_FORMAT(date_entered, '%Y-%m-%d %H:00:00') AS Time FROM deskegg_test.data WHERE DATE_FORMAT(date_entered, '%Y-%m-%d %H:00:00') >= DATE_FORMAT(DATE_ADD(NOW(), INTERVAL - 8 HOUR), '%Y-%m-%d %H:00:00') GROUP BY DATE_FORMAT(date_entered, '%Y-%m-%d %H:00:00');", connection);

	MySqlDataReader^ myReader;
	try {
		connection->Open();
		Console::Write("Open\n");
		myReader = cmdDataBase->ExecuteReader();
		while (myReader->Read())
		{
			condition[entry] = myReader->GetFloat(myReader->GetOrdinal("AVG("+cond+")"));
			dateTemp = myReader->GetString(myReader->GetOrdinal("Time")); // 2016-03-20 11:00:00
			Console::Write(condition[entry] + "\n");
			char delimiter = '-';
			dateTempDelimiter = dateTemp->Split(delimiter); //[20 16] [03] [20 11:00:00]

			char delimiter2 = ' '; // separate day from time
			dayTime = dateTempDelimiter[2]->Split(delimiter2); // [20]
			day = dayTime[0];
			//Console::Write("day:" + day + "\n");

			char delimiter3 = ':'; // separate hour from time
			time = dayTime[1]->Split(delimiter3);
			hour = time[0];
			//Console::Write("hour: " + hour + "\n");

			//Convert to integers and set values to data map
			intDay = Convert::ToInt32(day);
			intHour = Convert::ToInt32(hour);
			dayHour = ((intDay * 100) + intHour); // if 26 of march at 11:00am then mapping value is 2611
			dataStore.setArray(condition[entry], dayHour);
			entry++;
		}
		connection->Close();
	}
	catch (Exception^ex) {
		Console::Write("Data Base Error");
	}
	dataStore.printMap();
	dataStore.getMap(result);
	return result;
}

//Insert live data to database
void MySQL::insertValues( String^ temperature, String^ dewPoint, String^ dust, String^ noise, String^ light)
{
	String^ constring = L"datasource=localhost;port=3306;username=root;password=Light33!";
	MySqlConnection^ connection = gcnew MySqlConnection(constring);

	try {
		connection->Open();
		MySqlCommand^command = gcnew MySqlCommand("INSERT INTO deskegg_test.data (temperature, dew_point, dust, noise, ambient_light) values  (" + temperature + ", " + dewPoint + ", " + dust + ", " + noise + ", " + light +");", connection);
		Console::Write("DataBase Opened	\n");
		command->ExecuteNonQuery();
		Console::Write("Saved\n");
	}
	catch (Exception^ex) {
		Console::Write("DataBase Error\n");
	}
	connection->Close();
}

//insert feedback values
void MySQL::insertFeedback(String^ parameter, String^ value)
{
	Console::Write("InsertFeedback");
	String^ constring = L"datasource=localhost;port=3306;username=root;password=Light33!";
	MySqlConnection^ connection = gcnew MySqlConnection(constring);
	String^ cero = "0"; // temp
	String^ one = "1"; //dew point
	String^ two = "2"; // dust
	String^ three = "3"; // noise
	String^ four = "4"; // light
	try {
		connection->Open();
		MySqlCommand^command = gcnew MySqlCommand("INSERT INTO deskegg_test.Feedback (parameter, value) values (" + parameter + ", " + value + ");", connection);
		Console::Write("DataBase Opened	\n");
		command->ExecuteNonQuery();
		Console::Write("Saved\n");
	}
	catch (Exception^ex) {
		Console::Write("DataBase Error\n");
	}
	connection->Close();
}