#pragma once

#include "t_string.h"
#include "t_format.h"

namespace tbgeneral {

	struct t_error_info {
		enum { eLOGWrite = 0x4 << 20, eFlagsMask = 0xFF << 20 };
		int mflags;
		int error_id;
		stringA text;
		t_error_info() { error_id = 0; mflags = 0; }
		t_error_info(bool logw) { error_id = 0; mflags = logw ? eLOGWrite : 0; }
		void EnableLogWrite(bool en) { mflags = en ? (mflags | eLOGWrite) : (mflags & ~eLOGWrite); }
		//int pushf(int err, const char* inf, ...);
		template<typename... Rest> int pushf(int err, const char* inf, const Rest&... rest) {
			return push(err, format(inf, rest...));
		};
		//int push(int err, const char* inf);
		//int push(int err, const stringA& inf);
		int push(int err, const param_stringA& inf);
		int push(const t_error_info& e);
		void reset() { error_id = 0; text = ""; }
	};

}; // namespace tbgeneral{