#include <cctype>
#include <cwctype>

#include <gdata/tb_c_tools.h>
#include "tb_exception_base.h"
#include "tb_vector_cs.h"

#include "gdata/t_string.h"
#include "tb_buffer.h"

#include "t_error.h"
#include "t_format.h"

#include "tsys/tb_log.h"
#include "tsys/tb_sysdecls.h"


//#include "gen_tools.h"

#include <stdarg.h>



namespace tbgeneral{

void t_bitstream::push( int v ){
	auto bc = countbits_in & 7;
	uint sz = ( countbits_in >> 3 ) + 1;
	if (sz>= bitstream.size() ) { bitstream.push_back(0); };
	auto p= &bitstream[sz-1];
	if (v & 1) { *p |= 1<<bc; }
	countbits_in++;
};
bool t_bitstream::eof(){
	return countbits_in>= bitstream.size()*8;
};
bool t_bitstream::eof(int delta){
	return int(countbits_in)-delta>= int(bitstream.size())*8;
};
int t_bitstream::pop(){
	auto bc = countbits_in & 7;
	uint sz = ( countbits_in >> 3 ) + 1;
	countbits_in++;
	if (sz> bitstream.size() ) 
		{ return 0; }
	auto p= &bitstream[sz-1];
	return  (p[0] >> bc) & 1; 
};

void t_bitstream::finish_push() {
	uint sz= (countbits_in >> 3) + (countbits_in & 7?1:0);
	bitstream.resize(sz);
};
void t_bitstream::prepare_push( uint sz ){
	bitstream.resize(sz); 
	countbits_in=0;
	memset(bitstream.data(),0,bitstream.size());
};
void t_bitstream::prepare_pop( const darray<byte> & d ){
	countbits_in=0;
	bitstream = d;
};

}; //namespace tbgeneral{



warning_MSVC(disable , 4996)


namespace tbgeneral{

	// 1 - lp in rp , 2 - rp in lp , 3) lp > rp , 4) lp < rp
int intersection_mem(const void* _lp, size_t lsize, const void* _rp, size_t rsize) {
	auto lp = (const char*)_lp;
	auto rp = (const char*)_rp;
	auto l_E = lp + lsize, r_E = rp + rsize;

	if (lp < rp) {
		if (l_E <= rp) return 0;
		return (r_E <= l_E) ? 2 : 4;
	} else if (lp == rp) {
		return lp + lsize <= rp + rsize ? 1 : 2;
	} else { // rp<lp 
		if (r_E <= lp) return 0;
		return (l_E <= r_E) ? 1 : 3;
	}
};



int bitpower(uint l){
	static byte bc[]={0,1,1,2, 1,2,2,3 ,1,2,2,3 ,2,3,3,4};
	int r=0;
	for (;l;l>>=4) r+=bc[l & 0xf];
	return r;
}


std::string& pushalign(std::string&s , const char* d , int al){
	size_t l= strlen(d)-al;
	s.append( d );
	for (;l<0;l++) s.append(" "); ;
	return s;
}
std::string& pushalign(std::string&s , uint v , int al){
	char buff[256]; 
	sprintf(buff,"%d",v);
	return pushalign(s,buff,al);
}







int t_error_info::push(int err, const param_stringA& inf){
//int t_error_info::push(int err, const stringA& inf) {
	auto flags = (err & eFlagsMask) | mflags; err &= ~eFlagsMask;
	if (text.size()) text << "\n";
	error_id = err;
	auto back = text.size();
	//text<<oldformat("Error:%d:", err);
	format_to(text,"Error:%d:%s", err, inf.param );
	if (flags & eLOGWrite) LOG("%s", text.slice(back) );
	return error_id;
};
//int t_error_info::push( int err, const char * inf ){	return push(err, param_string<char>(inf).param );};

int t_error_info::push( const t_error_info & e ){
	if (&e == this) return error_id;
	if (e.text.size()) {
		if (text.size()) text<<"\n";
		text << e.text;
		if (mflags & ~e.mflags & eLOGWrite)
			LOG("%s", e.text );

	};	
	if (e.error_id)	error_id = e.error_id;
	return error_id;
};






// tb_vector_cs.h , for cBitSetC
void init_bitset_data( void * data , uint datasize, const stringA & bitnumbers_list ){
	memset(data,0,datasize);
	auto e = bitnumbers_list.cend();
	for (auto s=bitnumbers_list.cbegin(); s<e ;s++) {
		uint ofs = byte(*s) /8 , bitofs= byte(*s)%8; 
		if ( ofs>=datasize ) {  throw("Range Error!"); continue;  }
		((byte*)data)[ofs] |= 1<<bitofs;
	}
};



}; // namespace tbgeneral{


namespace crsys{

void eraise(const char* frmt,...){
	char s[2048]; 
    va_list marker;   va_start( marker, frmt );
	tb_vsnprintf(s, sizeof(s), frmt, marker ); 
	throw crsys::exception((const char*)s);
};
exception & exception::format(const char* frmt,...){
	char _s[4*1024], *s=_s, *e=s+sizeof(_s)-1;
	if (einf.file ) {
		int l=tb_snprintf(s, e-s, "RAISE error! at (%s: %d)  " , einf.file , einf.linenumber ); 
		if (l<0) s=e; else s+=l;
	};
    va_list marker;   va_start( marker, frmt );
	tb_vsnprintf(s, e-s , frmt, marker ); 
	*e=0; this->assign_text( _s );
	LOG_ERROR("%s" , _s );
	return *this;
};

void raise_do(const char * x , const char * _file, int _linen   ){
	LOG_ERROR("RAISE error! at (%s: %d) : '%s' " , _file , _linen , x );
	crsys::eraise("RAISE error! at (%s: %d) : '%s' " , _file , _linen);
};

void exception::assign_text( const char* s){
	if (ex) { free(ex); ex=0;}
	auto sz = tbgeneral::strlen_s( s );
	ex = (char*)malloc( sz+1 );
	memcpy( ex , s , sz+1 );
}; 

}; // namespace crsys{





/*

namespace tbgeneral{ namespace test{
	void testofcmp(){
		stringW sw;stringA sa;
		sw<<"sss"<<L"dd"<<sw<<sa;	
		sa<<"sss"<<L"dd"<<sw<<sa;	
		stringW("rrttt");	
		stringA(L"rrttt");
			
	}
};}; //namespace tbgeneral{ namespace test{
	*/




