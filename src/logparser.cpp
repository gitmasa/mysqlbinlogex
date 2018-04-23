#include "logparser.hpp"

using smatch = boost::match_results<const char*>;

// private functions
bool logparser::_getHeader(unsigned char *buff, Mysql_Logheader* setter)
{
	//ヘッダは、| ts(4byte) | type(1byte) | masterID(4byte) | size(4byte) | master_pos(4byte) | flags(2byte) | の19byte構成。
	setter->ts = uint4korr(buff);
	setter->type = (unsigned int)buff[4];
	setter->masterID = uint4korr(buff+5);
	setter->size = uint4korr(buff+9);
	setter->masterPos = uint4korr(buff+13);
	setter->flags = uint2korr(buff+17);
	return true;
}


bool logparser::_getByte(unsigned char* buff, int size)
{
	if (size < 0) {
		return false;
	}
	int ch=0;
	int i=0;
	for (i=0;i<size;i++) {
		ch = fgetc(_srcfp);
		if (ch == EOF) {
			return false;
		}
		buff[i] = (unsigned char)ch;
	}
	return true;
}


bool logparser::_skipByte(int size)
{
	if (size < 0) {
		return false;
	}
	int ch=0;
	int i=0;
	for (i=0;i<size;i++) {
		ch = fgetc(_srcfp);
		if (ch == EOF) {
			return false;
		}
	}
	return true;
}


// public functions
// constructor
logparser::logparser(string srcpath, string dstdir, bool no_crlf)
{
	_srcfp=NULL;
	_query_buff=NULL;
	_srcpath = srcpath;
	_dstdir = dstdir;
	_no_crlf = no_crlf;
	_fm = NULL;
	_start_dt = 0;
	_end_dt = 0;
	_database = "";

    try {
        _regDb1 = boost::regex("^INSERT\\s+INTO\\s+([^\\.\\s]+)\\.[^\\.\\s]+\\s+.+$", boost::regex::icase|boost::regex::extended);
        _regDb2 = boost::regex("^UPDATE\\s+([^\\.\\s]+)\\.[^\\.\\s]+\\s+.+$", boost::regex::icase|boost::regex::extended);
        _regDb3 = boost::regex("^DELETE\\s+FROM\\s+([^\\.\\s]+)\\.[^\\.\\s]+\\s+.+$", boost::regex::icase|boost::regex::extended);
    } catch (boost::regex_error& e) {
        std::cout << e.what()  << "[ERR0_3]" << std::endl;
    }
}

bool logparser::set_filter_param(string start_dt, string end_dt, string database)
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
string* logparser::parse()
{
	// そもそも開けなければ意味ない。
	if (!_srcpath.length()) {
		return new string("401_1:binlog file cannot open(1).\nparser abort.\n");
	}
	_srcfp = fopen(_srcpath.c_str(), "r");
	if( _srcfp == NULL ){
		return new string("401_2:binlog file cannot open.\nparser abort.\n");
	}

	// output directoryは、必ずディレクトリ。
	if (_dstdir.length() > 0) {
		struct stat sb;
		if (_dstdir[_dstdir.length()-2] != '/')
			_dstdir += '/';
		if (stat(_dstdir.c_str(), &sb) == -1) {
			return new string("401_3: output directory path is bad. Directory does not exists!\n");
		}
		// 保存ファイル名を作成。
		string cpy = _srcpath;
		unsigned long loc = cpy.find_last_of('/');
		if (loc != string::npos) {
			cpy = cpy.substr(loc+1);
		}
		cpy += ".txt";
		_fm = new filemanage(_dstdir, cpy);
	}
	unsigned char buff[19];
	unsigned char dbname[256];
	Mysql_Logheader header;
	int payloadLength=0, remainLength=0, schemaLength=0, statusLength=0;

	// 走査開始 ==========================================================
	//はじめの4byteは、必ず(fe 62 69 6e)。これでなければエラー。
	if (!_getByte(buff, 4))
		return new string("410_1:binlog file is invalid mysql-binlog.(too small)\nparser abort.\n");
	if (buff[0] != 0xfe || buff[1] != 0x62 || buff[2] != 0x69 || buff[3] != 0x6e)
		return new string("410_2:binlog file is invalid mysql-binlog.\nparser abort.\n");
	//1回目のクエリは、バージョン取得できる「type=0f(FORMAT_DESCRIPTION_EVENT)」のレコード。バージョン取得します。
	if (!_getByte(buff, 19) || !_getHeader(buff, &header))
		return new string("410_3:binlog file cannnot read Header.\nparser abort.\n");
	// check binlog-version
	if (!_getByte(buff, 2))
		return new string("410_4:binlog file not contain binlog-version info.\nparser abort.\n");
	if (uint2korr(buff) != 4)
		return new string("410_5:binlog file is not binlog-version4( >= MySQL5.0).\nparser abort.\n");

	// mysql5.6.1以降は後ろに4byteのチェックサムが付きます。
	if (!_getByte(buff, 6))
		return new string("410_6:binlog file not contain version info.\nparser abort.\n");

	int ver_major = (int)buff[0] - 0x30;
	int ver_minor = (int)buff[2] - 0x30;
	int ver_release = ((int)buff[4] - 0x30) * 10 + ((int)buff[5] - 0x30);
	enable_crc32 = 0;
	if (ver_major > 5) {
		enable_crc32 = 1;
	} else if (ver_major == 5) {
		if (ver_minor > 6) {
			enable_crc32 = 1;
		} else if (ver_minor == 6) {
			if (ver_release >= 1) {
				enable_crc32 = 1;
			}
		}
	}
	// skip mysql-server version & create_ts
	if (!_skipByte(50-6+4))
		return new string("410_7:binlog file Header invalid data size(no Data).\nparser abort.\n");
	// check event_handler_length
	if (!_getByte(buff, 1) || buff[0] != 19)
		return new string("410_8:binlog file is bad event_handler_length.\nparser abort.\n");
    payloadLength = header.size - 19;
	remainLength = payloadLength - 2 - 50 - 4 - 1;
	if (enable_crc32) {
		if (!_skipByte(remainLength - 5))
			return new string("410_9:binlog file Header invalid data size(error format FORMAT_DESCRIPTION_EVENT).\nparser abort.\n");
		// check event_handler_length
		if (!_getByte(buff, 1))
			return new string("410_10:binlog file is bad event_handler_length.\nparser abort.\n");
		if (buff[0] != 1) { // force off CRC32 checksum
			enable_crc32 = 0;
		}
		if (!_skipByte(4)) // 最後の4byteは強制crc32
			return new string("410_11:binlog file Header invalid data size(error format FORMAT_DESCRIPTION_EVENT).\nparser abort.\n");
	} else {
		if (!_skipByte(remainLength))
			return new string("410_12:binlog file Header invalid data size(error format FORMAT_DESCRIPTION_EVENT).\nparser abort.\n");
	}
	// はじめのクエリ走査は終了。
    payloadLength = 0;
    remainLength = 0;


	// メインループ。
	// 2回目以降のクエリは、「type=02 && Flags=00 00」のレコードのみを表示させます。
	while (1) {
		if (!_getByte(buff, 19) || !_getHeader(buff, &header))
			break; // 完了。
        payloadLength = header.size - 19;

/*
//debugging code:
    	fprintf(stdout,"=========DEBUG:type[0x%x(%d)][0x%x(%d)][%d]=========\n", header.type, header.type, header.flags, header.flags, header.size);
        fprintf(stdout,"=========DEBUG:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x=========\n"
				, buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6], buff[7], buff[8], buff[9], buff[10]
				, buff[11], buff[12], buff[13], buff[14], buff[15], buff[16], buff[17], buff[18]);
*/
		if (!_isInDate(header.ts)) {
			if (!_skipByte(payloadLength)) // ステータスデータ長だけ、不要なので飛ばす。
				return new string("420_1:binlog file data invalid(data_length err.1).\nparser abort.\n");
			continue;
		}


		if (header.type == 2 && header.flags == 0) {
            remainLength = payloadLength;
			// 通常のクエリログ (https://dev.mysql.com/doc/internals/en/query-event.html)
			if (!_skipByte(8)) // データの取得。頭の11byteは、| thread_id(4byte) | exec_time_sec(4byte) | schema length(1byte) | error_code(2byte) |なので、不要。
				return new string("421_1:binlog file data Header invalid(post head err).\nparser abort.\n");
            remainLength -= 8;
            if (!_getByte(buff, 1)) // 次の1byteはschema length。
                return new string("421_2:binlog file data Header invalid(post head schma_length err).\nparser abort.\n");
            remainLength -= 1;
            schemaLength = (int)buff[0];
            schemaLength+=1;
            if (!_skipByte(2)) // データの取得。頭の11byteは、| thread_id(4byte) | exec_time_sec(4byte) | schema length(1byte) | error_code(2byte) |なので、不要。
                return new string("421_3:binlog file data Header invalid(post head error_code err).\nparser abort.\n");
            remainLength -= 2;
			if (!_getByte(buff, 2)) // 次の2byteは、ステータスデータ長。
				return new string("421_4:binlog file data Header invalid(post head status_length err).\nparser abort.\n");
            remainLength -= 2;
			statusLength = uint2korr(buff);
			if (!_skipByte(statusLength)) // ステータスデータ長だけ、不要なので飛ばす。
				return new string("421_5:binlog file data Header invalid(post head status_data err).\nparser abort.\n");
            remainLength -= statusLength;

            if (!_getByte(dbname, schemaLength))
                return new string("421_6:binlog file data Header invalid(schema_name err).\nparser abort.\n");
            remainLength -= schemaLength;
            dbname[schemaLength] = 0; // 終端

			if (_query_buff != NULL) {
				free(_query_buff);
				_query_buff = NULL;
			}
			_query_buff = (unsigned char*)malloc(sizeof(unsigned char) * (remainLength+1));
			if (!_getByte(_query_buff, remainLength))
				return new string("421_7:binlog file data invalid(data_length err).\nparser abort.\n");
			if (enable_crc32) {
				_query_buff[remainLength-4] = 0; //crc32付きである場合は、data_len-4のバイトを\0にして、終端させる。
				logparser::safeUtf8Str(_query_buff, remainLength - 5, _no_crlf); // 文字列がバイナリコードを含んでいる場合はここで回して'?'に置き換える。
			} else {
				_query_buff[remainLength] = 0; //crc32無しの場合は、data_lenバイトを\0にして、終端させる。
				logparser::safeUtf8Str(_query_buff, remainLength, _no_crlf); // 文字列がバイナリコードを含んでいる場合はここで回して'?'に置き換える。
			}
			_showQuery((char*)dbname, (char*)_query_buff, header.ts);
			free(_query_buff);
			_query_buff = NULL;
            dbname[0] = 0;
            continue;

		} else if (header.type == 29 && header.flags == 128) {
			// RowBaseレプリケーションのクエリログ(参照用途：コメント扱い)
            remainLength = payloadLength;
			if (!_skipByte(1)) // 最初の1Byteは不明。
				return new string("422_1:binlog file data Header invalid(Row Query Comment post head err).\nparser abort.\n");
			remainLength -= 1;
            if (_query_buff != NULL) {
				free(_query_buff);
				_query_buff = NULL;
			}
			_query_buff = (unsigned char*)malloc(sizeof(unsigned char) * (remainLength+1));
			if (!_getByte(_query_buff, remainLength))
				return new string("422_2:binlog file data invalid(Row Query Comment Log data_length err).\nparser abort.\n");

			_query_buff[remainLength] = 0; //crc32無しの場合は、data_lenバイトを\0にして、終端させる。
			logparser::safeUtf8Str(_query_buff, remainLength - 1, _no_crlf); // 文字列がバイナリコードを含んでいる場合はここで回して'?'に置き換える。
            continue;

		} else if (header.type == 19 && header.flags == 0) {
            remainLength = payloadLength;
			// TABLE_MAP_EVENT(https://dev.mysql.com/doc/internals/en/table-map-event.html)
			if (!_skipByte(8)) // 最初の8Byteは[tableId(6),flags(2)]。
				return new string("491_1:binlog file data Header invalid(Row Query TABLE_MAP_EVENT post head err).\nparser abort.\n");
            remainLength -= 8;
			if (!_getByte(buff, 1)) // schema_length
				return new string("491_2:binlog file data Header invalid(Row Query TABLE_MAP_EVENT post head err).\nparser abort.\n");
            schemaLength = buff[0];
            remainLength -= 1;
			if (!_getByte(dbname, schemaLength))
				return new string("491_3:binlog file data Header invalid(Row Query TABLE_MAP_EVENT post head err).\nparser abort.\n");
            remainLength -= schemaLength;
			dbname[schemaLength] = 0; // \0にして、終端させる。
			if (!_skipByte(remainLength))
				return new string("491_4:binlog file Header invalid data size(no Data).\nparser abort.\n");

			if (_query_buff == NULL) {
				continue;
			}
			_showQuery((char*)dbname, (char*)_query_buff, header.ts);
            free(_query_buff);
            _query_buff = NULL;
            dbname[0] = 0;
            continue;
		}
        // その他スキップする分。
        if (!_skipByte(payloadLength))
            return new string("429:binlog file Header invalid data size(no Data).\nparser abort.\n");
	}
    if (_query_buff != NULL) {
        free(_query_buff);
        _query_buff = NULL;
    }
    return new string("");
}


bool logparser::_isInDate(unsigned int headerTs){
	if (_start_dt || _end_dt) {
		if (_start_dt && headerTs < _start_dt) {
			return false;
		}
		if (_end_dt && headerTs > _end_dt) {
			return false;
		}
	}
	return true;

}


void logparser::_showQuery(const char* tmpDbName,const char* tmpSql, unsigned int headerTs){
	char _dbname[256];
	time_t ts_tmp;
	struct tm *res;

    std::string str = std::string(tmpSql);
    boost::smatch results;
    try {
        if (boost::regex_match(str, results, _regDb1)) {
            strncpy(_dbname, results[1].str().c_str(), 255);
        } else if (boost::regex_match(str, results, _regDb2)) {
            strncpy(_dbname, results[1].str().c_str(), 255);
        } else if (boost::regex_match(str, results, _regDb3)) {
            strncpy(_dbname, results[1].str().c_str(), 255);
        } else {
            strncpy(_dbname, tmpDbName, 255);
            _dbname[255] = 0;
        }
    } catch (boost::regex_error& e) {
        strncpy(_dbname, tmpDbName, 255);
        _dbname[255] = 0;
    }

	// dbname filter
	if (_database.length() > 0 && strcmp(_dbname, _database.c_str()) != 0) {
		return;
	}

	ts_tmp = (time_t)headerTs;
	res = localtime(&ts_tmp);
	if (_fm != NULL) {
		FILE *tmpfp = _fm->getFile(_dbname);
		fprintf(tmpfp,"%02d/%02d/%02d %2d:%02d:%02d [%s] ", res->tm_year % 100, res->tm_mon+1, res->tm_mday, res->tm_hour, res->tm_min, res->tm_sec, _dbname);
		fprintf(tmpfp,"%s\n", tmpSql);
	} else {
		fprintf(stdout,"%02d/%02d/%02d %2d:%02d:%02d [%s] ", res->tm_year % 100, res->tm_mon+1, res->tm_mday, res->tm_hour, res->tm_min, res->tm_sec, _dbname);
		fprintf(stdout,"%s\n", tmpSql);
	}


}



bool logparser::safeUtf8Str(unsigned char* str, int len, int no_crlf){
	int i=0;
	while (i<len) {
		if (str[i] <=0x7f) { // 1byte文字
			// 制御文字：0x00-0x1f,0x7f
			// (0x09:水平タブ、0x0a:LF、0x0D:CR、0x20:半角スペース)は通す。
			if (str[i] <= 0x1f || str[i] == 0x7f) {
				if (str[i] != 0x09 && str[i] != 0x0a && str[i] != 0x0d && str[i] != 0x20){
					str[i] = '?';
				}
				if (no_crlf && (str[i] == 0x0a || str[i] == 0x0d)) {
					str[i] = ' '; //no_crlfの場合は、cr/lfをスペースに置き換え。
				}
			}
			i++;
		} else if (str[i] >= 0xc0 && str[i] <= 0xdf) { // 2byte文字
			if (i+1 >= len) {
				str[i] = '?';break;
			}
			// utf8では、0xc280以降がmysqlに登録できる。
			// また、2バイト目は、必ず0x80-0xbfまでと決まっている。（utf8の多バイト文字の2バイト目以降は必ず0x80-0xbf）
			// 先頭が0xc2未満は、mysqlが認めない。
			// 現在わかっているものは、0xc280-0xc2bfが2バイト制御文字。(2012/12)
			if (str[i+1] < 0x80 || str[i+1] > 0xbf) {
				str[i] = '?';str[i+1] = '?';
			} else if (str[i] < 0xc2 || (str[i] == 0xc2 && (str[i+1] >=0x80 && str[i+1] <= 0xb2))) {
				str[i] = '?';
				str[i+1] = '?';
			}
			i+=2;
		} else if (str[i] >= 0xe0 && str[i] <= 0xef) { // 3byte文字
			// 3バイト文字としての整合性を確認。
			// utf8では、0xe0a080以降がmysqlに登録できる。
			// また、2バイト目/3バイト目は、必ず0x80-0xbfまでと決まっている。（utf8の多バイト文字の2バイト目以降は必ず0x80-0xbf）
			// 現在わかっているものは、次の3バイト制御文字。(2012/12)
			// 0xe2808c-0xe2808f、0xe280a8-0xe280ae、0xe281a0、0xe281aa-0xe281af
			// この中に有名なe2808e,e2808f,e280aa,e280ab,e280ac,e280ad,e280aeがある。(表示方向制御文字)
			if (i+1 >= len) {
				str[i] = '?';break;
			}
			if (i+2 >= len) {
				str[i] = '?';str[i+1] = '?';break;
			}
			if (str[i+1] < 0x80 || str[i+1] > 0xbf || str[i+2] < 0x80 || str[i+2] > 0xbf) {
				str[i] = '?';str[i+1] = '?';str[i+2] = '?';
			} else if (str[i] == 0xe0 && str[i+1] <= 0xa0) {
				if (str[i+1] < 0xa0 || (str[i+1] == 0xa0 && str[i+1] < 0x80)) {
					str[i] = '?';str[i+1] = '?';str[i+2] = '?';
				}
			} else if (str[i] == 0xe2) {
				if (str[i+1] == 0x80 && ((str[i+2] >= 0x8c && str[i+2] <= 0x8f) || (str[i+2] >= 0xa8 && str[i+2] <= 0xae))) {
					str[i] = '?';str[i+1] = '?';str[i+2] = '?';
				}
				if (str[i+1] == 0x81 && (str[i+2] == 0xa0 || (str[i+2] >= 0xaa && str[i+2] <= 0xaf))) {
					str[i] = '?';str[i+1] = '?';str[i+2] = '?';
				}
			}
			i+=3;
		} else if (str[i] >= 0xf0 && str[i] < 0xf8) { // 4byte文字
			// utf8mb4でしか使用できない。基本の多バイト文字検証だけ行う。
			if (i+1 >= len) {
				str[i] = '?';break;
			}
			if (i+2 >= len) {
				str[i] = '?';str[i+1] = '?';break;
			}
			if (i+3 >= len) {
				str[i] = '?';str[i+1] = '?';str[i+2] = '?';break;
			}
			if (str[i+1] < 0x80 || str[i+1] > 0xbf || str[i+2] < 0x80 || str[i+2] > 0xbf || str[i+3] < 0x80 || str[i+3] > 0xbf) {
				str[i] = '?';str[i+1] = '?';str[i+2] = '?';str[i+3] = '?';
			}
			i+=4;
		} else { // 5byte文字とか6byte文字とか。
			str[i] = '?';
			i+=1;
		}
	}
	return true;
}



