#pragma once

#include "gdata/t_string.h"

namespace tbgeneral {
	struct parseline_result {
		stringA buff;
		darray<char*> list;
	};
	parseline_result parsecmdline(const char* cmd);

	namespace nsparse{
		enum {char_LE=10 ,char_LF=13 , char_SPACE=' ' };
		const char * goto_endofline(const char * d ,const char *  e);
		const char * goto_nextline(const char * d, const char * e);
		const char * goto_char(const char * d, const char * e , char c);
		bool char_inSYM(char c); 
		bool char_inSYMe(char c); 

		struct t_linesinfo {
			int line , col; 
			static t_linesinfo calc( const char* startpos , const char* from );
		};
		bool has_escape_char( const stringA & d );
		stringA unescape_std_str( const stringA& d );

	}
};//namespace tbgeneral
