/**
 * mysqltxtlog
 * mysqlbinlogexで排出されたログを見るコマンド。
 * 
 * copyright(c) TAP.co.ltd 2014
 *
 */
#include "src/mysqltxtlog.h"


/************************************************************************************************************************
 *
 * アプリケーションエントリポイント
 *
 ************************************************************************************************************************/

int main(int argc, char* argv[])
{
	////////////////////////////////////////////////////////
	// argの処理(起動引数の処理)
	// この工程で、画像ファイル存在等の確認を行う。
	////////////////////////////////////////////////////////
	string in_filepath="";
	string start_dt="", end_dt="";
	string database="";
	int result;
	while ((result = getopt(argc,argv,"i:s:e:d:h")) != -1) {
		switch (result) {
		case 'i':
			in_filepath = optarg;
			break;
		case 's':
			start_dt = optarg;
			break;
		case 'e':
			end_dt = optarg;
			break;
		case 'd':
			database = optarg;
			break;
		case 'h':
		case '?':
			print_usage();
			return 1;
			break;
		}
	}
	if (in_filepath.length() < 1) {
		fprintf(stdout,"400: input filepath is not set.\n");
		return 1;
	}
	txtlogparser parser(in_filepath);
	parser.set_filter_param(start_dt, end_dt, database);
	string* err = parser.parse();
	if (err->length()) {
		fprintf(stdout,"%s", err->c_str());
		return 1;
	}
	delete err;
	return 0;
}


/**
 * 簡易ヘルプ表示
 *
 */
void print_usage(void) {
	fprintf(stdout, "mysqltxtlog copyright(c) Tap.ltd.co 2014\nUsage: mysqltxtlog [opts]\n");
	fprintf(stdout, "\tVersion:%s\n\n", PACKAGE_VERSION);
	fprintf(stdout, "  *** notice ***\n\t");
	fprintf(stdout, "This command is parse for text_sql_log created by mysqlbinlogex.\n\n");
	fprintf(stdout, "\t-i : set input text_sql_log file path(*required entry)\n");
	fprintf(stdout, "\t-s : set start datetime filter.(ex. 'YY/MM/DD HH:ii:ss','YYYY/MM/DD HH:ii:ss'. 'HH:ii:ss' is optional)\n");
	fprintf(stdout, "\t-e : set end datetime filter.(ex. 'YY/MM/DD HH:ii:ss','YYYY/MM/DD HH:ii:ss'. 'HH:ii:ss' is optional)\n");
	fprintf(stdout, "\t-d : set database filter.(ex. -d database-name)\n");
	fprintf(stdout, "\t-h : Display this help and exit.\n");
}


