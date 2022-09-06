#pragma once

#include <algorithm>
#include <vector>
#include "tbconf.h"
#include <gdata/tb_map.h>
#include <gdata/t_error.h>
#include <gdata/tb_numbers.h>


namespace tbconf{

template <class tStr> void string_toupper( tStr & s) {	std::transform( s.begin(), s.end(),s.begin(), ::toupper); };	
template <class tStr> void string_tolower( tStr & s) {	std::transform( s.begin(), s.end(),s.begin(), ::tolower); };	

template< typename tchar> struct rInputOptions {
	//typedef typename tStr::value_type tchar;
	typedef std::basic_string<tchar, std::char_traits<tchar>, std::allocator<tchar> >	tStr;
	typedef rInputOptions<tStr> this_type;
	typedef const tchar* ptchar_const;

	struct rOption {  rOption(){ knownOptTag=-1; getscount=0;};
		int knownOptTag , getscount;
		tStr optname , optval;	
	};
	struct rParamFree {  rParamFree() {}; rParamFree(const tchar * pfr) { val=pfr; }
		tStr val;
	};
	std::vector<rOption> opts;
	std::vector<rParamFree> params;
	bool help_options_present;
	bool doprintf;	
	int getscount; 
	uint current_param;
	tStr message;

	rInputOptions( const tchar** argv , int argc) 
			{ current_param=getscount=0;help_options_present=doprintf=false; parsearray( argv , argc); }
	bool getoption( const tchar* optnm , tStr * v ){	
		tStr n(optnm);  
		string_tolower(n); //if (v) *v=tStr();
		for (uint i=0;i<opts.size();i++) if (0==n.compare(opts[i].optname)) 
			{ if (v) *v= opts[i].optval; opts[i].getscount++; if (opts[i].getscount==1) getscount++; return true;};
		return false;
	};
	bool getoption_v(tStr * v,va_list marker) {
		for(const tchar*s=s=va_arg(marker,tchar*); s ; s=va_arg(marker,tchar*))
			if (getoption( s , v )) return true;
		return false;
	}
	bool getoption_v(tbgeneral::stringA * v,va_list marker) { 
		tStr cv; auto r= getoption_v( &cv , marker ); if (r) *v= cv; return r;
	}
	bool getoption(tStr * v,...) { va_list marker;   va_start( marker, v ); return getoption_v(v, marker); };
	bool getoption(tbgeneral::stringA * v,...) { va_list marker;   va_start( marker, v ); return getoption_v(v, marker); };
	bool getoption(bool * v,...) { tStr vs; va_list marker;   va_start( marker, v ); 
		if ( !getoption_v(&vs, marker) ) return false; 
		if (vs.size()) { if (v) *v=true; return true; }
		int x; if (!atoi_t( vs , x )) return false;	
		if (v) *v = x!=0; 
		return true;
	 }
	bool getparam( uint ip , tStr & v ){	
		ip+=1; v=tStr(); if (ip>=params.size()) return false;
		v = params[ip].val; return true;
	};
	bool getparam(  tStr & v ){ bool r= getparam(current_param,v);current_param++; return r;	}
	tStr getUnknownOptions() { tStr m;
		if (getscount==opts.size())	 return m;
		for (uint i=0;i<opts.size();i++) { if (opts[i].getscount!=0) continue;
			if (m.size()) m.push_back(',');
			m.push_back('-');
			m.append( opts[i].optname );
		};
		return m;
	};

	void parsearray( const tchar** argv , int argc){
		for (int i=0;i<argc;i++) { const tchar* s=argv[i]  ; 
			if ((*s=='/')||(*s=='-')) { s++; const tchar* stN=0, *stV=0; int lenn=0, lenv=0; rOption opt;
				if (*s=='?') { help_options_present=true; continue; };
				for (stN=s;*s;s++) if (*s==':' || *s=='=') { lenn=s-stN; stV=s+1; break; }
				if (stV) { opt.optname=tStr(stN , lenn); opt.optval=tStr( stV ); }	
					else { opt.optname=tStr(stN );  }		
				string_tolower(opt.optname);
				opts.push_back( opt );
			} else { params.push_back( s );	}
		};
	}; 
};


}; //namespace tbconf{

namespace tbgeneral{
	struct r_cmdline_options{
		enum  { otag_Unique=1 , otag_Integral=2 , otag_isArray=4 , otag_NeedValue=8 };
		enum { ef_SpecialJson=1 };
		//enum  { InvalidID=-1 };
		typedef int t_internal_id;
		typedef int t_index;
		typedef int t_user_id;
		static const int InvalidID=-1;

		struct r_option {
			darray<stringA> ids;
			t_user_id user_id;
			stringA help;
			stringA default_val;
			stringA s_type;
			uint flags; // otag
			int i_type;
			//bool unique; // встречается только один раз
			t_internal_id internal_id;
			r_option() { internal_id= user_id = InvalidID; flags=0; i_type=0; }
		};

		int set_as_json( const stringA & data , int flags=ef_SpecialJson ); 

		struct re_parseresult{
			r_option * opt;
			stringA value;
			darray<stringA> a_value;
			int i_id;
		}; 
		struct r_parseresult{
			darray<re_parseresult> list;
			t_error_info error;
			std::map<t_internal_id,t_index> idx;
		};
		//r_parseresult parse_all(const partstringA & data );
		t_error_info error;
		r_parseresult lastparse_result;
		int parse_all(int argc , const char** argv );
		stringA printOptions( );
		
		// after parse_all 
		re_parseresult* get(const stringA & optkey); // after parse_all 
		stringA gets(const stringA & optkey){ auto p=get(optkey); return p?p->value:0; }; 
		//bool get(const partstringA & optkey , stringA & v); // after parse_all 
		bool get(const stringA & optkey , re_parseresult* & pr); // after parse_all 
		re_parseresult* get(const t_user_id optid ); // after parse_all 
		bool get(const t_user_id  optid , re_parseresult* & pr); // after parse_all 
		darray<re_parseresult> getlist() { return lastparse_result.list; }
		bool getvalue(const t_user_id optid , stringA& value ); 
		//bool getvalue(const t_user_id optid, stringA& value);

		r_option * getdesc(const stringA & optkey); 
		r_option * getdesc(const t_user_id  optid); 

		//private:
		spmap<t_index> m_opts;
		std::map<stringA,t_index> m_opts_case;
		std::map<t_user_id,t_index> m_opts_id;
		darray<r_option> l_opts;
	};
}; // namespace tbgeneral{

/*
------------------------ Пример исполбзования --------------------------

void printusage() {
const char * usage=
"finder.exe <inputfile> <resultfile> ([-|/]option=value)* \n\
	-f=<str> (|-file) : тоже что <inputfile> - входной файл для анализа	\n\
	-r=<str> (|-result) : тоже что <resultfile> - выходной файл результата	\n\
	-с=<str> (|-сfg) : файл конфигурации \n\
	-i=<str> (|-image) : файл системного образа приложения для анализируемого файла \n\
	-stof=<num> (|-startoffs) : задает начальное смещение для анализа файла \n\
	-e=<bool(1|0)> (|-emul) : включает/выключает использование эмулятора x86 \n\
	-m=<bool(1|0)> (|-map) : включает/выключает предварительное построение карты точек возможного входа \n\
"; 
 wprintf( L"%S" , usage );
};


	rInputOptions<wchar_t>  opts(  (const wchar_t**) argv , argc );
	rInputOptions<wchar_t>::tStr val;
	if (!opts.getoption(&params.ConfigFileName , L"c" , L"cfg" , 0 )) {}; 
	readconfig();

	if (!opts.getoption(&params.InputFileName	,L"f", L"file",0)) {	opts.getparam(params.InputFileName); };
	if (!opts.getoption(&params.OutFileName		,L"r" , L"result",0)) {	opts.getparam(params.OutFileName); };
	if ( opts.getoption(&params.ImageFileName	,L"i" , L"image",0 )) { em.paramsrun.MemImageFN = params.ImageFileName; };
	if ( opts.getoption(&val					,L"startoffs" , L"stof",0 ))  { tbconf::atoi_t( val , params.startoffsetForChecks );	};
	if ( opts.getoption(&params.enable_emulator	,L"e" , L"emul",0 ))  {};
	if (opts.help_options_present) { 	printusage(); return false; }
	val = opts.getUnknownOptions();
	if (val.size()) {
		LOG("Finder.exe:Ошибка параметров:Незнакомые опции- \"%S\" " , val);
		printusage(); return false; 
	}; 


*/