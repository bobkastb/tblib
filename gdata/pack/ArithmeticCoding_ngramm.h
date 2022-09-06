#pragma once
#include "gdata/pack/ArithmeticCoding.h"
#include "conf/json_parser.h"
#include <gdata/tb_map.h>
#include <gdata/tb_serialize.h>

namespace tbgeneral{

	struct t_ngramm_seq{
		using vtype = uint;
		vtype base , max_degree , start_offs ;
		vtype n_degree , n_offs , n_curr;
		bool is_degree0() { return n_degree==1; }
		int initbyval(vtype val ) {
			reset();
			while ( val - n_offs >= n_degree ) { n_offs+=n_degree; n_degree*=base;  }
			n_curr = val - n_offs;
			return 0;
		}
		size_t toChars( darray<int> & da  ){
			int data[32]; auto st=&data[0];
			auto c = n_curr ,  d= n_degree; // o = n_offs,
			for (;d>1;st++,d/=base,c/=base) { st[0]= c%base + start_offs; }
			auto sz = st-data;  da.resize(sz);
			for (auto ds=da.data() ;st>data; ds++ ) {st--; *ds=*st; }
			return sz;
		}
		vtype next(  vtype val  ) { 
			val -= start_offs;
			if (n_degree<max_degree) {
				n_offs+=n_degree; 
				n_degree*=base; 
			}
			n_curr = ( n_curr * base + val ) % max_degree;  
			return n_curr+n_offs; }
		vtype down_degree() {
			if (n_degree==1) { return 0; }
			n_degree/=base; 
			n_offs -=n_degree;
			n_curr %= n_degree;  
			return n_curr+n_offs;
		}
		t_ngramm_seq() { base = 1; max_degree=1; n_degree=1; n_offs=0; n_curr=0; start_offs=0; }
		t_ngramm_seq(vtype _base , vtype _max_degree , vtype _stoffs){ 
			base = _base; max_degree=_max_degree; start_offs = _stoffs;
			n_degree=1; n_offs=0; n_curr=0;
		};
		void reset() { n_degree=1; n_offs=0; n_curr=0; }
		vtype value() { return n_curr + n_offs; }
	};

	struct r_alphabet_ngramm{
		using t_freq = double;
		using  i_error = int;
		using  t_code = c_arithmetic_coding_base::t_code; // вероятность
		using  t_char = c_arithmetic_coding_base::t_char; // гловабльный код символа
		using  c_char = c_arithmetic_coding_base::c_char; // локальный код символа 
		using  i_alpha = c_arithmetic_coding_base::i_alpha; // идентификатор алфавита 
		using  i_ngrammseq = uint32;  //кодирование последовательности символов алфавита
		using  i_line = int;
		enum {	eBad_ngrammseq= i_ngrammseq(-1), 
				eBad_c_char= c_char(-1), 
				eBad_t_char= t_char(-1),
				eBad_i_alpha= i_alpha(-1),
				eBad_i_line= i_line(-1),
				eTerminalCharCode = 0
		};
		enum etypeChar { eCharIsTerminal=0,eCharIsAlphabet,eCharIsSymbol };
		struct t_text_ctx{
			enum { eAsWchar=1, eAsInt=2 };
		};
		t_error_info error;
		bool freqs_ready;
		stringA name;
		int ngramm; // max ngramm
		int ngramm_seq_maxsize; // max(i_ngrammseq)+1 :: размер i_ngrammseq. Количество символов алфавита в степени ngramm
		int ngramm_fullsize;
		bool OneSymbolPerWord;
		bool fullngramm; // not use f_seq2lines
		int c_alphabets; // количество алфавитов 
		int c_alpha_size; // количество символов алфавита 
		int c_code_size; // полное количество символов алфавита +кол-во алфавитов, +терминальный символ
		t_ngramm_seq start_seq;
		darray<t_freq> save_init_freqs_summ; // оригинальные суммы
		darray<t_freq> init_freqs; //начальная таблица частот 
		darray<t_code> wfreqs; // рабочая таблица частот , каждая строка размером (c_code_size+1)
			// структура строки [ <terminal_char>, <алфавиты>[1..c_alphabets] , <символы>[c_alpha_size] , <summ all> ]
		std::map<i_ngrammseq,i_line> f_seq2lines;

		size_t wfreqs_strsize() const {return c_code_size+1;} // в первом слоте всегда 0, в последнем слоте - сумма долей
		size_t init_freqs_strsize()const {return c_code_size;} 
		parray<t_code> get_algoline(i_line line) const { auto sz=wfreqs_strsize(); return wfreqs.slice( line*sz, sz ); }
		parray<t_freq> get_line(i_line line) { auto sz=c_code_size; return init_freqs.slice( line*sz, sz ); }
		parray<t_code> find_algoline(i_ngrammseq seq) { i_line l= fullngramm? seq : f_seq2lines[seq];  return get_algoline(l);  }
		parray<t_freq> find_line(i_ngrammseq seq);
		bool find_algoline(t_ngramm_seq & sseq , parray<t_code> & res );
		bool bfind_algoline(i_ngrammseq seq , parray<t_code> & res );
		bool bfind_line(i_ngrammseq seq , parray<t_freq> & res );
		c_char first_alpha() const  { return 1; } 
		c_char first_char() const { return first_alpha()+c_alphabets; } 
		etypeChar whatischar(c_char cc) { return cc<first_alpha() ? eCharIsTerminal : cc<first_char() ? eCharIsAlphabet : eCharIsSymbol; }


		// структуракодов [ <terminal_char>, <алфавиты>[1..c_alphabets] , <символы>[c_alpha_size]  ]
		std::map<t_char,c_char> char2code; 
		darray<t_char> code2char;
		size_t count_chars() { return char2code.size(); }

		darray<i_alpha> code2alpha; // таблица кодов алфавитов 
		darray<c_char> alpha2code;
		//size_t count_alpha() { return alpha2code.size(); }

		c_char terminal_char; // если  >=0 
		i_alpha id_alpha; //идентификатор алфавита. [1..count_alpha()]
		// ------------------- worker -----------------------
		bool has_terminal() { return terminal_char>=0; }
		// ------------------- getter -----------------------
		parray<t_char> get_t_chars(){ auto fc=first_char(); return code2char.slice(fc ); }

		// ------------------- maker ------------------------
		void initialize(i_alpha _id , int ngramm, int alpha_count, int chars_count);
		void initialize(i_alpha _id, int ngramm, int alpha_count, const stringW & chars);
		//void initialize(i_alpha _id, int ngramm, int alpha_count, const partstringA & chars) { initialize( _id, ngramm, alpha_count, stringW(chars) ) };
		void initialize(i_alpha _id, int ngramm, int alpha_count, const parray<t_char> & arr);
		void set_id_alpha( i_alpha id ) { id_alpha= id; }
		i_error make_seq_id(i_ngrammseq& res, const t_char * arr , int count);
		i_error make_seq_id(i_ngrammseq& res, const stringW & chars);
		parray<t_freq> get_line(const stringW & seqs);
		t_freq get_init_summ( i_ngrammseq seq );
		parray<i_ngrammseq> get_all_ngrammseq(  );
		template<class TValue>int init_set_chars_trans( i_ngrammseq seq , const parray<TValue> & chars ){
			freqs_ready=false;
			auto ld= find_line(seq); 
			auto st= first_char(); 
			for (int i=0; i<c_alpha_size;i++)  ld[i+st]=0;
			auto sz = std::min<int>(static_cast<int>(chars.size()) , c_alpha_size  );
			for (int i=0; i<sz;i++)  ld[i+st]= chars[i];
			return 0; // TODO ERR
		}; 
		int init_set_fin_weigth( double w ); // w=[0..1/20]
		template<class TValue>int init_set_fin_trans( i_ngrammseq seq , TValue fin ){
			freqs_ready=false;
			auto ld= find_line(seq); 
			if ( has_terminal()) ld[terminal_char] = fin;
			return 0; // TODO ERR
		}; 
		template<class TValue>int init_set_alpha_trans( i_ngrammseq seq , const parray<TValue> & alphas  ){
			freqs_ready=false;
			auto ld= find_line(seq); auto st= first_alpha(); 
			for (int i=0; i<c_alphabets;i++)  ld[i+st]=0;
			auto sz = std::min<int>(static_cast<int>(alphas.size()) , c_alphabets  );
			for (int i=0; i<sz;i++)  ld[i+st]= alphas[i];
			return 0; // TODO ERR
		}; 
		template<class TValue>int init_set_trans( i_ngrammseq seq , TValue fin, const parray<TValue> & alphas,const parray<TValue> & chars  ){
			init_set_fin_trans(seq,fin);
			init_set_alpha_trans(seq,alphas);
			init_set_chars_trans(seq,chars);
			return 0; // TODO ERR
			
		}; 
		int init_finalize( const c_arithmetic_coding_base & ac );
		// ----------------
		int savetojson( conf_struct_ref jn , uint saveopt );
		int loadjson(conf_struct_ref jr  );
		int serialize( t_serializer & ser );
		int deserialize( t_serializer & ser );

		r_alphabet_ngramm();

		public:
		size_t get_wfreqs_size() { return ngramm_fullsize * wfreqs_strsize(); };
		size_t get_ifreqs_size() { return ngramm_fullsize * init_freqs_strsize(); };
	};

	t_serializer & operator << (t_serializer & st , r_alphabet_ngramm v);
	t_serializer & operator >> (t_serializer & st , r_alphabet_ngramm & v);


	struct c_arithmetic_coding_ngramm : c_arithmetic_coding_base {
		static const double default_EndCharWeigth; //= 1.0/2000;
		enum { Minimum_Frequency=1 };
		using r_alphabet = r_alphabet_ngramm;

		// -------------- static variables
		std::map<t_char,i_alpha> char2alpha; 
		darray<r_alphabet> alphabet_s; // множество алфавитов 
		i_alpha start_alpha; 
		i_alpha alpha_has_terminal_char;




		//----------------------- work variables
		r_alphabet * curr_alpha; // рабочий алфавит
		r_alphabet * prev_alpha; // предыдущий алфавит
		i_alpha next_alfa;
		parray<t_code> next_wfreqs;
		t_ngramm_seq ng_seq;
		bool lastsym_isspecial;

		//------------------------
		bool isready(); 
		r_alphabet* get_alpha( i_alpha id_alpha ) { return &alphabet_s[alphaid2index(id_alpha)]; }
		int add_alphabet( r_alphabet * na );
		void prepare_alphabet_set(  ) override; // после добавления всех алфавитов
		bool is_eof_char( c_char symbol ) override;
		bool char_exists(t_char c) const override { return exists(char2alpha,c); };
		void prepare_encode_decode() override;
		int init_set_fin_weigth( double w ); // w=[0..1/20]
		c_arithmetic_coding_ngramm();

		enum eBJsonOption{ eBJO_saverateAsMap=1 , eBJO_UseDefaultSumm=0x10, eBJO_FloatTransition =0x20 , 
			eBJO_WChar =0x40 , eBJO_UseInputData=0x80 ,

			//eBJO_DefaultSumm=1000000, 
			eBJO_DefaultMask =eBJO_saverateAsMap | eBJO_WChar
		};
		int reset( int alpha_count=0);
		int savetojson(conf_struct_ref jn , uint saveopt=eBJO_DefaultMask );
		int savetojson( json_ctx * jo , const stringA & filenm , uint saveopt=eBJO_DefaultMask );
		int loadjson(conf_struct_ref jr  );
		int loadjson( json_ctx * jo , const stringA & filenm  );
		int serialize( t_serializer & ser );
		int deserialize( t_serializer & ser );

		protected:
		i_alpha  alphaid2index(i_alpha id) { return id-1; }
		void set_alphabet( i_alpha index , bool ispush=true  );
		int decode_tchar( c_char ic , t_char & tc ) override; 
		int encode_tchar( t_char tc , c_char & cc ) override;  
		int encode_endchar( c_char & cc  ) override; 

	};
}; //namespace tbgeneral{ 
