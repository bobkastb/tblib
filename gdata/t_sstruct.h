#pragma once

#include "tb_basedtools.h"
#include "tb_exception_base.h"
#include "tsys/tb_sysfun.h"
#include "tb_algo.h"

#include "w-metadata.h"


namespace tbgeneral {
	template<class TStruct> size_t sizeofrec(const TStruct& s) { return sizeof(s); }
	enum { eABSTRACT_PTR = 5533 };
	template<typename t_TStruct> struct _struct_header {
		using TStruct = t_TStruct;
		typedef _struct_header<TStruct> thistype;
		typedef _struct_header<TStruct>* pointer;
		int refcnt, shared_id; uint typeinfo; TStruct data[1];

		int refcount() const { return  this ? calcthis()->refcnt : 0; }
		void decuse() { calcthis()->_decuse(); }
		void incuse() { calcthis()->_incuse(); };

		template <typename UseType> static TStruct* alloc(const UseType* org) {
			pointer t = (pointer)malloc(sizeof(thistype) - sizeof(TStruct) + sizeof(UseType));
			//pointer t= (pointer)malloc(sizeof(thistype)-sizeof(TStruct)+sizeofrec(*org));
			RESETMEM(*t);
			t->refcnt = 1;
			constructorarray<UseType>((UseType*)t->data, 1, org, org ? 1 : 0);
			return t->data;
		}
		static TStruct* allocdef(const TStruct* org) {
			return alloc<TStruct>(org);
		}

		TStruct* makeunique() {
			pointer t = calcthis(); if (!this) return 0;
			return (t->refcnt > 1) ? alloc(t->data) : t->data;
		};

		static pointer __dbg_calcthis(void* p) { return pointer(p)->calcthis(); };
	private:
		TStruct* getdata() { return this->data; }
		_struct_header() {};
		~_struct_header() {};
		thistype* calcthis() const { return !this ? 0 : (pointer)((int8*)this - (int8*)((pointer)eABSTRACT_PTR)->data + eABSTRACT_PTR); };
		void _decuse()
		{
			if (!this) return; int r = crsys::_InterlockedDecrement(&refcnt);
			if (r <= 0) { destructorarray(data, 1); ::free(this); };
		};
		void _incuse() { if (!this) return; crsys::_InterlockedIncrement(&refcnt); };
	};

	template<typename t_TStruct> struct rc_base_struct {
		//typedef t_TStruct TStruct;
		using TStruct = t_TStruct;
		using thistype = rc_base_struct<TStruct>;
		typedef rc_base_struct<TStruct>* pointer;
		typedef rc_base_struct<TStruct>& ref;
		//typedef _struct_header<TStruct> headertp;
		using headertp = _struct_header<TStruct>;
		typedef _struct_header<TStruct>* hp;
		typedef TStruct* TStruct_Ptr;
		TStruct* fdata;
		//static thistype CreateNew() { thistype t; t.fdata = headertp::alloc<TStruct>(0);  return t; };
		static thistype CreateNew() { thistype t; t.fdata = headertp::allocdef(0);  return t; };
		template <typename DestType> static thistype CreateNewAs() { thistype t; t.fdata = headertp::template alloc<DestType>(0);  return t; };
		thistype& create() { clear(); fdata = headertp::allocdef(0);  return *this; };
		rc_base_struct() { fdata = 0; }
		//rc_base_struct(bool donew) { fdata = 0; }	
		~rc_base_struct() { clear(); }
		rc_base_struct(const TStruct& s) { fdata = 0; fdata = headertp::allocdef(&s); }
		rc_base_struct(const thistype& ss) { fdata = ss.fdata; hp(fdata)->incuse(); }
		thistype& operator = (const TStruct& s) { fdata = hp(fdata)->makeunique(); if (!fdata) fdata = headertp::allocdef(&s); else *fdata = s; return *this; }
		thistype& operator = (const thistype& ss) { if (fdata != ss.fdata) { hp(fdata)->decuse(); fdata = ss.fdata; hp(fdata)->incuse(); } return *this; }
		void clear() { if (fdata) { hp(fdata)->decuse(); fdata = 0; } }
		TStruct* data() const { return fdata; }
		TStruct* operator ->() const { return fdata; };
		TStruct& operator *() const { return *fdata; };
		bool empty() const { return fdata==0; }
		//operator TStruct_Ptr() const { return data; }
	};

	//using sher	rc_base_struct
	template <typename E> using shared_ptr = tbgeneral::rc_base_struct<E>;

}; //namespace tbgeneral 
