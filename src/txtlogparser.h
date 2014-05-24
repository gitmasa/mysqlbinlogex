
#ifndef __TXTLOGPARSER__
#define __TXTLOGPARSER__

#include "src/binlogex_common.h"
#include "src/my_datetime.h"
#include "fstream"

class txtlogparser
{
private:
	// valiables
	string _srcpath;
	unsigned int _start_dt;
	unsigned int _end_dt;
	string _database;

public:
	txtlogparser(string srcpath);
	bool set_filter_param(string start_dt, string end_dt, string database);
	string* parse();
};


#endif
