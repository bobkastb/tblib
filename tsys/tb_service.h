#pragma once
#include "tb_basetypes.h"
#include "tsys/systime.h"

#include "gdata/t_string.h"

namespace crsys{

struct r_service_process {
		using stringA = tbgeneral::stringA;
		int argc;	char** argv;
		stringA servicename;
		stringA systemservicename; // для систем в которых имя сервиса отличается от имени exe

        volatile int running; // 

		virtual void onRefresh() { };
		virtual void onStop() { running=0; };
		virtual int mainloop()=0;
		virtual std::string getSTDfiles( int stdi ){ return ""; };

		int callAsService();
		int run_mainloop(){ running=1; return mainloop(); };

		void init( int argcnt ,	 char** args ){ 
			argc=argcnt; argv= args;
			parsecmd();
			};
		r_service_process() { argc=0;argv=0;running=0; }
		protected:
		void parsecmd();
		void redirect_stdio();
};


}; // namespace crsys{
