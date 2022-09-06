#include "tb_env.h"
#include "tsys/tbfiles.h"
#include "tsys/sys_mutex.h"
#include <cstdlib> //getenv
#include "gdata/t_format.h"
//#include<stdio.h>




namespace tbgeneral{

//extern wchar_t **_wenviron;

static stringA * storage__get_all_environment=0;
static LocalEnv * copy_system_env=0;
static const wchar_t ** DEBUG_wenviron=0;
static const char ** DEBUG_environ=0;
static const wchar_t ** _get_all_environmentW(){
	return DEBUG_wenviron ? DEBUG_wenviron : crsys::get_all_environment_sysW();
}
static const char ** _get_all_environmentA(){
	return DEBUG_environ ? DEBUG_environ : crsys::get_all_environment_sysA();
}

void debug_set_new_environ( const char ** eA ,  const wchar_t ** eW   ){
	storage__get_all_environment = 0 ;
	copy_system_env=0;
	DEBUG_environ=eA;
	DEBUG_wenviron= eW;
}

void get_all_environment(stringA & res){
	auto storage= storage__get_all_environment;
	//static stringA storage; //TODO: Thread access
	if (storage){ res=*storage; return; }
	storage = new stringA();
	auto allw = _get_all_environmentW();
	if (allw) { char buff[1024]; stringA tstr; tstr.assign_storage(buff);
		for(auto s=allw;s[0];s++) {
			convert( tstr, *s,strlen_s(*s)+1 );
			storage->append(tstr);
		}
	} else {
	auto allc = _get_all_environmentA();
	for(auto s=allc;s[0];s++)
		storage->append(*s,strlen_s(*s)+1);
	};
	res=*storage;
	storage__get_all_environment = storage;
}


int LocalEnv::loadfrom_compact_text(const stringA & txt , char separator ){
	auto s=txt.cbegin(), e=txt.cend() , s0=s;
	stringA nm , val;
	int err=0;
	if (storage.empty()) storage=txt;
	for (;s<e;){ err=1;
		for (s0=s;s<e && *s!='='; s++); 
		if (s>=e) break;
		nm =  txt.slice( s0,s );	s++;
		for (s0=s;s<e && *s!=separator; s++); 
		if (s>=e) break;
		val =  txt.slice( s0,s );s++; 
		this->set( nm , val );
		err=0;
	}
	return err;
};


const LocalEnv * get_programm_env(){
	//static LocalEnv prgenv; // Thread access - максимум неприятностей это дубликаты copy_system_env в памяти
	if (copy_system_env) return copy_system_env;
	auto & prgenv = *(new LocalEnv(0));
	get_all_environment(prgenv.storage);
	prgenv.loadfrom_compact_text( prgenv.storage , 0 );
	copy_system_env = &prgenv;
	return &prgenv;
}

LocalEnv * get_programm_local_env(){
	static LocalEnv locenv(LocalEnv::gtUseGlobal); //TODO: Thread access
	if (!locenv.index.empty()) return &locenv;
	locenv.set("prg",filesystem::get_programm_dir_a());
	locenv.set("prgdir",filesystem::get_programm_dir_a());
	return &locenv;
};

bool getEnvVar( const stringA & name , stringA & res , const LocalEnv * env ){
	if (env) return env->tryget(name , res);
	auto syse = get_programm_env();
	auto loce = get_programm_local_env();
	return syse->find(name,res)  ? true : loce->find(name,res);
}


stringA getEnvVar( const stringA & name ,  const LocalEnv * env ){
	stringA res;
	return getEnvVar(  name ,  res ,env ) ?  res : stringA() ;
};

bool getEnvVar( const stringA & nm , stringA & res  ){
	return getEnvVar(nm,res, (LocalEnv *)0 );
}

stringA getEnvVar( const stringA & nm ){
	return getEnvVar(  nm  , 0 );
}


//stringA getEnvVar( const stringA & nm ){	return getEnvVar( partstringA(nm) );}

/*
bool getEnvVar( const partstringA & nm , stringA & res  ){
	partstringA rr;
	bool br = tbgeneral::getEnvVar( nm , rr );
	if (br) res = rr;
	return br;
};
*/

struct expand_env_ctx{
	using count_replace_t = LocalEnv::count_replace_t;
	const LocalEnv * env;
	int count_noreplace=0;
	int count_vars=0;
	stringA invalidvar;
	count_replace_t expand_environment_string(  const stringA & _str , stringA & res );
};

LocalEnv::count_replace_t expand_env_ctx::expand_environment_string(  const stringA & _str , stringA & res ){
	auto str = _str; // для случая когда &_str == &res и в строке нет замен
	auto s=str.cbegin(), e=str.cend(), s0=s , sr=s ;
	//int count_vars=0 , count_noreplace=0;
	count_vars=0; count_noreplace = 0;
	stringA varrepl;
	auto test_iv = !this->invalidvar.empty();
	while (s<e) {
		for(s0=s;s<e && *s!='%'; s++);
		if (s>=e) break;
		s++;
		for(sr=s;s<e && *s!='%'; s++);
		if (s>=e) break;
		if (count_vars==0) {
			res.resize(0);
			res.reserve(str.size());
		}
		count_vars++; 
		res.append( str.slice(s0,sr-1) );
		auto varname= str.slice(sr,s);
		if (s==sr)  res.push_back('%');
		else if (test_iv && 0==stricmp(varname,this->invalidvar) ) {
			return LocalEnv::eclos_CircularReference;
		} else if (getEnvVar( varname , varrepl , env )) {
			res.append(varrepl );
		} else {
			count_noreplace++;
			res.append(str.slice(sr-1,s+1));
		}
		s++; s0 = s;
	}
	if (!count_vars) res=str;
	else { res.append(str.slice(s0,e));}
	
	return count_vars-count_noreplace;

};


LocalEnv::count_replace_t expand_environment_string( const stringA & str , stringA & res , const LocalEnv * env){
	expand_env_ctx ctx={env,0,0};
	return ctx.expand_environment_string( str,res);
};

stringA expand_environment_string( const stringA & str , const LocalEnv * env ){ //TODO: улучшить!
	stringA res;
	expand_environment_string( str , res , env );
	return res;
}
//stringA expand_environment_string( const stringA & str ){	return expand_environment_string(partstringA(str)); }

/*
	enum { gtUseGlobal=1, gtUseSpecial=2 ,  gtUseAll=0xff };
	std::map<stringA,stringA> index;
	uint useglobal;
*/

static darray<stringA> split_envp( const stringA & str ) {
	const char* s=str.data(), *e=s+str.size(),  *sl,*sr;
	//*start=s,
	darray<stringA> res;
	while (s<e) {
		sl=(const char*)memchr(s,'%',e-s);
		if (!sl) { break; }
		sl++;
		sr=(const char*)memchr(sl,'%',e-sl);
		if (!sr) { break; }
		s=sr+1;
		res.push_back(str.slice(sl, sr));
	}
	return res;
}
static int replacer(const LocalEnv & le , darray<stringA> & sd , const stringA & orgstr, stringA & result ){
	result = orgstr;
	if (sd.size()==0) { return 0; }
	stringA ns;
	const char* s=orgstr.data(), *e=s+orgstr.size() , *lcs=s;
	int cntrepl=0;
	for ( auto x : sd ){
		stringA fv;
		if ( le.tryget( x , fv ) ) {
			ns.append( lcs, x.cbegin()-1);
			ns << fv;
			lcs = x.cend()+1;
			cntrepl++;
		}
	}
	if (lcs==orgstr.data()) { return 0; }
	ns.append( lcs, e );
	result = ns;
	return cntrepl;
}

darray<stringA> LocalEnv::split_env( const stringA & str ){
	return split_envp( str );
}; 

int LocalEnv::expand( const stringA & str , stringA & res ){
	return expand_environment_string(str , res, this );
};
stringA LocalEnv::expand( const stringA & str ){
	return expand_environment_string(str , this );
};
int LocalEnv::set( const t_key & key , const stringA & val ){
	index[key] = val;
	return 0;
};
stringA LocalEnv::get( const t_key & key )const{
	stringA res;
	return  tryget( key , res ) ? res : stringA(); 
};
bool LocalEnv::tryget( const t_key & key , stringA & val ) const { 
	//if (find(key,val)) return true;

	return find(key,val)? true 
		: (useglobal & gtUseSpecial) && get_programm_local_env()->find( key , val ) ? true 
		: (useglobal & gtUseGlobal) && get_programm_env()->find( key , val ) ? true
		:false;

};

bool LocalEnv::find( const t_key & key , stringA & val ) const {
	auto f= index.find(key);
	if (f==index.end()) { return false; }
	val = f->second;
	return true;
};


bool LocalEnv::closure(){
	return closure_ext()==eclos_OK;
}
LocalEnv::eclos_Result LocalEnv::closure_ext(){
	std::map<t_key,int, less_str_icase<stringA> > noexpkeys;
	int no_exist_vars=0 , fullchng;
	while(1){
		fullchng=0;no_exist_vars=0;
		for (auto v=index.begin(); v!=index.end();  v++ )	{
			auto key = v->first;	
			if (noexpkeys.find(key)!=noexpkeys.end()) continue;
			auto data=v->second;
			stringA newdata;
			expand_env_ctx ctx={this,0,0,key};
			auto r= ctx.expand_environment_string(  data , newdata );
			if ( r==eclos_CircularReference )
				{ return eclos_CircularReference;  }
			no_exist_vars += ctx.count_noreplace;
			if (r==0) 
				noexpkeys[key]=1;
			else {
				fullchng += 1;
				index[key] = newdata;
			}
		}
		if (fullchng==0) break;
	}
	return no_exist_vars ? eclos_NoExistVar : eclos_OK;
};


}; // namespace tbgeneral{

//extern char **environ;


