#pragma once

#include "t_array.h"
#include <string>
#include <ostream>




namespace tbgeneral {

template<typename TChar> class rc_string : public rc_basic_array<TChar, ebao_STRMODE> {
public:
	using thistype = rc_string<TChar>;
	using size_type = typename thistype::size_type;


	size_type length() const { return this->size(); }
	rc_string() {};

	rc_string(const TChar* s, const TChar* e) { this->lassign(s, e-s); };
	thistype& assign(const TChar* s, const TChar* e) { this->lassign(s, e-s); return *this; };
	thistype& append(const TChar* s, const TChar* e) { this->lappend(s, e - s); return *this; };

	rc_string(const TChar* d, size_type cnt) { this->lassign(d, cnt); };
	thistype& assign(const TChar* d, size_t count) { this->lassign(d, count); return *this; };
	thistype& append(const TChar* d, size_t count) { this->lappend(d, count); return *this; };
	thistype& insert(size_t pos, const TChar* d, size_t count) { this->linsert(pos, d, count); return *this; };

	rc_string(const thistype& d) { this->lassign(d); };
	thistype& append(const thistype& d) { this->lappend(d.data(), d.size()); return *this; };
	thistype& assign(const thistype& d) { this->lassign(d); return *this; };
	thistype& operator += (const thistype& d) { return append(d); };
	thistype operator + (const thistype& d) const { auto r=*this; return r.append(d);  };
	thistype& operator = (const thistype& d) { return assign(d); };
	thistype& insert(size_type pos, const thistype& d) { this->linsert(pos, d.data(), d.size()); return *this; };

	rc_string(const TChar* d) { assignPCnst(d); };
	thistype& operator += (const TChar* d) { return append(d); };
	thistype& operator = (const TChar* d) { assignPCnst(d); return *this; };
	thistype operator + (const TChar * d) const { auto r=*this; return r.append(d);  };
	thistype& append(const TChar* d) { this->lappend(d, cstylelen(d)); return *this; };
	thistype& assign(const TChar* d) { assignPCnst(d); return *this; };
	thistype& insert(size_type pos, const TChar* d) { this->linsert(pos, d, cstylelen(d)); return *this; };

	rc_string copy() { return rc_string( this->cbegin(), this->size() ); };

	bool is_cstr()const { return this->fsize==0 || this->sdata || (0 == this->sdata[this->fsize]); }
	thistype slice(size_type sti, size_type count = -1) const { thistype res; this->lslice(res, sti, count); return res; }
	thistype tocstr() const {
		if (is_cstr()) return *this;
		auto ns(*this); ns.makeunique();
		return ns;
	}
	thistype tocstr(void* localbuff, size_type sz) const {
		if (is_cstr()) return *this;
		thistype ns; ns.assign_storage(localbuff, sz); ns.assign(ns.cdata(), ns.size());
		return ns;
	}
	template<size_t N> thistype tocstr(TChar (&v)[N]) const { return tocstr(v, N*sizeof(TChar)); }
	thistype to_buff(void * localbuff, size_type sz) const {
		thistype res;
		res.assign_storage(localbuff, sz);
		res.assign(this->data(), this->size());
		return res;
	}
	template<size_t N> thistype to_buff(TChar(&v)[N]) { return to_buff(v, N); }
	const TChar* c_str() const {
		static  const TChar ZV = 0;
		if (this->sdata == 0 || this->fsize == 0) return &ZV;
		if (is_cstr()) return this->data();
		RAISE("slice is not a c string");
	}



	using std_string = std::basic_string<TChar>;
	operator std_string() { return std_string(this->data(), this->size()); }
	rc_string(const std_string& d) { this->lassign(d.data(), d.size()); };
	thistype& assign(const std_string& d) { this->lassign(d.data(), d.size()); return *this; };
	thistype& operator = (const std_string& d) { return assign(d); };
	thistype& operator += (const std_string& d) { return append(d.data(),d.size()); };
	thistype operator + (const std_string& d) const { auto res=*this;  return res.append(d.data(), d.size()); };

	template<typename SChar> rc_string(const std::basic_string<SChar>& d) {	convert(*this, d.data(), d.size());	};
	template<typename SChar> thistype& assign(const std::basic_string<SChar>& d) { convert(*this, d.data(), d.size()); return *this; };
	template<typename SChar> thistype& operator = (const std::basic_string<SChar>& d) { convert(*this, d.data(), d.size()); return *this; };
	template<typename SChar> thistype& operator += (const std::basic_string<SChar>& d) { append_convert(*this,d.data(), d.size()); return *this; };
	//template<typename SChar> thistype operator + (const std::basic_string<SChar>& d) { auto res = *this; append_convert(res,d.data(), d.size());  return res; };
	template<typename SChar> rc_string(const SChar* d) { convert(*this, d, strlen_s(d) ); };
	template<typename SChar> thistype& assign(const SChar* d) { convert(*this, d, strlen_s(d)); return *this; };
	template<typename SChar> thistype& operator = (const SChar* d) { convert(*this, d, strlen_s(d)); return *this; };
	template<typename SChar> thistype& operator += (const SChar* d) { append_convert(*this, d, strlen_s(d)); return *this; };
	template<typename SChar> rc_string(const SChar* d, size_t cnt) { convert(*this, d, cnt); };
	template<typename SChar> thistype& assign(const SChar* d, size_t cnt) { convert(*this, d, cnt); return *this; };
	template<typename SChar> rc_string(const rc_string<SChar>& d) { convert(*this, d.data(), d.size()); };
	template<typename SChar> thistype& assign(const rc_string<SChar>& d) { convert(*this, d.data(), d.size()); return *this; };
	template<typename SChar> thistype& operator = (const rc_string<SChar>& d) { convert(*this, d.data(), d.size()); return *this; };
	template<typename SChar> thistype& operator += (const rc_string<SChar>& d) { append_convert(*this, d.data(), d.size()); return *this; };
	//template<typename SChar> thistype operator + (const rc_string<SChar>& d) { auto res = *this; append_convert(res, d.data(), d.size()); return res; };

	

	bool isConstSegment(const TChar* d) { return crsys::testPointerInConstSegment(d); }
	//bool SetExternalStorage( void* data , size_t binsize) {} ;
	//bool FinalizeExternalStorage(void* data, size_t binsize) {};
	void MakeSafeStorage() { if (!isTruePointer()) this->makeunique();};
	//static const thistype makeAsCnst(const TChar* d) { return makeAsCnst(d, strlen_s(d)); }
	//static const thistype makeAsCnst(const TChar* d, size_type cnt) { thistype res; res.assign_cnstval(d, cnt); return res; }
private:
	bool isTruePointer() { return !this->size() || this->fstorage || isConstSegment(this->sdata); }
	void assignPCnst(const TChar* d) {
		if (isConstSegment(d))
			this->assign_cnstval(d, cstylelen(d));
		else this->lassign(d, cstylelen(d));
	};
	size_type cstylelen(const TChar* d) { return strlen_s(d); }; //TODO:
public:
	thistype& replace(size_type _pos, size_type _olen, const  TChar* d){ return replace(_pos,_olen,d, strlen_s(d)); }
	thistype& replace(size_type _pos, size_type _olen, const  thistype& d) { return replace(_pos, _olen, d.data(), d.size()); }
	thistype& replace(size_type _pos, size_type _olen, const  TChar* d, size_t _ilen ){
		size_type sz = this->size(), nsz;
		if (_pos < 0) { _olen += _pos; _pos = 0; }; if (_pos >= sz) { _olen = 0; _pos = sz; };
		_olen = std::max<size_type>(std::min(_pos + std::max<size_type>(_olen, 0), sz) - _pos, 0);
		nsz = sz - _olen + _ilen; 
		if (nsz > sz) this->doresize(nsz, thistype::eoptMakeUnique); else this->makeunique();
		copydata(this->sdata + _pos + _ilen, this->sdata + _pos + _olen, sz - _pos - _olen);
		copydata(this->sdata + _pos, d, _ilen);
		if (nsz < sz) this->resize(nsz);
		return *this;
	}
	thistype substr(size_type sti, size_type count = -1) const { return slice(sti,count); }
	size_type swap(thistype& v) const { auto r=v; v=*this; *this=v; };
	size_type find(const TChar* s, size_type pos, size_type n) const {};
	size_type find(const TChar s, size_type pos=0) const { return find_char( this->cbegin()+pos, this->cend() , s ); };
	size_type find(const thistype& str, size_type pos = 0) const { return find(str.data(),pos,str.size()); };
	size_type find(const TChar* s, size_type pos = 0) const { return find(s,pos,strlen_s(s));};
	// compare
	//size_type find_first_not_of(const basic_string& str, size_type pos = 0) const;
	//size_type find_first_not_of(const charT * s, size_type pos = 0) const;
	//size_type find_first_not_of(const charT* s, size_type pos, size_type n) const;
	//size_type find_first_not_of(charT c, size_type pos = 0) const;
	//size_type find_first_of(const basic_string& str, size_type pos = 0) const;
	//size_type find_first_of(const charT * s, size_type pos = 0) const;
	//size_type find_first_of(const charT* s, size_type pos, size_type n) const;
	//size_type find_first_of(charT c, size_type pos = 0) const;
	//size_type find_last_of(const basic_string& str, size_type pos = npos) const;
	//size_type find_last_of(const charT * s, size_type pos = npos) const;
	//size_type find_last_of(const charT* s, size_type pos, size_type n) const;
	//size_type find_last_of(charT c, size_type pos = npos) const;
	//size_type find_last_not_of(const basic_string& str, size_type pos = npos) const;
	//size_type find_last_not_of(const charT * s, size_type pos = npos) const;
	//size_type find_last_not_of(const charT* s, size_type pos, size_type n) const;
	//size_type find_last_not_of(charT c, size_type pos = npos) const;

	//TODO: shift_begin
	thistype slice(const TChar* s, const TChar* e) const { return slice(s - this->cbegin(), e-s); }
	thistype slice(const TChar* s) const { return slice(s - this->cbegin()); }
	void set_begin(const TChar* s) { auto e=this->sdata+this->fsize;  this->sdata = const_cast<TChar*>(s); this->fsize= e-this->sdata; }
	void set_end(const TChar* e) { this->fsize=e-this->sdata; }
	void reset_size() { this->fsize=0; } //сброс размера без освобождения storage
	using string_type = thistype;
};


// нужен как параметр процедуры, которая не выводит строку наружу. тогда char* как параметр никогда не приведет к хипу
template <typename TChar> struct param_string {
	rc_string<TChar> param;
	param_string(const TChar* v) { param.assign_cnstval(v, strlen_s(v)); }
	param_string(const TChar* v, size_t sz) { param.assign_cnstval(v, sz); }
	template<typename DChar> param_string(const rc_string<DChar>& s) { param = s; }
	template<typename DChar> param_string(const std::basic_string<DChar>& s) { param = s; }
	private: 
	param_string(const param_string & cp) {  }; // disabled
	param_string& operator =(const param_string& cp) {};
};

template <typename TChar> rc_string<TChar> param2string(const TChar* v) { rc_string<TChar> p; p.assign_cnstval(v, strlen_s(v)); return p; }


}//namespace tbgeneral

namespace tbgeneral {
	char* memset_char(char* dest, char c, size_t count);
	wchar_t* memset_char(wchar_t* dest, wchar_t c, size_t count);
	template<typename TChar> rc_string<TChar> str_aslocal(const TChar* v) { rc_string<TChar> res; res.assign_cnstval(v,strlen_s(v)); return res; }
	template<typename TChar> rc_string<TChar> str_aslocal(const TChar* v,size_t cnt) { rc_string<TChar> res; res.assign_cnstval(v, cnt); return res; }


	void convert(rc_string<char>& d, const wchar_t* s, size_t len);
	void convert(rc_string<wchar_t>& d, const char* s, size_t len);
	void append_convert(rc_string<char>& d, const wchar_t* s, size_t len);
	void append_convert(rc_string<wchar_t>& d, const char* s, size_t len);


	inline void convert(rc_string<char>& d, const wchar_t* s) { convert(d, s, strlen_s(s)); };
	inline void convert(rc_string<char>& d, const std::basic_string<wchar_t>& s) { convert(d, s.data(), s.size()); };
	inline void convert(rc_string<char>& d, const rc_string<wchar_t>& s) { convert(d, s.data(), s.size()); };
	inline void convert(rc_string<wchar_t>& d, const char* s) { convert(d, s, strlen_s(s)); };
	inline void convert(rc_string<wchar_t>& d, const std::basic_string<char>& s) { convert(d, s.data(), s.size()); };
	inline void convert(rc_string<wchar_t>& d, const rc_string<char>& s) { convert(d, s.data(), s.size()); };

	template <typename C> void convert(rc_string<C>& res, int8 d) { convert(res, int64(d)); };
	template <typename C> void convert(rc_string<C>& res, int16 d) { convert(res, int64(d)); };
	template <typename C> void convert(rc_string<C>& res, int32 d) { convert(res, int64(d)); };
	template <typename C> void convert(rc_string<C>& res, uint8 d) { convert(res, uint64(d)); };
	template <typename C> void convert(rc_string<C>& res, uint16 d) { convert(res, uint64(d)); };
	template <typename C> void convert(rc_string<C>& res, uint32 d) { convert(res, uint64(d)); };


	inline rc_string<char>		operator+ (const rc_string<char>& s, const wchar_t* d) { auto res = s; append_convert(res, d, strlen_s(d)); return res; };
	inline rc_string<wchar_t>	operator+ (const rc_string<wchar_t>& s, const char* d) { auto res = s; append_convert(res, d, strlen_s(d)); return res; };
	inline rc_string<char>		operator+ (const rc_string<char>& s, const std::basic_string<wchar_t> d) { auto res = s; append_convert(res, d.data(), d.size()); return res; };
	inline rc_string<wchar_t>	operator+ (const rc_string<wchar_t>& s, const std::basic_string<char> d) { auto res = s; append_convert(res, d.data(), d.size()); return res; };
	inline rc_string<char>		operator+ (const rc_string<char>& s, const rc_string<wchar_t> d) { auto res = s; append_convert(res, d.data(), d.size()); return res; };
	inline rc_string<wchar_t>	operator+ (const rc_string<wchar_t>& s, const rc_string<char> d) { auto res = s; append_convert(res, d.data(), d.size()); return res; };

	template<typename TChar> std::basic_ostream<TChar>& operator << (std::basic_ostream<TChar>& os, const rc_string<TChar>& d) {
		return os.write(d.data(), d.size());
	}
	std::basic_ostream<char>& operator << (std::basic_ostream<char>& os, const rc_string<wchar_t>& d);
	std::basic_ostream<wchar_t>& operator << (std::basic_ostream<wchar_t>& os, const rc_string<char>& d);
	template<typename TChar, typename DChar> std::basic_ostream<TChar>& operator << (std::basic_ostream<TChar>& os, const param_string<DChar>& d) {
		os << d.param; return os;
	}

	//inline std::string& operator <<(std::string& s, const char* d) { s.append(d);	return s; }
	template <class Cs, class Cd>  rc_string<Cd>& operator << (rc_string<Cd>& d, const Cs* s) { d.append(s);	return d; }
	template <class Cs, class Cd>  rc_string<Cd>& operator << (rc_string<Cd>& d, const rc_string<Cs>& s) { d.append(s);	return d; }
	template <class C>  rc_string<C>& operator << (rc_string<C>& d, C s) { d.append(&s, 1);	return d; }

	//template <class Cs, class Cd>  rc_string<Cd>  operator + (const rc_string<Cd>& d, const Cs* s) { auto r = d;	 r.append(s);	return r; }
	//template <class Cs, class Cd>  rc_string<Cd>  operator + (const rc_string<Cd>& d, const rc_string<Cs>& s) { auto r = d; r.append(s);	return r; }

	template<class TChar > bool operator < (const rc_string<TChar>& l, const rc_string<TChar>& r) { return 0>strcmp(l,r); };
	template<class TChar > bool operator < (const rc_string<TChar>& l, const TChar* r) { return 0>strcmp(l.data(), r, l.size(), strlen_s(r)); };
	template<class TChar > bool operator <= (const rc_string<TChar>& l, const rc_string<TChar>& r) { return strcmp(l, r) <= 0; };
	template<class TChar > bool operator <= (const rc_string<TChar>& l, const TChar* r) { return 0 >= strcmp(l.data(), r, l.size(), strlen_s(r)); };
	template<class TChar > bool operator > (const rc_string<TChar>& l, const rc_string<TChar>& r) { return strcmp(l, r) > 0; };
	template<class TChar > bool operator > (const rc_string<TChar>& l, const TChar* r) { return 0 < strcmp(l.data(), r, l.size(), strlen_s(r)); };
	template<class TChar > bool operator >= (const rc_string<TChar>& l, const rc_string<TChar>& r) { return strcmp(l, r) >= 0; };
	template<class TChar > bool operator >= (const rc_string<TChar>& l, const TChar* r) { return 0 <= strcmp(l.data(), r, l.size(), strlen_s(r)); };
	template <class TChar> bool operator == (const rc_string<TChar>& l, const rc_string<TChar>* r) { return 0 == strcmp(l.data(), r.data(), l.size(), r.size()); };
	template <class TChar> bool operator == (const rc_string<TChar>& l, const TChar* r) { return 0 == strcmp(l.data(), r, l.size(), strlen_s(r)); };
	template <class TChar> bool operator == (const rc_string<TChar>& l, const rc_string<TChar>& r) { return 0 == strcmp(l.data(), r.data(), l.size(), r.size()); };
	template <class TChar> bool operator == (const rc_string<TChar>& l, const std::basic_string<TChar>& r) { return 0 == strcmp(l.data(), r.data(), l.size(), r.size()); };
	template <class TChar> bool operator == (const std::basic_string<TChar>& l, const rc_string<TChar>& r) { return 0 == strcmp(l.data(), r.data(), l.size(), r.size()); };
	template <class TChar> bool operator != (const rc_string<TChar>& l, const TChar* r) { return 0 != strcmp(l.data(), r, l.size(), strlen_s(r)); };
	template <class TChar> bool operator != (const rc_string<TChar>& l, const rc_string<TChar>& r) { return 0 != strcmp(l.data(), r.data(), l.size(), r.size()); };
	template <class TChar> bool operator != (const rc_string<TChar>& l, const std::basic_string<TChar>& r) { return 0 != strcmp(l.data(), r.data(), l.size(), r.size()); };
	template <class TChar> bool operator != (const std::basic_string<TChar>& l, const rc_string<TChar>& r) { return 0 != strcmp(l.data(), r.data(), l.size(), r.size()); };

	template <class charT> int stricmp(const rc_string<charT> l, const rc_string<charT> r) { return f_stricmp(l.data(), r.data(), l.size(), r.size()); }
	template <class charT> int stricmp(const rc_string<charT> l, const std::basic_string<charT> r) { return f_stricmp(l.data(), r.data(), l.size(), r.size()); }
	template <class charT> int stricmp(const rc_string<charT> l, const charT* r) { return f_stricmp(l.data(), r, l.size(), strlen_s(r)); }
	template <class charT> int strcmp(const rc_string<charT> l, const charT * r) { return strcmp(l.data(), r , l.size(), strlen_s(r) ); }
	template <class charT> int strcmp(const rc_string<charT> l, const rc_string<charT> r) { return strcmp(l.data(), r.data(), l.size(), r.size()); }
	template <class charT> int strcmp(const rc_string<charT> l, const std::basic_string<charT> r) { return strcmp(l.data(), r.data(), l.size(), r.size()); }
	template <class charT> int strcmp(const std::basic_string<charT> l, const rc_string<charT> r) { return strcmp(l.data(), r.data(), l.size(), r.size()); }

	
	template <class charT> rc_string<charT> strupper(const rc_string<charT>& l) { rc_string<charT> r = l; r.makeunique();	f_strtoupper(r.data(), r.size()); return r; }
	template <class charT> rc_string<charT> strlower(const rc_string<charT>& l) { rc_string<charT> r = l; r.makeunique();	f_strtolower(r.data(), r.size()); return r; }

	//stringA format(const char* frmt, ...);
	//const char* bformat(stringA& res, const char* frmt, ...);

	//stringA  vformat(va_params_t& vaparams);
	//const char* vbformat(stringA& res, va_params_t& vaparams);
	//template <class TChar> std::basic_string<TChar> tostd_str(const lstring<TChar>& d) { return std::basic_string<TChar>(d.c_str(), d.size()); }





	//std::string& pushalign(std::string& s, const char* d, int al);
	//std::string& pushalign(std::string& s, uint v, int al);


	//template<class String> stringA tostring(const String& s) { return stringA(s.c_str(), s.size()); }
	//template<class String> stringW tostringW(const String& s) { return stringW(s.c_str(), s.size()); }



	rc_string<char> trim(const rc_string<char>& s);
	rc_string<wchar_t> trim(const rc_string<wchar_t>& s);
	rc_string<char> trim(const char* s, size_t cnt);
	rc_string<wchar_t> trim(const wchar_t* s, size_t cnt);
	template <typename C> rc_string<C> trim(const C* s) { return trim(s, strlen_s(s)); }

	rc_string<char> replace_str(const rc_string<char>& _s, const rc_string<char>& from, const rc_string<char>& to);
	rc_string<wchar_t> replace_str(const rc_string<wchar_t>& _s, const rc_string<wchar_t>& from, const rc_string<wchar_t>& to);
	rc_string<char> replacechars(const rc_string<char>& src, const rc_string<char>& find_set, const rc_string<char>& repl);

	rc_string<char> find_str(const rc_string<char>& s, const rc_string<char>& substr);
	rc_string<wchar_t> find_str(const rc_string<wchar_t>& s, const rc_string<wchar_t>& substr);
	int findchars(const rc_string<char>& src, const param_string<char>& find_set );
	size_t find_char( const char * begin, const char * end , char fc );
	size_t find_char( const wchar_t * begin, const wchar_t * end , char fc );

	rc_array< rc_string<char> > split_str(const rc_string<char>& d, char splitset);
	rc_array< rc_string<char> > split_str(const rc_string<char>& s, const rc_string<char>& substr);
	rc_array< rc_string<char> > split_chars(const rc_string<char>& s, const rc_string<char>& substr);
	int split_str2(const rc_string<char>& d, char splitset, rc_string<char>& l, rc_string<char>& r);


	template<class TArray,typename TChar> rc_string<TChar> join(const TArray& a, const rc_string<TChar>& sep) {
	//template<typename Elem , typename TChar> rc_string<TChar> join(const rc_array<Elem>& a, const rc_string<char>& sep) {
		rc_string<TChar> res;
		for (uint i = 0; i < a.size(); i++) {
			if (res.size()) res.append(sep);
			res.append(a[i]);
		}
		return res;
	}
	template<class TArray, typename TChar> rc_string<TChar> join(const TArray& a, const TChar* sep) {
		return join(a, str_aslocal(sep));
	}




	//template <class strt> strt str_replace(strt str, const strt& from, const strt& to) 

	//transfer to locale!
	// s_char2wchar_t|s_wchar_t2char: returned write chars count.
	size_t sarr_convert(wchar_t* dest, size_t destsize, const char* src, size_t srcsize, size_t& readsz);
	// s_char2wchar_t|s_wchar_t2char: returned write chars count.
	size_t sarr_convert(char* dest, size_t destsize, const wchar_t* src, size_t srcsize, size_t& readsz);


	template<typename TChar , size_t N> rc_string<TChar> str2localstorage(TChar(&v)[N]) { 
		rc_string<TChar> r; r.assign_storage(v, N * sizeof(TChar));
		return r; }
	//assign_cnstval(d, cstylelen(d));


}//namespace tbgeneral


namespace tbgeneral {

	using stringA = rc_string<char>;
	using stringW = rc_string<wchar_t>;
	using param_stringA = param_string<char>;
	using param_stringW = param_string<wchar_t>;
	//using partstringA = rc_string<char>;
	//using partstringW = rc_string<wchar_t>;
};
