#include "src/txtlogparser.h"

// public functions
// constructor
txtlogparser::txtlogparser(string srcpath)
{
	_srcpath = srcpath;
	_start_dt = 0;
	_end_dt = 0;
	_database = "";
}

bool txtlogparser::set_filter_param(string start_dt, string end_dt, string database)
{
	my_datetime dt(start_dt);
	_start_dt = dt.time;
	dt.str_to_time(end_dt);
	_end_dt = dt.time;
	_database = database;
	return true;
}

// main loop
// fprintf(stdout,"LINE:%d [debug]:[%s]\n", __LINE__, files[i]);
string* txtlogparser::parse()
{
	// そもそも開けなければ意味ない。
	if (!_srcpath.length()) {
		return new string("401:txtlog file cannot open(1).\nparser abort.\n");
	}

	ifstream ifs(_srcpath.c_str());

	string line,dbname;
	int year,month,day,hour,min,sec;
	unsigned int line_ts=0;
	struct tm _tm;
	time_t timer;
	_tm.tm_year=0;_tm.tm_mon=0;_tm.tm_mday=0;
	_tm.tm_sec=0;_tm.tm_min=0;_tm.tm_hour=0;_tm.tm_wday=0;_tm.tm_isdst=-1;
	int st=0,fn=0;
	while (getline(ifs, line)) {
		if (line.length() < 18) {
			continue;
		}
		year = (line[0] - 0x30)*10 + (line[1] - 0x30);
		month = (line[3] - 0x30)*10 + (line[4] - 0x30);
		day = (line[6] - 0x30)*10 + (line[7] - 0x30);
		hour = (line[9] - 0x30)*10 + (line[10] - 0x30);
		min = (line[12] - 0x30)*10 + (line[13] - 0x30);
		sec = (line[15] - 0x30)*10 + (line[16] - 0x30);
		_tm.tm_year=year+100;_tm.tm_mon=month-1;_tm.tm_mday=day;
		_tm.tm_sec=sec;_tm.tm_min=min;_tm.tm_hour=hour;_tm.tm_wday=0;_tm.tm_isdst=-1;

		timer = mktime(&_tm);
		if (timer == -1) {
			continue;
		}
		localtime(&timer);
		line_ts = (unsigned int)timer;
		
		st = line.find("[");
		fn = line.find("]");
		if (st == string::npos || fn == string::npos) {
			continue;
		}
		if (st+1 == fn) {
			dbname = "";
		} else {
			dbname = line.substr(st+1, fn-st-1);
		}
		if (_start_dt && line_ts < _start_dt)
			continue;
		if (_end_dt && line_ts > _end_dt)
			continue;
		if (_database.length() > 0 && strcmp(dbname.c_str(), _database.c_str()) != 0)
			continue;
		fprintf(stdout,"%s\n", line.c_str());
	}

	return new string();
}

