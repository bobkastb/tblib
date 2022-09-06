#include "tsys/tb_log.h"
#include <fstream>
#include <tsys/tbfiles.h>
#include <gdata/tb_env.h>
#include <parse/t_path.h>
namespace tbgeneral {

	int log_object_t::gCurrentLogLevel = 0;
	log_object_t* log_decls_t::currentlog = 0;
	log_object_t log_decls_t::defaultlog;

	log_object_t* log_decls_t::getcurrentlog() {
		return currentlog ? currentlog : &defaultlog;
	}

	void log_object_t::closefile(){
		auto locker = lockauto();
		if (os) { delete os; os = 0; }
	};

	void log_object_t::close() {
		auto locker = lockauto();
		if (currentlog == this)	currentlog = 0;
		for (auto rp : mresources)
			delete rp;
		mresources.resize(0);
		if (os) { delete os; os = 0; }
	};


	//int log_object_t::add_redirector(f_printfrepeater f) { redirectors.push_back(f); return 0; };
	int log_object_t::add_redirector(l_printfrepeater f, uint8_t flags ) { 
		lredirectors.push_back({ f,flags }); return 0; 
	};

	bool log_object_t::log_open(const stringA& fnm, const stringA& dataprefix, bool asdef, uint _mode) {
		close();
		auto locker = lockauto();
		mode = _mode;
		prefix = dataprefix;
		filename = fnm;
		if (!fnm.empty()) {
			if (0 > findchars(fnm, "/\\")) {
				filename = expand_environment_string(fnm);
				auto ldir = tbgeneral::get_file_dir(filename);
				filesystem::MakeDir(ldir);
			}
			else {
				auto ldir = filesystem::get_programm_dir() + "/logs/";
				filesystem::MakeDir(ldir);
				filename = format("%s/logs/%s.log.txt", ldir, fnm);
			}

			auto fs = new std::ofstream();
			auto msk = std::ios::binary | std::ios::out | std::ios::ate;
			msk |= ( (mode & mode_NoAppend) ? std::ios::trunc : 0 );
			filesystem::stream_open(*fs, filename, msk);
			if (!fs->is_open()) {
				//std::cerr << strerror(errno);
				format_to(std::cerr, "Log not open!(%s)", fnm);
				delete fs; fs = 0;
				return false;
			}
			os = fs;
		}
		if (asdef)
			currentlog = this;
		return true;
	};

	void log_object_t::msgflush(log_msg_t* m, const char* data, size_t size) {
		bool wrcon = !(this->mode & mode_NoCout) && ((!isFile()) || (this->mode & mode_ALWAYSPRINTF) || (m->msgtype < 0));
		//for(auto redir : redirectors )	redir( m ,  data, size );
		for (auto redir : lredirectors) {
			auto e = data + size;
			auto ds = (redir.flags & efrd_NoHeader) && (m->posMsg > 0) ? data + m->posMsg : data;
			redir.f(m, ds, e - ds);
		}
		if (os)
			os->write(data, size);
		if (wrcon)
			std::cout.write(data, size);
		m->posMsg = -1; m->posPrefix = -1;
	};

	void log_object_t::init_msg(log_msg_t& m, void* buff, size_t buffsize) {
		m.state = est_Begin;
		m.owner = this;
		m.proxy = &m.ostr;
		m.flags = 0;
		auto mptr = &m;
		auto flushf = [=](const char* d, size_t sz) { this->msgflush(mptr, d, sz); };
		m.ostr.make(flushf, buffsize, buff);
	};
	void log_object_t::begin_msg(int tp, log_msg_t& m) {
		m.msgtype = tp;
		auto& os = m.proxy[0];
		if (0 == (mode & mode_NoPushTIME)) {
			auto frmt = (mode & mode_TIME_HL) ? crsys::ns_timef::frmtYear4 : crsys::ns_timef::frmtYear2;
			crsys::format_datetime_to(os, frmt, crsys::getunixtime_ms());
		}
		m.posPrefix = static_cast<int>(os.tellp());
		format_to(m.proxy[0], " %s>", prefix);
		m.posMsg = static_cast<int>(os.tellp());
		m.state = est_Process;
	};

	void log_object_t::on_begin_msg(int tp, log_msg_t& m, void* buff, size_t buffsize) {
		init_msg(m, buff, buffsize);
		begin_msg(tp, m);
	};

	log_msg_t* log_object_t::on_begin_msg(int tp) {
		log_msg_t* r = 0;
		{	auto locker = lockauto();
		for (auto mp : mresources) if (mp->state == log_msg_t::est_Free) { r = mp; break; }
		if (!r) {
			r = new log_msg_t();
			init_msg(*r, 0, this->msgbuffsize);
			if (mresources.size() < cMaxMessagesCount) {
				mresources.reserve(cMaxMessagesCount);
				mresources.push_back(r);
				r->flags |= emf_InHeapOnly;
			}
		};
		r->state = est_Begin;
		} // end lock scope
		begin_msg(tp, *r);
		return r;
	}
	void log_object_t::on_end_msg(log_msg_t* m) {
		m->proxy[0] << "\n";
		m->proxy[0].flush();
		m->state = est_End;
		m->state = est_Free;
		if (m->flags & emf_InHeapOnly) {
			delete m;
		}
		if (os)
			os->flush();
	};

};// namespace tbgeneral

// handle
//

namespace tbgeneral {
	bool log_handle::isopen() {
		return hndl && hndl->isOpen();
	};

	uint log_handle::getmode() {
		return hndl ? hndl->mode : -1;
	};
	int log_handle::getLogLevel() {
		return hndl ? hndl->getloglevel() : 0;
	};
	void log_handle::setLogLevel(int ll) {
		if (hndl)
			hndl->setloglevel(ll);
	};

	bool log_handle::setmode(uint mode) {
		if (!hndl) return false;
		hndl->setmode(mode);
		return true;
	};

	log_handle::~log_handle() {
		if (hndl) delete hndl;
		hndl = 0;
	};
	void log_handle::closefile() {
		if (hndl) hndl->closefile();
	};

	bool log_handle::log_open(const char* lognm, const char* dataprefix, bool asdef, uint mode) {
		if (!hndl) {
			hndl = new log_object_t();
		};
		auto r = hndl->log_open(lognm, dataprefix, asdef, mode);
		return r;
	};
	bool log_handle::setasdefault() {
		if (!hndl) return false;
		log_decls_t::currentlog = hndl;
		return true;
	};
	void log_handle::setprefix(const param_stringA& prefix) {
		if (!hndl) RAISE("log object not open!");
		hndl->prefix = prefix.param;
	};
	int log_handle::addrepeater(l_printfrepeater f, uint8_t redirector_flag) {
		if (!hndl) return 0;
		hndl->add_redirector(f, redirector_flag);
		return 1;
	};

} // namespace tbgeneral {





using namespace tbgeneral;

void log__DEF(int logt, va_params_t& vaparams) {
	auto s = vformat(vaparams);
	log_gen_push( logt , "%s" , s);
};

void log__DOv(int tp, va_params_t& vaparams) {
	log__DEF(tp, vaparams);
};

void log__ERR(const char* frmt, ...) {
	va_Init(vaparams, frmt);
	log__DEF(-1, vaparams);
};

void log__DO(const wchar_t* frmt, ...) {
	va_Init(vaparams, frmt);
	auto vap = (va_params_t*)&vaparams;
	stringA frmtA = frmt;
	vap->format = frmtA.c_str();
	log__DEF(0, *vap);
};

void log__DO(const char* frmt, ...) {
	va_Init(vaparams, frmt);
	log__DEF(0, vaparams);
}
void log__DO(int tp, const char* frmt, ...) {
	va_Init(vaparams, frmt);
	log__DEF(tp, vaparams);
}

