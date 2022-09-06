#pragma once

//#include "tb_parray.h"
#include "tb_map.h"
#include <map>
#include "gdata/t_string.h"

namespace crsys{

const char** get_all_environment_sysA();
const wchar_t** get_all_environment_sysW();

};
namespace tbgeneral{





struct LocalEnv{
	
	using count_replace_t = int;
	enum { gtUseGlobal=1, gtUseSpecial=2 ,  gtUseAll=0xff };
	enum eclos_Result { eclos_OK ,  eclos_CircularReference=-10 , eclos_NoExistVar = 1 };
	typedef stringA t_key;
	using map_type=std::map<t_key,stringA, less_str_icase<stringA> >;
	map_type index;
	uint useglobal;
	stringA storage; // еспользуется если строки окружения задаются одной большой областью , например системное окружение
	
	stringA expand( const stringA & str );
	int expand( const stringA & str , stringA & res );
	static darray<stringA> split_env( const stringA & str ); 
	int set( const t_key & key , const stringA & val );
	stringA get( const t_key & key ) const;
	bool tryget( const t_key & key , stringA & val ) const;
	bool find( const t_key & key , stringA & val ) const;
	bool closure();
	int loadfrom_compact_text(const stringA & txt , char separator=0 );
	void clear(){ index.clear();storage.clear(); }
	eclos_Result closure_ext();
	LocalEnv() {useglobal=gtUseAll;}
	LocalEnv(uint options) {useglobal=options;}
};

const LocalEnv * get_programm_env();
LocalEnv * get_programm_local_env();
void get_all_environment(stringA & res);

bool getEnvVar( const stringA & nm , stringA & res  );
stringA getEnvVar( const stringA & nm );
bool getEnvVar( const stringA & name , stringA & res , const LocalEnv * env );
stringA getEnvVar( const stringA & name ,  const LocalEnv * env );
stringA expand_environment_string( const stringA & str , const LocalEnv * env=0);
LocalEnv::count_replace_t expand_environment_string( const stringA & str , stringA & res , const LocalEnv * env=0);



};