#pragma once

#include "gdata/t_string.h"

namespace tbgeneral{
struct r_split_path {
	//stringA buff;
	//const char * drive, * ddir ,* fname, * fname_ext, * ext; 
	stringA drive, ddir, fname, fname_ext, ext;
	//r_split_path() { drive=ddir=fname=fname_ext=ext=""; };
};
r_split_path split_path(const stringA& path, int flag = 0xF);
stringA get_file_dir(const stringA& path);
stringA get_file_name(const stringA& path);
bool isBaseFileName(const stringA& path);
inline bool isSlash(wchar_t c) { return c == '\\' || c == '/'; };

stringA join2path(const stringA& p1, const stringA& p2);


}; // namespace tbgeneral {