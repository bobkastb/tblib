#pragma once
#include <string>
#include "tb_basetypes.h"
#include "gdata/tb_basedtools.h"
#include "gdata/t_format.h"
#include "gdata/t_sstream.h"
#include <functional>

//#define LOG log__DO
#define LOG  tbgeneral::log_gen_push
//#define vLOG log__DOv
#define LOG_ERROR tbgeneral::log_err_push
//log__ERR

namespace tbgeneral {
	enum {
		loglev_Always = 0,
		loglev_Warning = 10,
		loglev_Release = 100,
		loglev_Debug = 1000,
		loglev_ExtraDebug = loglev_Debug + 10,
		loglev_Error = int(-1)
	};
}
namespace crsys {
	enum {
		loglev_Always = tbgeneral::loglev_Always,
		loglev_Warning = tbgeneral::loglev_Warning,
		loglev_Release = tbgeneral::loglev_Release,
		loglev_Debug = tbgeneral::loglev_Debug,
		loglev_ExtraDebug = tbgeneral::loglev_ExtraDebug,
		loglev_Error = tbgeneral::loglev_Error
	};
};

/*
void log__ERR(const char* frmt, ...);
void log__DO(const char* frmt, ...);
void log__DO(const wchar_t* frmt, ...);
void log__DO(int tp, const char* frmt, ...);
void log__DOv(int tp, const char* frmt, tbgeneral::va_params_t& vaparams); // va_list marker
*/

namespace tbgeneral {

	// old log procedure - c-style




	struct log_object_t;
	struct log_msg_t;
	struct log_decls_t{
		enum eState { est_Free = 0, est_Begin, est_Process, est_End };
		enum eMsgFlags { emf_InHeapOnly = 1 };
		enum eRedirectorFlags { efrd_NoHeader = 1 };
		enum { cMsgStackBufferSize = 2*1024 , cMaxMessagesCount=50 };
		enum { mode_ALWAYSPRINTF = 1, mode_WideChars = 2, mode_DEFAULT = 0, mode_TIME_HL = 4, mode_NoAppend = 8 
			, mode_NoUseLocalBuff = 0x10 , mode_NoPushTIME = 0x20 , mode_NoCout = 0x40
		};
		typedef void (*f_printfrepeater)(log_msg_t* m, const char* s, size_t cnt);
		using l_printfrepeater =  std::function< void (log_msg_t*, const char* , size_t ) >;
		struct redrector_rec_t { l_printfrepeater f; uint8_t flags; };
		static log_object_t* currentlog;
		static log_object_t  defaultlog;
		static log_object_t* getcurrentlog();

	};
	struct log_msg_t : public log_decls_t {
		eState state = est_Free;
		log_object_t* owner=0;
		int msgtype=0;
		std::ostream* proxy=0;
		limit_orcstringstream<char> ostr;
		uint8_t flags = 0;
		int posPrefix=0 ,  posMsg=0;
	};
	// Если реальный размер данных в  Push будет больше чем msgbuffsize, то в поток будут попадать порции по msgbuffsize
	struct log_object_t : public log_decls_t {
		static int gCurrentLogLevel;

		size_t msgbuffsize= cMsgStackBufferSize;
		std::ostream* os=0;

		darray<redrector_rec_t> lredirectors;
		darray<log_msg_t*> mresources;
		stringA prefix;
		stringA filename;
		int mode= mode_ALWAYSPRINTF | mode_NoPushTIME;
		
	protected:
		int push_bin_data(log_msg_t* msg, const char* data, size_t size);
		virtual log_msg_t* on_begin_msg(int tp);// = 0;
		virtual void on_end_msg(log_msg_t*);// = 0;
		virtual void on_begin_msg(int tp, log_msg_t& m, void* buff, size_t buffsize);
		void begin_msg(int tp, log_msg_t& m);
		void init_msg(log_msg_t& m, void* buff, size_t buffsize);
		void msgflush(log_msg_t* m, const char* data, size_t size);
	public:
		virtual bool checkloglevel(int tp){ return gCurrentLogLevel<=tp;	};// = 0;
		bool isFile() { return os!=0;}
		bool isOpen() { return os!=0 || !lredirectors.empty();	}
		int getloglevel() const { return gCurrentLogLevel; }
		void setloglevel(int ll) { gCurrentLogLevel = ll; }
		void setmode( int m) { mode=m;}

		template<typename... Rest> void push(int tp, const tbgeneral::param_stringA& frmt, const Rest&... rest) {
			if (!checkloglevel(tp)) return;
			if (cMsgStackBufferSize < msgbuffsize || mode & mode_NoUseLocalBuff) {
				auto msg = on_begin_msg(tp);
				tbgeneral::format_to(msg->proxy[0], frmt.param, rest...);
				on_end_msg(msg);
			} else {
				char buffer[cMsgStackBufferSize];
				log_msg_t msg;
				on_begin_msg(tp, msg , buffer , sizeof(buffer) );
				tbgeneral::format_to(msg.proxy[0], frmt.param, rest...);
				on_end_msg(&msg);
			}
		}
		void push_data(int tp, const char* d, size_t sz) { push( tp , "%s" , param_stringA(d,sz) ); }

		public: 
		bool log_open(const stringA& fnm, const stringA & dataprefix , bool asdef = true, uint mode = mode_DEFAULT);
		//int add_redirector(f_printfrepeater f);
		int add_redirector(l_printfrepeater f , uint8_t redirector_flag=0);
		void close();
		void closefile();
		~log_object_t(){ close(); }
		int lockauto(){ return 0; } //TODO:

	};

	struct log_handle : public log_decls_t {
		log_object_t* hndl;
		log_handle() { hndl = 0; };
		~log_handle();

		bool isopen();
		void close();

		bool log_open(const char* lognm, const char* dataprefix = 0, bool asdef = true, uint mode = mode_DEFAULT);
		void setprefix(const param_stringA & prefix);
		bool setasdefault();
		bool setmode(uint mode);
		int addrepeater(l_printfrepeater f, uint8_t redirector_flag=0);
		int getLogLevel();
		void setLogLevel(int ll);

		void closefile();
		uint getmode();

	};

template<typename... Rest> void log_gen_push(int tp, const tbgeneral::param_stringA & frmt, const Rest&... rest){
	log_decls_t::getcurrentlog()->push(tp, frmt, rest...);
};
template<typename... Rest> void log_gen_push( const tbgeneral::param_stringA& frmt, const Rest&... rest) {
	log_decls_t::getcurrentlog()->push(loglev_Always, frmt, rest...);
};

template<typename... Rest> void log_err_push( const tbgeneral::param_stringA& frmt, const Rest&... rest) {
	log_decls_t::getcurrentlog()->push(loglev_Error, frmt, rest...);
};
template<typename... Rest> void log_common_push(const tbgeneral::param_stringA& frmt, const Rest&... rest) {
	log_decls_t::getcurrentlog()->push(loglev_Always, frmt, rest...);
};
}; //namespace tbgeneral



namespace crsys{
using log_handle= tbgeneral::log_handle;

}; // namespace crsys{