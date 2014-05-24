#include "src/logparser.h"

// private functions
bool logparser::_getHeader(Mysql_Logheader* setter)
{
	//ヘッダは、| ts(4byte) | type(1byte) | masterID(4byte) | size(4byte) | master_pos(4byte) | flags(2byte) | の19byte構成。
	unsigned char buff[19];
	if (!_getByte(buff, 19)) {
		return false;
	}
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
		return new string("401:binlog file cannot open(1).\nparser abort.\n");
	}
	_srcfp = fopen(_srcpath.c_str(), "r");
	if( _srcfp == NULL ){
		return new string("401:binlog file cannot open.\nparser abort.\n");
	}

	// output directoryは、必ずディレクトリ。
	if (_dstdir.length() > 0) {
		struct stat sb;
		if (_dstdir[_dstdir.length()-2] != '/')
			_dstdir += '/';
		if (stat(_dstdir.c_str(), &sb) == -1) {
			return new string("405: output directory path is bad. Directory does not exists!\n");
		}
		// 保存ファイル名を作成。
		string cpy = _srcpath;
		int loc = cpy.find_last_of('/');
		if (loc != string::npos) {
			cpy = cpy.substr(loc+1);
		}
		cpy += ".txt";
		_fm = new filemanage(_dstdir, cpy);
	}
	unsigned char buff[19];
	int enable_crc32=0;
	Mysql_Logheader header;
	int data_len=0, status_len=0, sql_pos=0;
	struct tm *res;
	time_t ts_tmp;
	
	// 走査開始 ==========================================================
	//はじめの4byteは、必ず(fe 62 69 6e)。これでなければエラー。
	if (!_getByte(buff, 4))
		return new string("450:binlog file is invalid mysql-binlog.(too small)\nparser abort.\n");
	if (buff[0] != 0xfe || buff[1] != 0x62 || buff[2] != 0x69 || buff[3] != 0x6e)
		return new string("451:binlog file is invalid mysql-binlog.\nparser abort.\n");
	//1回目のクエリは、バージョン取得できる「type=0f」のレコード。バージョン取得します。
	if (!_getHeader(&header))
		return new string("452:binlog file cannnot read Header.\nparser abort.\n");
	// mysql5.6.1以降は後ろに4byteのチェックサムが付きます。
	if (!_getByte(buff, 8))
		return new string("453:binlog file not contain version info.\nparser abort.\n");

	int ver_major = (int)buff[2] - 0x30;
	int ver_minor = (int)buff[4] - 0x30;
	int ver_release = ((int)buff[6] - 0x30) * 10 + ((int)buff[7] - 0x30);
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
	data_len = header.size - 19 - 8;
	if (!_skipByte(data_len))
		return new string("454:binlog file Header invalid data size(no Data).\nparser abort.\n");
	// はじめのクエリ走査は終了。

	// メインループ。
	// 2回目以降のクエリは、「type=02 && Flags=00 00」のレコードのみを表示させます。
	while (1) {
		if (!_getHeader(&header))
			break; // 完了。
		if (header.type != 2 || header.flags != 0) {
			data_len = header.size - 19;
			if (!_skipByte(data_len))
				return new string("460:binlog file Header invalid data size(no Data).\nparser abort.\n");
			continue;
		}
		if (!_skipByte(11)) // データの取得。頭の11byteは、| thread_id(4byte) | exec_time_sec(4byte) | error_code(2byte) |なので、不要。
			return new string("461:binlog file data Header invalid(post head err).\nparser abort.\n");
		if (!_getByte(buff, 2)) // 次の2byteは、ステータスデータ長。
			return new string("462:binlog file data Header invalid(post head status_length err).\nparser abort.\n");
		status_len = uint2korr(buff);
		if (!_skipByte(status_len)) // ステータスデータ長だけ、不要なので飛ばす。
			return new string("463:binlog file data Header invalid(post head status_data err).\nparser abort.\n");

		//データを取得。(はじめの\0までは、データベース名称。)
		data_len = header.size - 19 - 11 - 2 - status_len;
		if (_start_dt || _end_dt) {
			if (_start_dt && header.ts < _start_dt) {
				if (!_skipByte(data_len)) // ステータスデータ長だけ、不要なので飛ばす。
					return new string("464:binlog file data invalid(data_length err.1).\nparser abort.\n");
				continue;
			}
			if (_end_dt && header.ts > _end_dt) {
				if (!_skipByte(data_len)) // ステータスデータ長だけ、不要なので飛ばす。
					return new string("464:binlog file data invalid(data_length err.2).\nparser abort.\n");
				continue;
			}
		}

		_query_buff = (unsigned char*)malloc(sizeof(unsigned char) * (data_len+1));
		if (!_getByte(_query_buff, data_len))
			return new string("464:binlog file data invalid(data_length err).\nparser abort.\n");
		// はじめの\0を探す。
		for (sql_pos=0;sql_pos<data_len;sql_pos++) {
			if (_query_buff[sql_pos] == 0)
				break;
		}
		sql_pos++;
		if (enable_crc32) {
			_query_buff[data_len-4] = 0; //crc32付きである場合は、data_len-4のバイトを\0にして、終端させる。
			logparser::safeUtf8Str(&_query_buff[sql_pos], data_len - sql_pos - 4, _no_crlf); // 文字列がバイナリコードを含んでいる場合はここで回して'?'に置き換える。
		} else {
			_query_buff[data_len] = 0; //crc32無しの場合は、data_lenバイトを\0にして、終端させる。
			logparser::safeUtf8Str(&_query_buff[sql_pos], data_len - sql_pos, _no_crlf); // 文字列がバイナリコードを含んでいる場合はここで回して'?'に置き換える。
		}
		if (_database.length() > 0 && strcmp((char*)_query_buff, _database.c_str()) != 0) {
			free(_query_buff);
			_query_buff = NULL;
			continue;
		}
		ts_tmp = (time_t)header.ts;
		res= localtime(&ts_tmp);
		if (_fm != NULL) {
			FILE *tmpfp = _fm->getFile((char*)_query_buff);
			fprintf(tmpfp,"%02d/%02d/%02d %2d:%02d:%02d [%s] ", res->tm_year % 100, res->tm_mon+1, res->tm_mday, res->tm_hour, res->tm_min, res->tm_sec, _query_buff);
			fprintf(tmpfp,"%s\n", &_query_buff[sql_pos]);
		} else {
			fprintf(stdout,"%02d/%02d/%02d %2d:%02d:%02d [%s] ", res->tm_year % 100, res->tm_mon+1, res->tm_mday, res->tm_hour, res->tm_min, res->tm_sec, _query_buff);
			fprintf(stdout,"%s\n", &_query_buff[sql_pos]);
		}
		free(_query_buff);
		_query_buff = NULL;
	}

	return new string();
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



