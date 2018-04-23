/**
 * mysqlbinlogex
 * mysqlbinlogは使いづらい&結構重いので、使い易いように変更。
 * 
 * copyright(c) TAP.co.ltd 2014
 *
 */

#include "mysqlbinlogex.hpp"


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
	string in_filepath="", out_dirpath="";
	string start_dt="", end_dt="";
	string database="";
	bool no_crlf=false;
	int result;
	while ((result = getopt(argc,argv,"i:o:s:e:d:h")) != -1) {
		switch (result) {
		case 'i':
			in_filepath = optarg;
			break;
		case 'o':
			out_dirpath = optarg;
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
//		case 'n':
//			no_crlf = true;
//			break;
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
	no_crlf=true; // falseにする価値があまり感じられないので、常にtrue
	logparser parser(in_filepath, out_dirpath, no_crlf);
	parser.set_filter_param(start_dt, end_dt, database);
	string* err = parser.parse();
	if (err != NULL && err->length() > 0) {
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
	fprintf(stdout, "mysqlbinlogex copyright(c) Tap.ltd.co 2014\nUsage: mysqlbinlogex [opts]\n");
	fprintf(stdout, "\tVersion:%s\n", PACKAGE_VERSION);
	fprintf(stdout, "\t-i : set input binlog file path(*required entry)\n");
	fprintf(stdout, "\t-o : set output direcroty path (default=null)\n");
	fprintf(stdout, "\t\tIf -o option is null dump to stdout, else create database-name directory and create each dump file.)\n");
	fprintf(stdout, "\t-s : set start datetime filter.(ex. 'YY/MM/DD HH:ii:ss','YYYY/MM/DD HH:ii:ss'. 'HH:ii:ss' is optional)\n");
	fprintf(stdout, "\t-e : set end datetime filter.(ex. 'YY/MM/DD HH:ii:ss','YYYY/MM/DD HH:ii:ss'. 'HH:ii:ss' is optional)\n");
	fprintf(stdout, "\t-d : set database filter.(ex. -d database-name)\n");
	fprintf(stdout, "\t-h : Display this help and exit.\n");
}


