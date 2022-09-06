#include "t_path.h"
#include "t_parse.h"

namespace tbgeneral{

r_split_path split_path(const rc_string<char>& path, int flag) {
	auto p = path.begin() , e= path.end();
	if (p == 0) return r_split_path();
	r_split_path res; //int state=0;


	const char * x, * s_ext = 0, * s_fn = 0, * s_dir = 0;
	for (x = e - 1; x >= p; x--) {
		char c = *x; if (c == '\\') c = '/';
		if (c == '.' && !s_ext) s_ext = x + 1;
		if (c == '/' && !s_fn)  s_fn = x + 1;
		if (c == ':' && !s_dir) s_dir = x + 1;
	};

	if (s_dir) res.drive = path.slice(p, s_dir - 1);
	s_dir = max(s_dir, p);
	if (s_fn) res.ddir = path.slice(p, s_fn - 1);
	else res.ddir = res.drive;
	s_fn = max(s_fn, s_dir);
	if (s_ext) res.ext = path.slice(s_ext, e);
	if (s_fn) res.fname_ext = path.slice(s_fn, e);
	if (s_fn) res.fname = path.slice(s_fn, (s_ext ? s_ext - 1 : e));
	return res;
};



template<class TChar> rc_string<TChar> get_file_name_s(const rc_string<TChar>& path) {
	if (path.empty()) return rc_string<TChar>();
	auto x = path.cbegin(), e = path.cend() , ee=e;
	for (e--; x <= e; e--) if ((*e == '/') || (*e == '\\')) return path.slice(e + 1, ee);
	return path;
}
template<class TChar> rc_string<TChar> get_file_dir_s(const rc_string<TChar>& path) {
	if (path.empty()) return rc_string<TChar>();
	auto x = path.cdata() , e = path.end();
	for (e--; x<e && ((*e == '/') || (*e == '\\')); e--);
	for (; x<e && ((*e != '/') && (*e != '\\')); e--);
	return path.slice(x, e);
}

stringA get_file_dir(const stringA& path) {
	return get_file_dir_s(path);
};
stringA get_file_name(const stringA& path) {
	return get_file_name_s(path);
};
stringA join2path(const stringA& p1, const stringA& p2) {
	if (p1.empty()) return p2;
	if (p2.empty()) return p1;
	return p1 + "/" + p2;
}

bool isBaseFileName(const stringA& fn) {
	return tbgeneral::find_str(fn, "/").empty() && tbgeneral::find_str(fn, "\\").empty();
}


namespace nsparse{

bool char_inSYM(char c) { 
	return (c>='0' && c<='9')||((byte)c>128)||(c>='a' && c<='z')||(c>='A' && c<='Z')||(c=='_'); 
}
bool char_inSYMe(char c) { 
	return (c>='0' && c<='9')||((byte)c>128)||(c>='a' && c<='z')||(c>='A' && c<='Z')||(c=='_')||(c=='-');
}
const char * goto_endofline(const char * d ,const char *  e) {
	for (;*d!=0 && *d!=char_LE && *d!=char_LE;d++); 
	return d;
};
const char * goto_char(const char * d, const char * e ,char c){
	for (;d<e && *d!=c;d++); 
	return d;
};
const char * goto_nextline(const char * d, const char * e) {
	for (;d<e && *d!=char_LE;d++); 
	for (;d<e && (*d==char_LE || *d==char_LF);d++); 
	return d;
};

t_linesinfo t_linesinfo::calc( const char* startpos , const char* from ){
	t_linesinfo res={0,0};
	auto s= startpos; auto linest=s;
	for (; s<from; s++ ) {	if (*s=='\n') { res.line++; linest=s+1; } }
	for (auto l=linest; l<s;  l++ ) {	res.col += (*l=='\t')? 4:1; }
	return res;
}


bool has_escape_char( const stringA & d ){
	auto r= memchr(d.data() , '\\', d.size()); return r!=0; 
}


stringA unescape_std_str( const stringA& d ){
	//if (!has_escape_char(d)) { return d; }
	stringA res; 
	auto e= d.cend() , s=d.cbegin() , bs=s;
	for ( ;s<e;s++) { 
		for ( ;s<e && *s!='\\'; s++);
		if (s==e) break;
		if(res.empty()) res.reserve(d.size());
		res.append( bs , s-bs );
		s++;
		switch (s[0]) {
		case 't': res<<'\x09';break;
		case 'r': res<<'\r';break;
		case 'n': res<<'\n';break;
		case 'f': res<<'\x0C';break;
		case 'a': res<<'\x07';break;
		case 'b': res<<'\x08';break;
		default : res << s[0];
		}
		bs=s+1;
	}
	if(res.empty()) return d;
	res.append( bs , s-bs ); 
	return res;
};


}

}; //namespace tbgeneral {