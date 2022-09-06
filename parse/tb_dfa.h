#pragma once

#include "gdata/tb_vector_cs.h"
#include "gdata/t_tools.h"

#include "gdata/t_string.h"

namespace tbgeneral {



//template <class _istate_t=uint16 , class _userdata_t=void* > 
class c_DFA_NativeCode_Base {
public:

typedef uint16 istate_t;
typedef int16 irule_t;
typedef uint8 char_t;
enum {//BADSTATE=istate_t(-1) 
	BADSTATE=0 
	, BAD_TINDEX=0x80000000 
	, BAD_RULE_HI=0 , BAD_RULE=irule_t(-1) };
enum {MB=256};
enum eStatus_t{ eDefStatus=0, ePacked =1 };
typedef cVectorC<char_t,MB> cVectorChars;  
typedef c_DFA_NativeCode_Base thistype;
	
	//struct cRule { _userdata_t userData; };
	struct cState { 
				int startTransferIndex; //для упакованного -начальный смещение в transfers, для неупакованного - в unp_transfers
				irule_t Rule; 
				//istate_t id;
				char_t pre_mask; // and mask for input chars
				bool isfinal() { return Rule>=BAD_RULE_HI; } // pre_mask
			};
	struct cTransfer{ istate_t to,from;};
	struct c_inputword{ // используется в add_In_word
			struct c_symbol { char_t mask; int16 chrsofs; int16 cntchrs; };
			bool overrideRule; 
			irule_t Ud;
			darray<c_symbol> symbols;
			darray<char_t> chars;
			c_inputword() { ZEROMEM(*this); }
			template <class Vec > bool getsymbol(uint num , c_symbol & rs, Vec & ch ) const {
				if (num>=symbols.size()) return false;
				rs=symbols[num]; 
				for (int i=rs.chrsofs;i<rs.chrsofs+rs.cntchrs;i++) ch.push_back(chars[i]);
				return true;
			};
			template <class Vec > int pushsymbol( char_t mask, Vec & ch ) {
				c_symbol r; r.mask = mask; 
				r.cntchrs= static_cast<int>(ch.size());
				r.chrsofs= static_cast<int16>(chars.size());
				symbols.push_back(r);
				for (uint i=0;i<ch.size();i++) chars.push_back(ch[i]);
				return (int)symbols.size()-1;
			};
			int pushsymbol( char_t mask, char_t ch ) {
				cVectorC<char_t,16> chs; 
				chs.push_back(ch); return pushsymbol(mask , chs);
			};
			int whatis( int index) const { int sz=(int)symbols.size();  return index<0 ? -1 : index<sz-1 ? 0 : index==sz-1 ? 1 : 2; };
			bool isend( int index) const { return whatis(index)==2; }
			bool smake( char_t* _mask , char_t* _val , irule_t _Ud , int cnt ) { 
				*this=c_inputword(); Ud= _Ud;
				for (int i=0;i<cnt && _mask[i];i++) { pushsymbol( _mask[i] , _val[i]); };
				return true;
			};
			c_inputword(char_t* _mask , char_t* _val , irule_t _Ud , int cnt ) { smake( _mask ,  _val , _Ud , cnt); } ;
			 //for (int i=0;i<cnt && mask[i];i++) { iw.pushsymbol( mask[i] , val[i]); };

	};

	uint32 charsPower() { return MB; };

	//darray<cRule> rules;
	darray<cState> states;
	darray<cTransfer> transfers;  // при упаковке этот массив полнеет 
	istate_t startstate;
	uint status;

		struct unp_transfer { char_t c; istate_t st; };
	protected:
		struct cEState:public cState { istate_t id; bool empty(){return id==BADSTATE; } };
		cEState getstate(istate_t id){ cEState r= *(cEState*) &states[id]; r.id=id; return r; };
		bool setstate(cEState st) { if (st.id == BADSTATE) doerr(211); states[st.id] = st; return true; };

		typedef darray<unp_transfer> unpacked_transfers_t;
		darray<unpacked_transfers_t> unp_transfers; // при упаковке этот массив пустеет 
		istate_t findunpTransfer( const cState & from , char_t c );
		unpacked_transfers_t & GetMakeTT( cState & from );
		unpacked_transfers_t GetTT( istate_t ist );

		struct c_addindata{ 			istate_t curr;	int mindex; 		};
		struct c_addinctx {
			irule_t Ud;
			istate_t final;
			c_addindata start;
			c_inputword iw;
		};
		int makecopystatechain( istate_t from );
		int add_In_word(c_addinctx & ctx , c_addindata nd);
		int newmaskforstate(cEState & st, char_t nm); 
		void setFinalState( cEState & st , c_addinctx & ctx);

	void makezerostate();
	cEState addnewstate();
	//irule_t newRule(const _userdata_t & Ud){cRule rule; rule.userData=Ud; return rules.push_back( rule );};

	//**************************************
	public:

	c_DFA_NativeCode_Base() { startstate=BADSTATE; status=eDefStatus; }
	int doerr(int errc){ throw errc; };
	void reset() { states.resize(0); transfers.resize(0); unp_transfers.resize(0);
				startstate=BADSTATE; status=eDefStatus;
	};
	void pack();
	bool ispacked() { return status &ePacked; };

	int add_In_word( uint64 mask, uint64 val , irule_t Ud ){ return add_In_word((char_t *)&mask , (char_t *) &val , Ud , sizeof(mask)/sizeof(char_t) ); } ; // длина до 8 байт
	int add_In_word( const c_inputword & iw );
	int add_In_word( char_t * mask, char_t * val , irule_t Ud ,  int cnt=4 );
	

	int find4( uint buff , irule_t & frule ); // return size of cmd, for bad - return 0 , speed find for 4 bytes
	int find( const c_DataPtr<char_t> & buff , irule_t & frule ); // return size of cmd, for bad - return 0 
	int find( const char * buff , irule_t & frule ){ return find(c_DataPtr<char_t>((char_t*)buff,(uint)strlen(buff)) , frule ); }; 
	int findUnPack( const c_DataPtr<char_t> & buff , irule_t & frule ); // return size of cmd, for bad - return 0 
	int findUnPack( const char * buff , irule_t & frule ){ return findUnPack(c_DataPtr<char_t>((char_t*)buff,(uint)strlen(buff)) , frule ); }; 

	struct cStats{	int states , transitions , transitions_unuse;	};
	cStats getstats();

	struct cUserRuleInfo{ virtual void PutRuleInfo( const thistype & slf, irule_t rule , stringA & buff )=0;};
	struct cOptionGeneration{ 
		enum {CHARS , HEX};
		int opt; const char* charformat ;
		char separator;
		cUserRuleInfo * urinf;
		cOptionGeneration(int _opt=CHARS) { separator='\n'; opt=_opt; charformat=(opt==CHARS) ? "%c" : " %02x"; urinf=0; };
	};
	static cOptionGeneration GenASCI() { return cOptionGeneration(cOptionGeneration::CHARS); };
	static cOptionGeneration GenHEX() { return cOptionGeneration(cOptionGeneration::HEX); };
	struct cResultGeneration{	stringA txt ,comments; int cntwords,loops;cResultGeneration() {cntwords=0; loops=0;};	};
	friend struct cGenerationCtx;
	cResultGeneration Generation( cOptionGeneration opg );
	template <class UserFunc> struct cURInfo : public cUserRuleInfo { UserFunc F; void PutRuleInfo( const thistype & slf, irule_t rule , stringA & buff ) { F(slf,rule,buff); }; };
	template <class UserFunc> cResultGeneration Generation( cOptionGeneration opg , UserFunc F){
		cURInfo<UserFunc> URInfo;URInfo.F=F;
		opg.urinf= &URInfo;
		return Generation(opg);
	};
}; // class c_DFA_NativeCode

typedef c_DFA_NativeCode_Base c_DFA_NativeCode;



}; // namespace tbgeneral 