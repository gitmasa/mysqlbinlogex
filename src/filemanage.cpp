#include "filemanage.hpp"

filemanage::filemanage(string base_dir, string base_fname){
	_savedir = base_dir;
	_fname = base_fname;
	_fname += ".txt";
}

FILE* filemanage::getFile(string dbname){
	return getFile(dbname.c_str());
}

FILE* filemanage::getFile(char* dbname){
	string name = dbname;
	if (name.length() == 0) {
		name = "nodbset";
	}
	if (_files.find(name) == _files.end()) {
		string dirname = _savedir + "/" + name;
		string fname = dirname + "/" + _fname;
		// folderの存在を調査。
		struct stat sb;
		if (stat(dirname.c_str(), &sb) == -1) {
			mkdir(dirname.c_str(), 0755);
		}
		FILE* fp = fopen(fname.c_str(), "w");
		if (fp == NULL) {
			return NULL;
		}
		_files[name] = fp;
	}
	return _files[name];
}

