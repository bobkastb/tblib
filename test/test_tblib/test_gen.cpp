#include "gdata/t_string.h"
//#include "gdata/tb_buffer.h"
#include "test_state.h"
#include <functional>

std::function<char(int)> getmembuff_f(){
	auto buff = (char*)  malloc(256);
	for (int i=0;i<100;i++) buff[i]=i;
	return [=](int i){ 
		return buff[i];
	};
}


int test_functions(){
	auto f = getmembuff_f();
	char b[10];
	for (int i=0;i<10;i++) b[i] = f(i);
	return 0;
}


namespace tbgeneral {
	namespace test_ns {

		std::string format_ss(const char* format, ...) {
			//std::string buff;
			char buffer[4 * 1024];
			va_Init(vaparams, format);
			vsnprintf(buffer, sizeof(buffer), vaparams.format, vaparams.vars);
			return buffer;
		};
		std::string format_ssf(const char* format, va_params_t& vaparams) {
			//std::string buff;
			char buffer[4 * 1024];
			//va_Init(vaparams, format);
			vsnprintf(buffer, sizeof(buffer), vaparams.format, vaparams.vars);
			return buffer;
		};
		tTestProcRegistrator::tTestProcRegistrator(	test_proc_type proc , const char* name , eTestProcStatus st ){
			this->proc = proc;
			this->name=name;
			this->state = st;
			gindex = -1;
			RegisterTestProc(this);
		};
		void RegisterTestProc(tTestProcRegistrator * preg){
			tTestCollection::gettests().Register( preg );
		};
		void tTestCollection::Register(tTestProcRegistrator * preg){
			name_type key = preg->name;
			if (getbyname(key)!=0) {
				std::cout << "try repeat registration for: "<< key << "\n";
			}
			preg->gindex = regslist.size();
			this->regslist.push_back(preg);
			this->index_nms[key] = preg;

		};
		tTestCollection::preg_t tTestCollection::getbyname( const std::string & nm){
			auto res = index_nms.find( nm );
			if (res!= index_nms.end() ) return res->second;
			return 0;
		};
		tTestCollection& tTestCollection::gettests(){
			static tTestCollection tests;
			return tests;
		};


	} // namespace test_ns {
};//namespace tbgeneral {
	
