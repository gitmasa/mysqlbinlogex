
#ifndef __LOGPARSER__
#define __LOGPARSER__

#include "binlogex_common.hpp"
#include "filemanage.hpp"
#include "my_datetime.hpp"

typedef struct{
	unsigned int ts;
	unsigned int type;
	unsigned int masterID;
	unsigned int size;
	unsigned int masterPos;
	unsigned int flags;
} Mysql_Logheader;

class logparser
{
private:
	// valiables
	string _srcpath;
	string _dstdir;
	bool _no_crlf;
	FILE* _srcfp;
	unsigned char* _query_buff;
	filemanage* _fm;
	unsigned int _start_dt;
	unsigned int _end_dt;
	string _database;

	int enable_crc32;
	boost::regex _regDb1,_regDb2,_regDb3;

	// method
	bool _getHeader(unsigned char *buff, Mysql_Logheader* setter);
	bool _getByte(unsigned char* buff, int size);
	bool _skipByte(int size);
	bool _isInDate(unsigned int headerTs);
	void _showQuery(const char* tmpDbName, const char* tmpSql, unsigned int headerTs);

public:
	logparser(string srcpath, string dstdir, bool no_crlf);
	bool set_filter_param(string start_dt, string end_dt, string database);
	string* parse();
	~logparser()
	{
		if (_fm != NULL) {
			delete _fm;
		}
		if (_srcfp != NULL) {
			fclose(_srcfp);
		}
		if (_query_buff != NULL) {
			free(_query_buff);
		}
	}
	bool safeUtf8Str(unsigned char* str, int len, int no_crlf);

};





#endif
