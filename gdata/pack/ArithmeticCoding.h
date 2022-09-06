#pragma once

#include <map>
#include <gdata/t_array.h>
#include <gdata/t_string.h>
#include <gdata/t_sstruct.h>
#include <gdata/tb_map.h>
#include <gdata/t_error.h>
#include <gdata/tb_buffer.h>

#define _INFERNO_T_TEXT(CHAR_TYPE, STR) ((sizeof(CHAR_TYPE) == 1)?reinterpret_cast<const CHAR_TYPE*>(STR):reinterpret_cast<const CHAR_TYPE*>(L ## STR))

namespace tbgeneral{

template <class TChar, class TFreq> struct r_symbol_freq_m { 
	TChar ch; 
	TFreq freq; 
	template <class T_Char, class T_Freq> r_symbol_freq_m( const r_symbol_freq_m<T_Char,T_Freq> & m  ) {
		ch = m.ch; freq=m.freq;
	}
	template <class T_Char, class T_Freq> r_symbol_freq_m( const T_Char _ch, const T_Freq _freq  ) {
		ch = _ch; freq=TFreq(_freq);
	}
	r_symbol_freq_m() { ch=0; freq=0; }
};

template <class TChar> using r_symbol_freq = r_symbol_freq_m<TChar,float>;



struct c_arithmetic_coding_base;
using ref_arithmetic_coding = shared_ptr<c_arithmetic_coding_base>;

struct c_arithmetic_coding_base {
	enum {
	eArithmeticCodingBase = 200200,
	ecErrEOFOnDecode = eArithmeticCodingBase + 1,
	ecErrSequenceOfSpecialSymbol = eArithmeticCodingBase +2,
	ecErrAlphaDuplicateChar =eArithmeticCodingBase +3 , 
	ecErrUnknownCharacter= eArithmeticCodingBase +4 ,
	ecErrDuplicateTerminal=eArithmeticCodingBase +5, 
	ecErrRangeError=eArithmeticCodingBase +6, 
	ecErrTransitionNotSet=eArithmeticCodingBase +7, 
	ecErrABInvalidJsonFromat=eArithmeticCodingBase +8, 
	eAC_BigEndgramm = eArithmeticCodingBase + 9,
	eAC_Badchar = eArithmeticCodingBase + 10,
	eAC_BadIntSequence = eArithmeticCodingBase + 11,
	eAC_InvalidAlphabetIndex = eArithmeticCodingBase + 12,
	eAC_InvalidWideChar = eArithmeticCodingBase + 13,
	eAC_UnknownJsonChar = eArithmeticCodingBase + 14,
	eAC_SerializeError = eArithmeticCodingBase + 15,
	eAC_DeSerializeError = eArithmeticCodingBase + 16,
	eAC_AlphabetNotReady = eArithmeticCodingBase + 17,
	eAC_DeSerCrcError = eArithmeticCodingBase + 18,
	eAC_DeSerAlphaCrcError = eArithmeticCodingBase + 19,
	eAC_JsonNotFoundNode = eArithmeticCodingBase + 20,

	ecErrACNothing=1000, 
	};

	// вырезает начальные пробелы некоторых последовательностей ng2. Например те, которые начинаются с пробела и закачиваются знаками препинания или концом строки
	static stringW DefaultIgnoreNG2sequence( const stringW & s);
	// Возсращает список 2-х символов ng2 последовательности " ,.!:?;-\n\a	"
	static stringA DefaultIgnoreNG2sequence_List();

	// public
	typedef uint32 t_code;
	typedef int t_char; // символ
	typedef int c_char; // код символа
	typedef int i_alpha; // номер алфавита 
	using thistype = c_arithmetic_coding_base;
	enum {  
		/*
		CODE_LEN=30, // 31
		MAX_tcode= (t_code(1)<<CODE_LEN)-1,
		MASK_tcode= (t_code(1)<<CODE_LEN)-1,
		Top_value=MAX_tcode,
		Max_frequency = (1 << (CODE_LEN-2) )-1,

		First_qtr = (Top_value/4+1),  // Конец пеpвой чеpвеpти
		Half =      (2*First_qtr),    // Конец пеpвой половины
		Third_qtr=  (3*First_qtr),
		*/
		//eEOFchar = 0
		DEF_CODE_LEN = 16, //hfpvth rjljdjuj ghjcnhfycndf d ,bnfp
		Def_Min_frequency = 5,

		eErrorChar = 0,
		eTerminalChar = 0,
		eErrorCode = -1
	};

	struct t_scale_vars {
		t_code	CODE_LEN; 
		t_code	MASK_tcode, Top_value, Max_frequency , Min_frequency; 
		t_code  First_qtr; // Конец пеpвой чеpвеpти
		t_code	Half;		// Конец пеpвой половины
		t_code Third_qtr; 

		void set_CodeLen(uint cl){
			CODE_LEN= cl;
			MASK_tcode= (t_code(1)<<CODE_LEN)-1;
			Top_value=(t_code(1)<<CODE_LEN)-1;
			Max_frequency = (t_code(1) << (CODE_LEN-2) )-1;
			Min_frequency = Def_Min_frequency;

			First_qtr = (Top_value/4+1);  // Конец пеpвой чеpвеpти
			Half =      (2*First_qtr);    // Конец пеpвой половины
			Third_qtr=  (3*First_qtr);
		}
		t_scale_vars() { set_CodeLen(DEF_CODE_LEN); }
	} sv;

	parray<t_code> wfreqs; // рабочая таблица частот
	//int error_id;
	t_error_info error;
	int err() { return error.error_id; }

	struct t_work_values{
		int bits_to_follow=0;
		t_code cLow=0,cHigh=0;	// for encode & decode
		t_code cValue=0; // for decode
		t_bitstream bitstream;
		const void* indata_current=0;

		t_work_values() {}; //{ RESETMEM(*this); }
		void init( t_code _high ) { *this = t_work_values();	cHigh = _high;	}
	} ;
	t_work_values wv;
	void prepare_work_values() {	wv.init( sv.Top_value ); };

	t_code alignRate( double v ) const;

	//typedef char Char;


	c_arithmetic_coding_base();
	virtual ~c_arithmetic_coding_base(){};

	//c_char char2code( t_char c , i_alpha & ia );
	//t_char code2char( c_char c , i_alpha ia );
	//t_char code2char( c_char c ); // с текущим алфавитом


	/*
	virtual darray<c_char> tchar2code( const darray<t_char> & ca  )=0; 
	virtual darray<t_char> code2tchar( const darray<c_char> & ca )=0; 


	template <class TStr> darray<c_char> string2code( const TStr & s ){
		prepare_alphabet_set();
		darray<t_char> r; r.resize(s.size());
		int i=0; FOREACH( x ,s  ) { r[i] = *x; i++; } 
		return tchar2code( r );
	}
	template <class TStr> TStr code2string( const darray<c_char> & ca ){
		prepare_alphabet_set();
		auto cha = code2tchar( ca );
		TStr s; s.resize(cha.size()); 
		int i=0; FOREACH( x, cha ) { s[i]=*x; i++; };
		return s;
	}
	*/
	template <class TString> TString testUnknownChars(const TString & s) {
		prepare_alphabet_set();
		TString res;
		FOREACH( pc, s )
			if (!char_exists(*pc) ) res<<*pc;
		return res;
	}
	template <class TString> TString IgnoreUnknownChars(const TString & s) {
		prepare_alphabet_set();
		TString res; res.reserve(s.size());
		FOREACH( pc, s )
			if (char_exists(*pc) ) res.push_back(*pc);
		return res;
	}


	//darray<byte> encode( const darray<c_char> & input );
	//darray<c_char> decode( const  darray<byte> & input );

	protected:
	virtual bool is_eof_char( c_char symbol ){ return true;};
	virtual int decode_tchar( c_char ic, t_char & tc ){return -1;}; 
	virtual int encode_tchar( t_char tc , c_char & cc ){ return -1; }; 
	virtual int encode_endchar( c_char & cc ){ return -1;}; 
	protected:
	template <class TString,class TStorage>  int b_encode_algo( const TString & d , TStorage & stor , t_work_values & wv ){
		c_char cc;
		FOREACH( pc, d ) {
			wv.indata_current = pc;
			while (!err()) {
				auto er= encode_tchar( *pc ,cc );
				if (er>=0) stor.push(wv, cc);
				if (er<=0) break;
			}
			if (err()) break;
		}
		while(!err()) {auto er= encode_endchar(cc); if (er>=0) stor.push( wv, cc); if (er<=0) break; }
		return err();
	}
	template <class TString, class TStorage>  TString decode_algo( size_t needsz , TStorage & stor , t_work_values & wv ){
		//typename TString::value_type;
		//TString::value_type chr;
		t_char val;
		TString res; res.reserve( needsz );
		while(err()==0) { 
			auto c= stor.get(wv);
			if ( is_eof_char(c) ) break;
			while (!err()) {
				auto er=decode_tchar( c, val ); 
				if (er>=0) res.push_back( typename TString::value_type(val));
				if (er<=0) break;
			}
			if ( stor.eof(wv)) { pusherror(ecErrEOFOnDecode); break; }
		}
		return res;
	}
	public:
	struct r_extrainfo_encode { size_t index; c_char cc; float weight; };
	template <class TString>  darray<r_extrainfo_encode> extra_encode_string( const TString & indata ){
		using ptr_char = const typename TString::value_type *;
		struct t_edc {  
			thistype * ac; 
			ptr_char p_start;
			darray<r_extrainfo_encode> resa;
			void push( t_work_values & wv , c_char cc ) { 
				resa.push_back( r_extrainfo_encode{ size_t(ptr_char(wv.indata_current) -  p_start) , cc , (float) ac->getchar_weight(cc) } );
		};};
		t_edc call{this , indata.begin()};
		encode_start();
		b_encode_algo<TString>(indata, call , wv  );
		encode_end();
		return call.resa;
	}
	template <class TString>  darray<byte> encode_string( const TString & indata ){
		struct t_dc {  
			thistype * ac; 
			void push( t_work_values & wv , c_char cc ) { ac->encode_char(cc); }
		};
		t_dc call{this};
		encode_start();
		b_encode_algo<TString>(indata, call , wv  );
		encode_end();
		return wv.bitstream.data();
	}
	template <class TString> darray<c_char> string2code( const TString & indata ){
		struct t_buff {  
			darray<c_char> b; 
			void push( t_work_values & wv , c_char cc ) { b.push_back(cc); }
		};
		prepare_encode_decode();
		t_buff call; call.b.reserve(indata.size());
		b_encode_algo<TString>(indata, call , wv  );
		return call.b;
	}
	darray<c_char> tchar2code( const darray<t_char> & ca  ){
		return string2code( ca );
	}; 

	template <class TString>  TString decode_string( const darray<byte> & input ){
		struct t_dc {  
			thistype * ac; 
			c_char get( t_work_values & wv ) { return ac->decode_char(); }
			bool eof( t_work_values & wv ) { return wv.bitstream.eof( ac->sv.CODE_LEN ); }
		};
		t_dc b; b.ac=this; 
		decode_start( input );
		return decode_algo<TString,t_dc>( input.size() , b , wv );
	}
	template <class TString> TString code2string( const darray<c_char> & ca ){
		struct t_buff {  
			parray<c_char> pa;
			c_char get(t_work_values& wv) { if (!pa.size()) return 0; auto v = pa[0]; pa.shift_begin(+1); return v;  }
			bool eof( t_work_values & wv ) { return pa.size()<=0; }
		};
		t_buff b; b.pa = ca;
		prepare_encode_decode();
		return decode_algo<TString,t_buff>( ca.size() ,b , wv );
	}
	darray<t_char> code2tchar( const darray<c_char> & ca ){
		return code2string<darray<t_char>>( ca );
	}

	/*
	template <class TString>  darray<byte> encode_string( const TString & d ){
		return encode( string2code( d ) );
	}
	template <class TString>  TString decode_string( const darray<byte> & d ){
		return code2string<TString>( decode(d) ); 
	}
	*/

	protected:
	bool alpha_ready; // алфавиты подготовленны. Используется только в дочерних классах
	public:


	//bool lastsym_isspecial;

	virtual bool char_exists(t_char c) const { return false; } // exists(char2alpha, *pc)
	virtual void prepare_alphabet_set(  ){}; // после добавления всех алфавитов
	virtual void prepare_encode_decode();

	void encode_start();
	void encode_char( c_char symbol );
	double getchar_weight( c_char symbol );
	void encode_end();

	void decode_start( const  darray<byte> & input );
	c_char decode_char();
	void decode_end( ) { }
	void bit_plus_follow(int bit);

	//virtual void update_model(c_char symbol){}; 
	void pusherror( int e ); 


}; // c_arithmetic_coding_base

struct c_arithmetic_coding_c : c_arithmetic_coding_base {
	// public
	using rchar_freq = r_symbol_freq_m<t_char,double>;
	darray<rchar_freq> freqs; // входной массив частот

	std::map<t_char,c_char> char2code; 
	darray<t_char> code2char;

	template <class TChar, class TFreq> void set_freqs( const r_symbol_freq_m<TChar,TFreq> * sa , size_t count=0 ){
		rchar_freq nr;
		alpha_ready=false;
		freqs.resize(0);freqs.reserve(count);
		for (auto s=sa,e=sa+count; s<e;s++ )  {
			nr.ch = s->ch; nr.freq = s->freq;
			if (!nr.ch) break;
			freqs.push_back(nr);
		}

	};
	template <class TString> void int_freq_equiprobable( const TString & fulllist ) {
		alpha_ready=false;
		rchar_freq chfr;
		freqs.resize(0);freqs.reserve(fulllist.size());
		//freqs.push_back( rchar_freq{} );
		for (uint i=0;i<fulllist.size();i++) {
			chfr.ch = fulllist[i];
			chfr.freq = 1;
			freqs.push_back( chfr );
		};
	};

	bool char_exists(t_char c) const override { return exists( char2code , c ); } // exists(char2alpha, *pc)
	void prepare_alphabet_set(  ) override ; // после добавления всех алфавитов

	bool is_eof_char( c_char symbol ) override { return eTerminalChar==symbol;};
	int decode_tchar( c_char ic, t_char & tc ) override; 
	int encode_tchar( t_char tc , c_char & cc ) override; 
	int encode_endchar( c_char & cc ) override; 

	//darray<c_char> tchar2code( const darray<t_char> & ca  )  override ; 
	//darray<t_char> code2tchar( const darray<c_char> & ca )  override ; 
	//void update_model(c_char symbol){}; 


}; // struct c_arithmetic_coding_c

}; // namespace tbgeneral{


namespace tbgeneral{




struct t_alphabet_transit{ 
		typedef int t_id; // non zero 
		typedef t_alphabet_transit t_transit;
		t_id to_id; double rate;
		void assign( const t_transit & t) { *this=t; }
		template <class Transit> void assign( const Transit & t) { to_id=t.ch;rate = t.freq; }
	};

template <class TChar, class TFreq> struct r_alphabet_extrainfo{
		typedef TChar Chartype;
		typedef TFreq Freqtype;
		using t_id = t_alphabet_transit::t_id; // non zero 
		using d_rates = darray <r_symbol_freq_m<TChar,TFreq>>;
		//typedef r_symbol_freq_m<t_id,double> t_transit;
		using t_transit = t_alphabet_transit;
		/*struct t_transit{ t_id to_id; double rate;
			void assign( const t_transit & t) { *this=t; }
			template <class Transit> void assign( const Transit & t) { to_id=t.ch;rate = t.freq; }
		};*/
		using d_transit = darray<t_transit>;
		int id; // необходим при заполнении массива переходов на другие алфавиты transit
		d_rates rates;
		double weight; // вес относительно других алфавитов / среднее количество символов до смены алфавита
						// параметр не задается, если заданы id & transit
		bool OneSymbolPerWord; // после одного символа алфавита, произойдет переключение на другой алфавит
		d_transit transit;
		//t_transit transit[20];	uint transit_sz;

		r_alphabet_extrainfo(){ ZEROMEM(*this); };
		r_alphabet_extrainfo(int _id, const d_rates & _rates, double _weight,  bool _OneSymbolPerWord , const t_transit* ta=0 , int t_sz=0 ){ 
			ZEROMEM(*this);
			id= _id; rates= _rates; weight= _weight; OneSymbolPerWord = _OneSymbolPerWord;
			init_c_transit( ta , t_sz);
		};
		void init_c_transit(const t_transit* ta=0 , int t_sz=0 ) {
			if (!ta) return;
			if (!t_sz) { for(auto p=ta;p->to_id!=0;p++,t_sz++); }
			assign_transit(ta , uint(t_sz));
		}
		template <class NChar,class NFreq> void assign(const r_alphabet_extrainfo<NChar,NFreq> & e) {
			id= e.id; 
			weight = e.weight;
			OneSymbolPerWord = e.OneSymbolPerWord;
			//memcpy( transit , e.transit , sizeof(transit));	transit_sz= e.transit_sz;
			//assign_tarnsit(e.transit.data(), e.transit.size());
			transit = e.transit; transit.makeunique();
			rates.resize(e.rates.size());
			for (uint i=0;i<rates.size();i++)
				rates[i] = r_symbol_freq_m<TChar,TFreq>( e.rates[i] );
		}
		//template <class NChar,class NFreq> void assign_tarnsit(const r_symbol_freq_m<NChar,NFreq> * ta , uint cnt) {
		template <class Transit> void assign_transit(const Transit * ta , uint cnt) {
			//transit_sz = cnt;
			transit.resize(cnt);
			for (uint i=0;i<transit.size();i++) {
				transit[i].assign( ta[i]);
				//transit[i].to_id = ta[i].ch;	transit[i].rate =  ta[i].freq;
			};
		}
};

struct c_arithmetic_coding_m : c_arithmetic_coding_base {
	// public



	//typedef char Char;
	using r_input_sf = r_symbol_freq_m<t_char,double>;
	using r_alphabet_ei = r_alphabet_extrainfo<t_char,double>;

	struct r_alphabet{
		darray<t_code> wfreqs; // рабочая таблица частот

		std::map<t_char,c_char> char2code; 
		darray<t_char> code2char;

		darray<i_alpha> code2alpha; // таблица переходов между алфавитами 
		darray<c_char> alpha2code;
		c_char terminal_char; 
		i_alpha index;

		r_alphabet_ei ei;
		
		//darray<r_input_sf> in_freqs; // начальная таблица частот
		//double weight;
		//double middlewordlen;  // среднее количество символов до смены алфавита
		//bool OneSymbolPerWord;
		r_alphabet();
	};

	std::map<t_char,i_alpha> char2alpha; 
	darray<r_alphabet> alphabet_s; // множество алфавитов 
	r_alphabet * start_alpha;
	i_alpha alpha_has_terminal_char;

	c_arithmetic_coding_m();


	void add_alphabet(  const r_alphabet_ei & exi );

	template <class TChar, class TFreq> void add_alphabet( const r_alphabet_extrainfo<TChar,TFreq> & exi ){
		r_alphabet_ei nei;
		nei.assign( exi );
		add_alphabet( nei );
	};




	protected:
	public:

	r_alphabet * curr_alpha; // рабочий алфавит
	r_alphabet * prev_alpha; // предыдущий алфавит
	i_alpha next_alfa;
	bool lastsym_isspecial;
	//c_char alpha_count; 



	bool is_eof_char( c_char symbol ) override;
	bool char_exists(t_char c) const override { return exists(char2alpha,c); };
	void prepare_alphabet_set(  ) override; // после добавления всех алфавитов
	void prepare_encode_decode() override;

	//darray<c_char> tchar2code( const darray<t_char> & ca  ) override; 
	//darray<t_char> code2tchar( const darray<c_char> & ca ) override; 


	//void update_model(c_char symbol) override; 
	void set_alphabet( i_alpha index , bool ispush=true  );

	protected:
	int decode_tchar( c_char ic , t_char & tc ) override; 
	int encode_tchar( t_char tc , c_char & cc ) override;  
	int encode_endchar( c_char & cc  ) override; 

}; // c_arithmetic_coding_m

}; // namespace tbgeneral{
