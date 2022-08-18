#pragma once

#include "conf/json_parser.h"
#include <gdata/tb_map.h>
#include <gdata/tb_serialize.h>
#include "gdata/pack/ArithmeticCoding_ngramm.h"


namespace tbgeneral{


struct t_ac_text_learn{
	using g_char = uint16;
	using l_char = int;
	using c_alpha = int;
	//using g_char = uint16;

	t_error_info error;

	//-------------- input 
	//t_ac_compress_setting * compress;
	stringA source_filepath  // путь к папке/файлу для обучения
		,result_filepath;	// путь к файлу резуьтата
		
	// -- onsave
	darray<wchar> code2char; // g_char -> wchar_t
	//-------------- work 
	darray<g_char> char2code; // TODO:! нужно убрать это!
	darray<c_alpha> g_char2alpha;  // TODO:! нужно убрать это!
	char specchars[256]; // множество символов после которых игнорируентся символы пробела ' ' 
	struct t_alpha {
		stringA  name;
		stringW  chars;

		int index;
		int maxNgram; 
		bool OneSymbolPerWord;
		int alphasize , // количество символов алфавита
			tr_size; // полное количество символов алфавита . alphasize+ символы переходы, + terminal
		darray<int> gchar2lchar; // g_char -> l_char
		int l_first_char, // индекс первого символа алфавита
			l_first_alpha; // индекс первого символа-перехода
		darray<wchar> lcode2wchar;
			
		darray<int> maps[10];
		int maps_size[10];
		uint sizeof_stk;
		int cnt_cotinues;
		int getid() { return index+1;}
		stringW getchars() { 	
			//return parray<wchar>( lcode2wchar , l_first_char, alphasize);
			return stringW(&lcode2wchar[l_first_char] , alphasize );
		}

		// stats
	};
	darray<t_alpha> ar_alpha;

	stringA ac_params; // сохранение параметров AC
	double rate_endchar;

	// --- claculate params
	uint64 totalchars,badchars;
	uint handled_files, errored_files;
	int c_alphabets;



	int make( const stringA & _path =0 , const stringA & outfile=0 );
	int make_from_data( const stringA & data ) { return fordata(data); };
	int saveresult(const stringA & outfile=0 , int saveopt=-1 );
	int saveresult_buff( stringA & outbuff , int saveopt=-1);
	int InitFrom( conf_struct_ref jr );
	int InitFrom( const stringA & jsontext   );

	t_ac_text_learn() { ZEROMEM(*this); }
	int save_srcsetting(conf_struct_ref jr);
	int init_ac( c_arithmetic_coding_ngramm  & ac );

	private:
	int init0();
	int init1();
	//int init();
	//int finish();
	int fordata( const stringA & data );
	int forfile( const stringA & fnm );
	int rndfiles( const stringA & dirname );
	void printworkpercent( size_t hsize , size_t fsize ){};
	//struct r_alpha { int num; stringA key; stringW list; } ;

};

}; //namespace tbgeneral{