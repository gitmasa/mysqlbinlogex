#include "my_datetime.hpp"

using smatch = boost::match_results<const char*>;


// YY-MM-DD, YYYY-MM-DD, YY-MM-DD HH:MM:SS
my_datetime::my_datetime(string dt_str) {
	initDate();
	if (dt_str.length() > 0) {
		str_to_time(dt_str);
	}
}

bool my_datetime::str_to_time(string dt_str) {
	initDate();
	if (dt_str.length() == 0) {
		return true;
	}

	boost::regex _reg1, _reg2, _reg3, _reg4;
	std::string str = trim(dt_str);
	boost::smatch results;
	_reg1 = boost::regex("^([0-9]{2})[-/]([0-9]{2})[-/]([0-9]{2})$", boost::regex::icase|boost::regex::extended);
	_reg2 = boost::regex("^([0-9]{4})[-/]([0-9]{2})[-/]([0-9]{2})$", boost::regex::icase|boost::regex::extended);
	_reg3 = boost::regex("^([0-9]{2})[-/]([0-9]{2})[-/]([0-9]{2})[ _:-]([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$", boost::regex::icase|boost::regex::extended);
	_reg4 = boost::regex("^([0-9]{4})[-/]([0-9]{2})[-/]([0-9]{2})[ _:-]([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$", boost::regex::icase|boost::regex::extended);

	struct tm _tm;
	struct tm* _p_tm;
	_tm.tm_year=0;_tm.tm_mon=0;_tm.tm_mday=0;
	_tm.tm_sec=0;_tm.tm_min=0;_tm.tm_hour=0;_tm.tm_wday=0;_tm.tm_isdst=-1;

	if (boost::regex_match(str, results, _reg1)) {
		_tm.tm_year = atoi(results[1].str().c_str()) + 100;
		_tm.tm_mon = atoi(results[2].str().c_str()) - 1;
		_tm.tm_mday = atoi(results[3].str().c_str());
	} else if (boost::regex_match(str, results, _reg2)) {
		_tm.tm_year = atoi(results[1].str().c_str()) - 1900;
		_tm.tm_mon = atoi(results[2].str().c_str()) - 1;
		_tm.tm_mday = atoi(results[3].str().c_str());
	} else if (boost::regex_match(str, results, _reg3)) {
		_tm.tm_year = atoi(results[1].str().c_str()) + 100;
		_tm.tm_mon = atoi(results[2].str().c_str()) - 1;
		_tm.tm_mday = atoi(results[3].str().c_str());
		_tm.tm_hour = atoi(results[4].str().c_str());
		_tm.tm_min = atoi(results[5].str().c_str());
		_tm.tm_sec = atoi(results[6].str().c_str());
	} else if (boost::regex_match(str, results, _reg4)) {
		_tm.tm_year = atoi(results[1].str().c_str()) - 1900;
		_tm.tm_mon = atoi(results[2].str().c_str()) - 1;
		_tm.tm_mday = atoi(results[3].str().c_str());
		_tm.tm_hour = atoi(results[4].str().c_str());
		_tm.tm_min = atoi(results[5].str().c_str());
		_tm.tm_sec = atoi(results[6].str().c_str());
	}

	time_t timer = mktime(&_tm);
	if (timer == -1) {
		return false;
	}
	_p_tm = localtime(&timer);
	this->time = (unsigned int)timer;
	this->year = _p_tm->tm_year;
	this->month = _p_tm->tm_mon;
	this->day = _p_tm->tm_mday;
	this->hour = _p_tm->tm_hour;
	this->min = _p_tm->tm_min;
	this->sec = _p_tm->tm_sec;
	return true;
}

void my_datetime::initDate() {
	this->time = 0;
	this->month = 0;
	this->day = 0;
	this->year = 0;
	this->hour = 0;
	this->min = 0;
	this->sec = 0;
}

string my_datetime::trim(const string& str, char* trimCharacterList) {
	string result;
	const char* trimStrs = (trimCharacterList == NULL) ? " \t\v\r\n" : trimCharacterList;
	// 左側からトリムする文字以外が見つかる位置を検索します。
	string::size_type left = str.find_first_not_of(trimStrs);
	if (left != string::npos) {
		// 左側からトリムする文字以外が見つかった場合は、同じように右側からも検索します。
		string::size_type right = str.find_last_not_of(trimStrs);
		// 戻り値を決定します。ここでは右側から検索しても、トリムする文字以外が必ず存在するので判定不要です。
		result = str.substr(left, right - left + 1);
	}
	return result;
}

