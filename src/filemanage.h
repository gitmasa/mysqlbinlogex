#ifndef __FILEMANAGE__
#define __FILEMANAGE__

#include "src/binlogex_common.h"

class filemanage
{
private:
	map<string, FILE*> _files;
	string _fname;
	string _savedir;
public:
	filemanage(string base_dir, string base_fname);
	FILE* getFile(char* dbname);
	FILE* getFile(string dbname);
	~filemanage(){
		map<string, FILE*>::iterator it = _files.begin();
		while( it != _files.end() ) {
			fclose((*it).second);
			++it;
		}
	}
};


#endif
