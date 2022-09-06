#pragma once

#include "tb_basetypes.h"

namespace tbgeneral {

	template <class T> struct c_DataPtr {
		T* p; int cnt;
		c_DataPtr() { p = 0; cnt = 0; }
		c_DataPtr(T* _p, uint _cnt) { p = _p; cnt = _cnt; };
	};
	typedef c_DataPtr<byte> c_BytePtr;
#define INITDptr(nm,s) c_DataPtr<byte> nm(s,sizeof(s))
}