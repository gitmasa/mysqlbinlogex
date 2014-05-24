
#ifndef __MY_DATETIME__
#define __MY_DATETIME__

#include "src/binlogex_common.h"

class my_datetime
{

public:
	unsigned int time;
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;
	my_datetime(string dt_str);
	bool str_to_time(string dt_str);
	void initDate();
	string trim(const string& str, char* trimCharacterList=" \t\v\r\n");
};





#endif
