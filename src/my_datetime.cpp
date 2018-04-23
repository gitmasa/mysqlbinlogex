#include "my_datetime.hpp"

// YY-MM-DD, YYYY-MM-DD, YY-MM-DD HH:MM:SS
my_datetime::my_datetime(string dt_str)
{
	initDate();
	if (dt_str.length() > 0) {
		str_to_time(dt_str);
	}
}

bool my_datetime::str_to_time(string dt_str)
{
	initDate();
	if (dt_str.length() == 0) {
		return true;
	}

	string str = trim(dt_str);
	regex_t pat1,pat2,pat3,pat4;
	size_t nmatch = 7;
	regmatch_t pmatch[nmatch];
	int i=0;

	if (regcomp(&pat1, "^([0-9]{2})[-/]([0-9]{2})[-/]([0-9]{2})$", REG_EXTENDED|REG_NEWLINE)) {
		return false;
	}
	if (regcomp(&pat2, "^([0-9]{4})[-/]([0-9]{2})[-/]([0-9]{2})$", REG_EXTENDED|REG_NEWLINE)) {
		return false;
	}
	if (regcomp(&pat3, "^([0-9]{2})[-/]([0-9]{2})[-/]([0-9]{2})[ _:-]([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$", REG_EXTENDED|REG_NEWLINE)) {
		return false;
	}
	if (regcomp(&pat4, "^([0-9]{4})[-/]([0-9]{2})[-/]([0-9]{2})[ _:-]([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$", REG_EXTENDED|REG_NEWLINE)) {
		return false;
	}
	string tmp="";
	struct tm _tm;
	struct tm* _p_tm;
	_tm.tm_year=0;_tm.tm_mon=0;_tm.tm_mday=0;
	_tm.tm_sec=0;_tm.tm_min=0;_tm.tm_hour=0;_tm.tm_wday=0;_tm.tm_isdst=-1;
	if (regexec(&pat1, str.c_str(), nmatch, pmatch, 0) == 0) {
		i=1;
		_tm.tm_year = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) + 100;
		i=2;
		_tm.tm_mon = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) - 1;
		i=3;
		_tm.tm_mday = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
	} else if (regexec(&pat2, str.c_str(), nmatch, pmatch, 0) == 0) {
		i=1;
		_tm.tm_year = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) - 1900;
		i=2;
		_tm.tm_mon = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) - 1;
		i=3;
		_tm.tm_mday = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
	} else if (regexec(&pat3, str.c_str(), nmatch, pmatch, 0) == 0) {
		i=1;
		_tm.tm_year = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) + 100;
		i=2;
		_tm.tm_mon = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) - 1;
		i=3;
		_tm.tm_mday = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
		i=4;
		_tm.tm_hour = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
		i=5;
		_tm.tm_min = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
		i=6;
		_tm.tm_sec = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
	} else if (regexec(&pat4, str.c_str(), nmatch, pmatch, 0) == 0) {
		i=1;
		_tm.tm_year = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) - 1900;
		i=2;
		_tm.tm_mon = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str()) - 1;
		i=3;
		_tm.tm_mday = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
		i=4;
		_tm.tm_hour = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
		i=5;
		_tm.tm_min = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
		i=6;
		_tm.tm_sec = atoi(str.substr((int)pmatch[i].rm_so, ((int)pmatch[i].rm_eo - (int)pmatch[i].rm_so)).c_str());
	}

	regfree(&pat1);
	regfree(&pat2);
	regfree(&pat3);
	regfree(&pat4);

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

void my_datetime::initDate()
{
	this->time = 0;
	this->month = 0;
	this->day = 0;
	this->year = 0;
	this->hour = 0;
	this->min = 0;
	this->sec = 0;
}

string my_datetime::trim(const string& str, char* trimCharacterList)
{
	string result;
	// 左側からトリムする文字以外が見つかる位置を検索します。
	string::size_type left = str.find_first_not_of(trimCharacterList);
	if (left != string::npos) {
		// 左側からトリムする文字以外が見つかった場合は、同じように右側からも検索します。
		string::size_type right = str.find_last_not_of(trimCharacterList);
		// 戻り値を決定します。ここでは右側から検索しても、トリムする文字以外が必ず存在するので判定不要です。
		result = str.substr(left, right - left + 1);
	}
	return result;
}

