#include "gdata/pack/ArithmeticCoding_ngramm.h"
#include "tsys/tb_log.h"
#include "conf/json_parser.h"
#include "gdata/tb_buffer.h"
#include <cmath>
#include "tsys/tbfiles.h"
#include "gdata/t_format.h"

#include "gdata/tbt_locale.h"
//#include "gdata/pack/ArithmeticCoding_ngramm2.h"
//ArithmeticCoding_ngramm.cpp



//------------------------------------------------
//--------------------- END HEADER ---------------
//------------------------------------------------

namespace tbgeneral{

using  t_code = c_arithmetic_coding_base::t_code; // вероятность
using  c_char = c_arithmetic_coding_base::c_char; // локальный код символа 
using  i_alpha = c_arithmetic_coding_base::i_alpha; // идентификатор алфавита 
using  i_ngrammseq = uint32; // 
using  i_line = r_alphabet_ngramm::i_line;
using  t_freq = r_alphabet_ngramm::t_freq;
using  t_ac_base = c_arithmetic_coding_base;
using i_error=r_alphabet_ngramm::i_error;

const double c_arithmetic_coding_ngramm::default_EndCharWeigth= 1.0/2000;

r_alphabet_ngramm::r_alphabet_ngramm() { 
	initialize(0,0,0,0);
}

void r_alphabet_ngramm::initialize(i_alpha _id, int _ngramm, int _alpha_count, int _chars_count){
	error.reset();
	freqs_ready = false;
	ngramm=_ngramm; // max ngramm
	OneSymbolPerWord = false;
	fullngramm = false; // not use f_seq2lines
	c_alphabets = _alpha_count;
	c_alpha_size=_chars_count; // количество символов алфавита 
	c_code_size=c_alpha_size+c_alphabets+1; // полное количество символов алфавита +кол-во алфавитов, +терминальный символ
	// рабочая таблица частот , каждая строка размером (c_code_size+1)
	// структура строки [ <0>, <terminal_char>, <алфавиты>[1..] , <символы>[c_alpha_size] , <summ all> ]
	ngramm_fullsize =0; for(int i=0,powng=1;i<ngramm;i++,powng*=c_alpha_size) ngramm_fullsize+=powng; 
	ngramm_seq_maxsize = ngramm>0 ? (int)std::pow(c_alpha_size,ngramm-1) : 1;
	fullngramm = ngramm <= 2; 
	wfreqs.resize( ngramm_fullsize * wfreqs_strsize() );
	init_freqs.resize( ngramm_fullsize * init_freqs_strsize() );
	f_seq2lines.clear( );
	start_seq = t_ngramm_seq( c_alpha_size ,  ngramm_seq_maxsize , first_char() );

	char2code.clear(); 
	code2char.resize( c_code_size );

	code2alpha.resize(_alpha_count+1); // таблица переходов между алфавитами 
	alpha2code.resize(_alpha_count+1);
	auto fa = first_alpha();
	for (c_char c=fa; c<fa+_alpha_count; c++) 
		{ code2alpha[c]=c; alpha2code[c]=c; }
	code2alpha[0] = eBad_i_alpha;
	alpha2code[0] = eBad_c_char;
	terminal_char = eTerminalCharCode ; // если  >=0 
	id_alpha=_id; //идентификатор алфавита. [1..count_alpha()]

};


void r_alphabet_ngramm::initialize(i_alpha _id,int _ngramm, int _alpha_count, const stringW & chars){
	t_char arr[1024]; 
	auto pa = array2localstorage( chars.size() , arr );
	//array2localstorage
	auto s=pa.data();
	FOREACH( pc , chars) { *s = *pc; s++; };
	initialize( _id, _ngramm , _alpha_count , pa );
};


void r_alphabet_ngramm::initialize(i_alpha _id,int _ngramm, int _alpha_count, const parray<t_char> & arr){
	initialize(_id,  _ngramm , _alpha_count, static_cast<int>(arr.size()) );
	c_char cc = first_char();
	FOREACH( pc , code2char )  pc[0]= eBad_t_char; 
	FOREACH( pc , arr ) { 
		code2char[cc] = *pc;
		char2code[*pc] = cc;
		cc++;
	};
};




template <class Tarr> i_error fmake_seq_id( const r_alphabet_ngramm & ab, i_ngrammseq & res, const Tarr & arr ){
	int sz = std::min<int>( ab.ngramm , static_cast<int>(arr.size()));
	if (size_t(sz)!=arr.size()) return t_ac_base::eAC_BigEndgramm;
	t_ngramm_seq v= ab.start_seq;
	FOREACH(p,arr) {
		auto cc= get_def( ab.char2code, *p ,c_char(r_alphabet_ngramm::eBad_c_char));
		if (cc== r_alphabet_ngramm::eBad_c_char ) return t_ac_base::eAC_Badchar;
		v.next( cc );
	}
	res= v.value();
	return 0;
}

parray<t_freq> r_alphabet_ngramm::get_line(const stringW & seqs) { 
	i_ngrammseq iseq;
	auto e=make_seq_id(iseq,seqs); if (e) throw e;
	return find_line(iseq ); }

i_error r_alphabet_ngramm::make_seq_id(i_ngrammseq& res, const t_char * arr , int count){
	return fmake_seq_id( *this, res, parray<t_char>( arr , (size_t)count) );
};
i_error r_alphabet_ngramm::make_seq_id(i_ngrammseq& res, const stringW & chars){
	return fmake_seq_id( *this, res, chars);
};


int c_arithmetic_coding_ngramm::init_set_fin_weigth( double w ){
	FOREACH( pa , alphabet_s) {
		if (pa->has_terminal()) 
			pa->init_set_fin_weigth(w);
	}
	return 0;
}
int r_alphabet_ngramm::init_set_fin_weigth( double w ){
	if (!has_terminal()) return 0;
	if (init_freqs.size()==0) return 1;
	auto ifr = init_freqs.data(); auto ssz= init_freqs_strsize(); auto eifr = ifr +init_freqs.size();
	for (; ifr<eifr; ifr += ssz) {
			t_freq sm= 0; for (auto f=ifr;f<ifr+ssz;f++) sm+=f[0];
			ifr[ terminal_char ] = w*sm;
	};
	return 0;
}; 

int r_alphabet_ngramm::init_finalize( const c_arithmetic_coding_base & ac ){
	// преобразование init_freqs к work_freqs
	if (freqs_ready) { return 0;}
	if (init_freqs.size()!=0){
		bool isinit_fsumm = save_init_freqs_summ.size()!=0;
		if (!isinit_fsumm) save_init_freqs_summ.resize(ngramm_fullsize);
		auto ifr = init_freqs.data(); auto eifr = ifr +init_freqs.size();
		auto wfr = wfreqs.data();
		auto ifr_ssz = init_freqs_strsize(); //auto wfr_ssz=wfreqs_strsize();
		i_ngrammseq iseq=0;
		size_t fai=first_alpha(), eai=fai+c_alphabets;
		for (; ifr<eifr; ifr += init_freqs_strsize(), wfr+=wfreqs_strsize() , iseq++ ) {
			t_freq sm= 0; auto w=wfr;
			ifr[fai+id_alpha -1]=0;
			if (OneSymbolPerWord) {
				for (auto f=ifr+fai;f<ifr+eai;f++) f[0]=0;
			}
			for (auto f=ifr;f<ifr+ifr_ssz;f++) sm+=f[0];
			if (sm==0.0) for (auto f=ifr;f<ifr+ifr_ssz;f++) { f[0]=1; sm+=f[0]; }
			if (!isinit_fsumm || save_init_freqs_summ[iseq]==0) save_init_freqs_summ[iseq] = sm;
			*w=0;w++; t_code wsumm=0;
			for (auto f=ifr;f<ifr+ifr_ssz;f++,w++) { 
				auto v = ac.alignRate(double(f[0])/sm);
				//if (v==0) { v=1; }
				wsumm+=v;
				*w=wsumm;
			};
		}
		freqs_ready=true;
	};
	return 0;
};

//----------------

//parray<t_code> find_algoline(i_ngrammseq seq) { i_line l= fullngramm? seq : f_seq2lines[seq];  return get_algoline(l);  }
parray<t_freq> r_alphabet_ngramm::find_line(i_ngrammseq seq){
	i_line l = (seq==0)? 0 : fullngramm ? seq : get_def(f_seq2lines,seq, (i_line) eBad_i_line);
	if (l==eBad_i_line) throw t_ac_base::eAC_BadIntSequence;
	return get_line(l);
};
bool r_alphabet_ngramm::bfind_line(i_ngrammseq seq , parray<t_freq> & res ){
	i_line l = (seq==0)? 0 : fullngramm ? seq : get_def(f_seq2lines,seq, (i_line) eBad_i_line);
	if (l==eBad_i_line) return false;
	res= get_line(l);
	return true;
};

bool r_alphabet_ngramm::bfind_algoline(i_ngrammseq seq , parray<t_code> & res){
	i_line l = (seq==0)? 0 : fullngramm ? seq : get_def(f_seq2lines,seq, (i_line) eBad_i_line);
	if (l==eBad_i_line) return false;
	res= get_algoline(l);
	return true;
};
bool r_alphabet_ngramm::find_algoline(t_ngramm_seq & sseq , parray<t_code> & res ){
	auto iseq = sseq.value();
	while ( !bfind_algoline( iseq , res )) {
		iseq=sseq.down_degree(  );
	};
	return true;
};

//----------------


int c_arithmetic_coding_ngramm::decode_tchar( c_char cc , t_char & tc ) {
	switch ( curr_alpha->whatischar( cc ) ) {
	case r_alphabet::eCharIsAlphabet :{
		if (lastsym_isspecial) { pusherror( ecErrSequenceOfSpecialSymbol ); }
		set_alphabet( curr_alpha->code2alpha[cc] );
		lastsym_isspecial=true;
		return -1;} 
	case r_alphabet::eCharIsSymbol:{
		lastsym_isspecial=false;
		tc = curr_alpha->code2char[ cc ];
		if (curr_alpha->OneSymbolPerWord) 
			set_alphabet( prev_alpha->id_alpha, false );
		else {
			ng_seq.next(  cc  );
			curr_alpha->find_algoline(ng_seq,wfreqs);
		}
		return 0;}
	default:  
		pusherror( ecErrSequenceOfSpecialSymbol );
		return -1;
	};
	return -1;
}; 
int c_arithmetic_coding_ngramm::encode_tchar( t_char tc , c_char & cc ) {
	if ((next_alfa != curr_alpha->id_alpha  ) && (next_alfa>=0)) {
		set_alphabet( next_alfa , false );
	} else {
		wfreqs = next_wfreqs;
	}
	auto c= get_def( curr_alpha->char2code, tc , t_char( eErrorCode ) );
	if (c!=eErrorCode) {
		cc = c;
		if ( curr_alpha->OneSymbolPerWord ) 
			next_alfa= prev_alpha->id_alpha;
		else {
			ng_seq.next( cc );
			curr_alpha->find_algoline(ng_seq,next_wfreqs);
		}
		return 0;
	} else {
		auto ai = get_def( char2alpha , tc , -1 );
		if (ai<=0 ) { 
			pusherror(ecErrUnknownCharacter); 
			return -1;
		} else {
			cc = curr_alpha->alpha2code[ai];
			prev_alpha = curr_alpha;
			next_alfa= ai;
		}
		return 1;
	}
};  

int c_arithmetic_coding_ngramm::encode_endchar( c_char & cc  ){
	if ((next_alfa != curr_alpha->id_alpha) && (next_alfa>=0)) {
		set_alphabet( next_alfa , false );
	} else {
		wfreqs = next_wfreqs;
	}
	if (!curr_alpha->has_terminal()) { 
		auto ai= alpha_has_terminal_char; 
		cc  = curr_alpha->alpha2code[ai];
		next_alfa = ai;
		return 1;
	}
	cc= curr_alpha->terminal_char;
	return 0;
}; 


void c_arithmetic_coding_ngramm::set_alphabet( i_alpha id_alpha , bool ispush ){
		if (ispush ) prev_alpha = curr_alpha;
		curr_alpha = get_alpha(id_alpha );
		next_wfreqs = wfreqs = curr_alpha->get_algoline(0); 
		ng_seq = curr_alpha->start_seq;
}

void c_arithmetic_coding_ngramm::prepare_encode_decode(){
	//error_id = 0;
	c_arithmetic_coding_base::prepare_encode_decode();
	//if (!start_alpha) pusherror(11);
	lastsym_isspecial = false;
	next_alfa =  start_alpha;
	prev_alpha=curr_alpha= get_alpha( start_alpha );
	set_alphabet( start_alpha ); // set wfreqs &  prev_alpha
};
bool c_arithmetic_coding_ngramm::is_eof_char( c_char symbol ){
		return symbol == r_alphabet::eTerminalCharCode;
};


void c_arithmetic_coding_ngramm::prepare_alphabet_set(  ) {
	//TODO: check!
	if ( alpha_ready) return;
	FOREACH( pa , alphabet_s ) {
		pa->init_finalize( *this );
	}
	for( auto pa=alphabet_s.end()-1; pa>=alphabet_s.begin(); pa--  ) {
		if ( pa->has_terminal() ) 
			alpha_has_terminal_char = pa->id_alpha;
		FOREACH( pp, pa->char2code) {
			char2alpha[pp->first] = pa->id_alpha;
		}
	}
	//	std::map<t_char,i_alpha> char2alpha; 
	start_alpha = alpha_has_terminal_char; 
	alpha_ready = true;
};

int c_arithmetic_coding_ngramm::add_alphabet( r_alphabet * na ){
	alpha_ready = false;
	auto index= alphaid2index( na->id_alpha );
	if (index<0 || (index>= na->c_alphabets)) return error.pushf( t_ac_base::eAC_InvalidAlphabetIndex,"");
	if (na->c_alphabets>(int)alphabet_s.size()) alphabet_s.resize(na->c_alphabets);
	auto pa =  get_alpha(na->id_alpha);
	*pa = *na;
	return 0;
};

c_arithmetic_coding_ngramm::c_arithmetic_coding_ngramm(){
		//char2alpha 
		sv.Min_frequency = Minimum_Frequency;
		alphabet_s.reserve(32);
		start_alpha= r_alphabet::eBad_i_alpha; 
		alpha_has_terminal_char= r_alphabet::eBad_i_alpha; 

		//----------------------- work variables
		curr_alpha=0;
		prev_alpha=0;
		next_alfa = r_alphabet::eBad_i_alpha;
		//next_wfreqs;
		//t_ngramm_seq ng_seq;
		lastsym_isspecial= false;
};
int c_arithmetic_coding_ngramm::reset( int alpha_count){
	*this = c_arithmetic_coding_ngramm();
	alphabet_s.resize(alpha_count);
	return 0;
};


}; //namespace tbgeneral{ 

namespace tbgeneral{ 


	/*
struct rab2text_transformer_base {
	virtual partstringA symbol2str( int symbol )=0;
	virtual bool is_link_strings( )=0;
};
struct rab2text_transformer_int : rab2text_transformer_base {
	partstringA symbol2str( int symbol ) override{	};
	virtual bool is_link_strings( ) override { return false; };
};
struct rab2text_transformer_wchar : rab2text_transformer_base {
	virtual partstringA symbol2str( int symbol ) override {
	};
	virtual bool is_link_strings( ) override { return true; };
	//return join( pa , "");
};
*/

// ----------------------- SAVER JSON ------------------

static json_ctx::json_print_option acjo_prorate_map(20);
static json_ctx::json_print_option acjo_prorate_arr(40);

struct r_ctx_saver_alphabet_ngramm {
	using t_AC = c_arithmetic_coding_ngramm;
	

	t_error_info error;
	darray<stringA> char2name;
	r_alphabet_ngramm * ab=0 ; 
	stringA buffname;
	uint saveopt=0;
	darray<int> charsbuff;
	json_ctx::json_print_option * popt_rate=0;
	//bufferization_string bufferA;
	stringA buffer_seqA;
	darray<t_freq> freq_buff;
	stringW keyw;

	parray<t_freq> cline;
	t_freq cnorm=0 , csumm=0;

	int err_unkjnode(const char* nm); 

	r_ctx_saver_alphabet_ngramm( r_alphabet_ngramm * _ab , uint opt , size_t bsz=10*1024 ) {
		ab = _ab;
		saveopt = opt;
		popt_rate = saveopt & c_arithmetic_coding_ngramm::eBJO_saverateAsMap ? &acjo_prorate_map : &acjo_prorate_arr;
		//bufferA.make( bsz );
		buffer_seqA.reserve(bsz);
	}

	/*
	t_freq get_norm( const parray<t_freq> & l , int seqval) {
		bool initsumm = (saveopt & t_AC::eBJO_UseInputData) || !(saveopt & t_AC::eBJO_UseDefaultSumm); 
		return initsumm ? ab->get_init_summ( seqval ) : l[l.size()-1] ;
	}*/
	conf_variant make_tr( t_freq val , t_freq newnorm ) { 
		if(saveopt & t_AC::eBJO_FloatTransition) return conf_variant(val);
		return conf_variant( int( std::lround(val*newnorm)) );
	}

	private: //use cline,csumm,cnorm
		conf_variant tr_terminal(  int seqval ){
		return make_tr( cline[0]/csumm , cnorm );
	}
	conf_struct_ref tr_chars2jnode(  int seqval ){
		auto atchars = ab->get_t_chars();
		bool asmap = saveopt & c_arithmetic_coding_ngramm::eBJO_saverateAsMap;
		auto nn=conf_struct_t::newnode(asmap ? conf_variant::ejtObject : conf_variant::ejtArray);
		nn->print_option =	popt_rate;
		auto tf = cline.slice( ab->first_char() );
		int cc=0;
		FOREACH( f , tf ) {
			auto v= make_tr( f[0]/csumm, cnorm  );
			if (asmap ) nn->push_back( char2name[cc] , v  );
			else nn->push_back( v );
			cc++;
		}
		return nn;
	}
	conf_struct_ref tr_ab2jnode(  int seqval ){
		auto nn= conf_struct_t::newnode(conf_variant::ejtArray);
		nn->print_option =	popt_rate;
		auto tf = cline.slice( ab->first_alpha() , ab->c_alphabets );
		FOREACH( f , tf ) 
			nn->push_back( make_tr( f[0]/csumm, cnorm  ) );
		return nn;
	};
	public:
	int jnode2tr(conf_struct_ref nn ,  int seqval );

	int tr2jnode(conf_struct_ref nn ,  int seqval ){
		if ( saveopt & t_AC::eBJO_UseInputData ) {
			cline = ab->get_line(seqval);
			cnorm = ab->get_init_summ( seqval );
			csumm = cnorm;
		} else {
			//cline = ab->get_algoline(seqval);
			auto l = ab->get_algoline(seqval); auto sz = l.size()-1;
			freq_buff.resize( sz );
			auto ls = l.data();
			for(size_t i=0;i<sz;i++)  freq_buff[i]=ls[i+1]-ls[i];
			cline = freq_buff ; 
			csumm = l[l.size()-1];
			cnorm = saveopt & t_AC::eBJO_UseDefaultSumm ? csumm : ab->get_init_summ( seqval );
		}

		nn->set("summrate" , int(std::lround(cnorm)) );
		//nn->set("summrate" , make_tr(cnorm,1) );
		nn->set("alpha" ,	tr_ab2jnode( seqval ) );
		nn->set("chars" ,	tr_chars2jnode( seqval ) );
		nn->set("terminal" ,tr_terminal( seqval ) );
		return 0;
	};

	stringA ngseq2name(i_ngrammseq iseq){
		auto seq = ab->start_seq;
		seq.initbyval(iseq);
		seq.toChars( charsbuff );
		FOREACH( p , charsbuff) { *p -= ab->first_char(); }
		bool is_wchar = 0!=(saveopt & c_arithmetic_coding_ngramm::eBJO_WChar);

		/*
		bufferA.lock();
		FOREACH( p , charsbuff) { 
			bufferA.push( char2name[*p] );
			if (!is_wchar ) bufferA.push(",",1);
		}
		auto res= bufferA.unlock();
		*/
		auto fp = buffer_seqA.size();
		for( auto & p : charsbuff) { 
			buffer_seqA.append( char2name[p] );
			if (!is_wchar ) buffer_seqA.append(",",1);
		}
		auto res = buffer_seqA.slice( fp  );

		return res;
	}
	int loadjson_ab( r_alphabet_ngramm & ab , conf_struct_ref jr  );


};

t_freq r_alphabet_ngramm::get_init_summ( i_ngrammseq seq ){
	return save_init_freqs_summ[seq];
	//auto l =get_algoline(seq); return l[l.size()-1];
};
parray<i_ngrammseq> r_alphabet_ngramm::get_all_ngrammseq(  ){
	darray<i_ngrammseq> res; res.reserve(ngramm_fullsize);
	if ( fullngramm ) { 
		for( int i=0;i<ngramm_fullsize;i++)  res.push_back(i); 
	} else {
		FOREACH(p, f_seq2lines) res.push_back( p->first );
	}
	return res;
};

int r_alphabet_ngramm::savetojson(conf_struct_ref jroot , uint saveopt ){
	auto jr = jroot->add_node(  ); 

	r_ctx_saver_alphabet_ngramm ctx( this , saveopt);
	jr->set("id",int( id_alpha ));
	jr->set("name", name );
	jr->set("ngramm",int( ngramm ));
	jr->set("Single", OneSymbolPerWord );
	jr->set("symbols", c_alpha_size );
	jr->set("cnt_alpha", c_alphabets );
	auto atchars = get_t_chars();
	{int cc=0; ctx.char2name.resize( atchars.size() );
		if (saveopt & c_arithmetic_coding_ngramm::eBJO_WChar ) {
			stringA as; as.reserve( atchars.size() );
			FOREACH( p, atchars) { 
				ctx.char2name[cc] = wide_to_utf8_add( ctx.buffname , *p   );
				if (ctx.char2name[cc].size()==0) {
					return 125;
				}
				as << ctx.char2name[cc];
				cc++;
			}
			jr->set("alphabet",as );
		} else { 
			FOREACH( p, atchars) { ctx.char2name[cc] = format("%d",*p); cc++; } //TODO:fast??
			jr->set_array("alphabet", atchars );
			//jr->set()
		}
	};

	auto nn = jr->set_node( "transition_0" );
	ctx.tr2jnode( nn , 0 );

	if ( ngramm>1 ) {
		auto wn =jr->set_node( "transitions" );
		auto ngseq = get_all_ngrammseq().slice(1);
		FOREACH( ps, ngseq ) {
			auto nm= ctx.ngseq2name(*ps);
			auto nn = wn->add_node( nm );
			ctx.tr2jnode( nn , *ps );
		}
	};
	return 0;
	//alphabet

};

int c_arithmetic_coding_ngramm::savetojson(conf_struct_ref root , uint saveopt ){
	prepare_alphabet_set();

	//auto nal = json_node::newnode( json_node::ejtArray );
	//pro_rates.values_per_line = 10;

	//auto nsrc =root->setnode( "source" , json_node::ejtArray );
	//FOREACH( pd, extrainfo.named_props ) nsrc->add( pd->key, pd->data );
	if (1) {
	auto acmn = root->set_node("ac_params");
	acmn->set("start_alpha",start_alpha);
	acmn->set("alpha_has_terminal_char",alpha_has_terminal_char );
	acmn->set("minimum_frequency", sv.Min_frequency );
	};
	//acmn->set("" )

	auto aln = root->set_node("alphabets", conf_variant::ejtArray );
	FOREACH( pa, alphabet_s ){ 
		pa->savetojson( aln , saveopt );
	};
	return 0;
};
int c_arithmetic_coding_ngramm::savetojson( json_ctx * jo , const stringA & filenm , uint saveopt ){
	json_ctx jol;
	if (!jo) { jol.new_root(); jo=&jol; }
	savetojson( jo->root , saveopt);
	jo->savefile( filenm );
	return 0;
};

// -----------------------
// ----------------------- LOAD JSON ------------------

//eAC_JsonNotFoundNode

int r_ctx_saver_alphabet_ngramm::err_unkjnode(const char* nm) {
	return error.pushf(t_ac_base::eAC_JsonNotFoundNode,"Not found node <%s>",nm);
}

int r_ctx_saver_alphabet_ngramm::jnode2tr(conf_struct_ref nn ,  int seqval ){
	if (nn.empty()) return error.push(t_ac_base::ecErrTransitionNotSet,"AC.json.load:Transition node not found ");
	//TODO: seqval
	double term,summrate;
	if (!nn->get("terminal",term)) return err_unkjnode("terminal");
	if (!nn->get("summrate",summrate)) return err_unkjnode("summrate");
	auto nchars= nn->getnode("chars"); if(nchars.empty()) return err_unkjnode("chars");
	auto nalpha= nn->getnode("alpha"); if(nchars.empty()) return err_unkjnode("alpha");
	//auto term= nn->get<double>("terminal");
	ab->init_set_fin_trans( seqval, term );

	if (nchars->stype == conf_variant::ejtArray ) {
		freq_buff.resize( nchars->size() ); 
		int i=0; for( auto p : *nchars ) {
			p.Value().convert(freq_buff[i]); i++; }
	} else {
		freq_buff.resize( nchars->size() ); 
		for( auto p : *nchars ) {
			auto & key = p.Key();
			utf8_to_wide( keyw, key );
			if (keyw.size()!=1) return error.pushf(t_ac_base::eAC_InvalidWideChar,"AC.json.load: Invalid string <%s>", key );
			auto cc = get_def( ab->char2code , keyw[0] , -1 );
			if ( cc== c_char(-1) ) return error.pushf(t_ac_base::eAC_UnknownJsonChar,"AC.json.load: Undefined char <%s>", key );
			cc -= ab->first_char();
			p.Value().convert(freq_buff[cc]); 
		}
	};
	ab->init_set_chars_trans( seqval, parray<t_freq>( freq_buff ) );

	nalpha->toArray( freq_buff );
	ab->init_set_alpha_trans( seqval, parray<t_freq>( freq_buff ) );

	//auto summrate = nn->get<double>("summrate");
	ab->save_init_freqs_summ[seqval] = summrate;
	return 0;
};

int r_ctx_saver_alphabet_ngramm::loadjson_ab( r_alphabet_ngramm & ab , conf_struct_ref jr  ){
	return 0;
};

int r_alphabet_ngramm::loadjson(conf_struct_ref jr  ){
	r_ctx_saver_alphabet_ngramm ctx( this , 0 , 0 );
	error.reset();

	bool single=false;	// default
	ngramm = 1;			// default
	jr->get("id",id_alpha);
	jr->get("name", name );
	jr->get("ngramm", ngramm );
	jr->get("single", single );
	jr->get("cnt_alpha", c_alphabets );
	auto jv = jr->getvar("alphabet");  
	switch ( jv.type ) {
		case conf_variant::ejtArray: {
			auto ia = jr->get_array<int>("alphabet");
			initialize(id_alpha, ngramm , c_alphabets , ia );
			};break; 
		case conf_variant::ejtStr: {
			auto sa = jr->get<stringW>("alphabet");
			initialize(id_alpha, ngramm , c_alphabets , sa );
			}; break;
		default: return error.push(t_ac_base::ecErrABInvalidJsonFromat,"AC.json.load: Invalid json format");
			//return error.push( t_ac_base::ecErrABInvalidJsonFromat ,"Invalid <alphabet> node format!" );
	};
	OneSymbolPerWord = single;
	save_init_freqs_summ.resize(ngramm_fullsize);

	{
	 //auto trs = jr->getnode("transition_0");	
	 if (ctx.jnode2tr( jr->getnode("transition_0") , 0 )) 
		 return error.push(ctx.error);
	}
	if (ngramm>1) {
		auto trs = jr->getnode("transitions");
		if (trs.empty()) return error.push(t_ac_base::ecErrTransitionNotSet , "AC.json.load:Transition node not found " );
		ctx.keyw.reserve(120);
		for( auto t : *trs ){
			auto & key = t.Key();
			utf8_to_wide( ctx.keyw, key );
			i_ngrammseq iseq;
			auto e = make_seq_id(iseq , ctx.keyw); 
				if (e) return error.pushf(e, "AC.json.load: Invalid char sequence %s" , key );
			if (ctx.jnode2tr( t.Value().toNode() , iseq ))
				return error.push(ctx.error);
	}};

	return 0;

	//initialize()
};


int c_arithmetic_coding_ngramm::loadjson(conf_struct_ref root  ){
	static const char* def_err="AC:Invalid json fromat";
	alpha_ready=false;
	//int err;
	auto aln = root->getnode("alphabets");
	auto na = aln->toArray<conf_struct_ref>();
	alphabet_s.resize( aln->size() );
	stringA mask_notset;
	FOREACH( pn, na ){ 
		r_alphabet_ngramm ab;
		ab.c_alphabets = static_cast<int>(na.size());
		try {
			int e= ab.loadjson( *pn );
			if (e==ecErrTransitionNotSet) format_to(mask_notset,"%d,",ab.id_alpha);
			else if (e!=0) 
				return error.push( ab.error );
		} catch( crsys::iexception ie ) { return error.push(ie.ecode,def_err);
		} catch (int e) { return error.push(e,def_err);
		} catch (...) { return error.push(ecErrABInvalidJsonFromat , def_err); };

		if (add_alphabet( &ab )) 
			return error.error_id;
	};
	auto acmn = root->getnode("ac_params");
	if (1) { if(!acmn.empty()) {
	acmn->get("start_alpha",start_alpha);
	acmn->get("alpha_has_terminal_char",alpha_has_terminal_char );
	acmn->get("minimum_frequency", sv.Min_frequency );
	}}

	prepare_alphabet_set();
	if (mask_notset.size()) 
		return error.pushf(ecErrTransitionNotSet,"transitions not set in <%s>",mask_notset);

	return err();
};

int c_arithmetic_coding_ngramm::loadjson( json_ctx * jo , const stringA & filenm  ){
	json_ctx jol;
	if (!jo) { jol.new_root(); jo=&jol; }
	if (jo->parse_DataOrFile(filenm))  {
		return error.push(jo->error);
	}
	return loadjson(jo->root);
};


}; //namespace tbgeneral{ 

// ----------------------- LOAD/SAVE binary  ------------------
namespace tbgeneral { 

//c_arithmetic_coding_base::t_scale_vars 
t_serializer & operator << (t_serializer & st , r_alphabet_ngramm v) { 
	if (v.serialize( st )) throw t_ac_base::eAC_SerializeError; 
	return st;}
t_serializer & operator >> (t_serializer & st , r_alphabet_ngramm & v) { 
	if (v.deserialize( st )) throw t_ac_base::eAC_DeSerializeError; 
	return st;}

int r_alphabet_ngramm::serialize( t_serializer & ser ){
	if (!freqs_ready) throw t_ac_base::eAC_AlphabetNotReady;
	auto startofs = ser.make_start_crc();
	auto atchars = get_t_chars();
	ser << id_alpha << ngramm << c_alpha_size << c_alphabets << OneSymbolPerWord;
	ser << atchars;
	ser << save_init_freqs_summ;
	ser << wfreqs;
	ser.fix_end_crc(startofs);
	return 0;
};
int r_alphabet_ngramm::deserialize( t_serializer & ser ){
	//TODO: check
	if (!ser.check_crc()) throw t_ac_base::eAC_DeSerAlphaCrcError;
	darray<c_char> atchars; bool single;
	ser >> id_alpha >> ngramm >> c_alpha_size >> c_alphabets >> single;
	ser >> atchars;
	initialize(id_alpha, ngramm , c_alphabets , atchars );
	OneSymbolPerWord = single;
	freqs_ready = true;
	ser >> save_init_freqs_summ;
	ser >> wfreqs;
	return 0;
};


int c_arithmetic_coding_ngramm::serialize( t_serializer & ser ){
	prepare_alphabet_set();
	auto startofs = ser.make_start_crc();
	ser<< start_alpha << alpha_has_terminal_char << sv.Min_frequency;
	ser << alphabet_s;
	ser.fix_end_crc(startofs);
	return 0;
};
int c_arithmetic_coding_ngramm::deserialize( t_serializer & ser ){
	//TODO: check
	if (!ser.check_crc()) return error.push(t_ac_base::eAC_DeSerCrcError,0);
	try{
	alpha_ready=false;
	ser >> start_alpha >> alpha_has_terminal_char >> sv.Min_frequency;
	ser >> alphabet_s;
	} catch( crsys::iexception ie ) { return error.push(ie.ecode,ie.what());
	} catch (int e) { return error.push(e,0);
	};
	prepare_alphabet_set();
	return 0;
};
bool c_arithmetic_coding_ngramm::isready(){
	return this->alpha_ready;
}; 


}; //namespace tbgeneral{ 

//======= test =============



#include "ArithmeticCoding_test.h"



namespace tbgeneral{ namespace test{ namespace ArithmeticCoding{

enum { SingleSymbolPerWord = 0x10 };

using t_AC = c_arithmetic_coding_ngramm;

static void test_r_alphabet_ngramm_init( c_arithmetic_coding_ngramm & ac , int id, int acnt, int ngr , const stringW & sw , int opt=0 ){
	r_alphabet_ngramm ab;
	ab.initialize( id , ngr , acnt , sw );
	ab.OneSymbolPerWord = 0 != (opt & SingleSymbolPerWord);
	darray<int> ab_trans;  ab_trans.resize(acnt);
	darray<int> chars_trans; chars_trans.resize(sw.size());
	FOREACH(p , ab_trans)  *p = 100;
	FOREACH(p , chars_trans)  *p = 1000;

	ab.init_set_trans(0, 1, parray<int>( ab_trans) , parray<int>(chars_trans));
	if ( ab.ngramm==2 ) {
		auto f = ab.start_seq;
		int delta=10, dofs=0;
		FOREACH( pc , sw ) { dofs+=delta;
			auto cc = ab.char2code[*pc];
			auto val = f.next( cc );
			FOREACH(p , chars_trans)   *p += delta; 
			//ab.init_set_trans(val, 1, parray<int>( ab_trans) , parray<int>(chars_trans));
			ab.init_set_trans<int>(val, 1, ab_trans , chars_trans );
			f.down_degree();
		};
	};
	if (ac.add_alphabet( &ab ))
		throw 1;
}

static void set_spec_weigth( r_alphabet_ngramm * ab , const stringW path ,  r_alphabet_ngramm::t_freq val ) {
	//auto fp = ab->start_seq;
	//if (ab.ngramm==1) 
	int sz = std::min<int>(static_cast<int>(path.size()), ab->ngramm);
	if (sz<=0) { return; }
	auto ppath = path.substr(0,sz-1);
	auto l= ab->get_line( ppath ); 
	auto cc= ab->char2code[ path.at(sz-1) ];
	l[cc] = val;
}

static void test_ac_encode_decode( t_AC & ac , const stringW & idata ){
	int e_bitcnt, d_bitcnt;
	if (0){
		auto cdata = ac.string2code( idata );
		auto odata = ac.code2string<stringW>(cdata);
		if (odata != idata) {
			printf("char: code/decode error!"); return;
		}
	}
	auto edata = ac.encode_string(idata);
	e_bitcnt = ac.wv.bitstream.countbits_in;
		if ( ac.err()) { throw ac.err(); }
	auto dedata= ac.decode_string<stringW>( edata );
	d_bitcnt = ac.wv.bitstream.countbits_in;
		if ( ac.err()) { throw ac.err(); }
	format_to(std::cout,"-----rate %f ( %d/%d) bits:(%d | %d)\n" , float(edata.size())/idata.size() , edata.size() , idata.size() , e_bitcnt, d_bitcnt );
	format_to(std::cout,"on decode = %s\n" , dedata);
	format_to(std::cout,"code/decode = %s : %s \n", idata==dedata? "OK" :"BAD" , dedata);

}
static void test_ac_data_encode_decode(t_AC & ac){
	stringW idata[] = { 
	L"test write. 123",
	L"test . 123456789 test . 123456789 test . 123456789 test . 123456789 test",
	L"abababababababababababababababababababababababababababababababababababab", 
	L"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccca", 
	L"rwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrwrw", 
	L"ab" 
	};
	for(size_t i=0;i<ARRAYLEN(idata);i++) test_ac_encode_decode( ac , idata[i] );
}


static void test_ac_ngramm_initAC(t_AC & ac){
	auto enl=L"abcdefghijklmnopqrstuvwxyz ,.";
	auto rul=L"абвгдеёжзийклмнопрстуфхцчшщъыьэюя ,.";
	auto genl=L"()-_.?,:;\"";
	auto nums="0123456789.";
	test_r_alphabet_ngramm_init( ac , 1,4 , 2 , enl );
	test_r_alphabet_ngramm_init( ac , 2,4 , 2 , rul );
	test_r_alphabet_ngramm_init( ac , 3,4 , 1 , genl,	SingleSymbolPerWord );
	test_r_alphabet_ngramm_init( ac , 4,4 , 1 , nums );
	{	auto pab = ac.get_alpha(1);
		set_spec_weigth( pab , L"c" , 0 );
		set_spec_weigth( pab , L"cc" , 0 );
		set_spec_weigth( pab , L"ab" , 50000 );
		set_spec_weigth( pab , L"aa" , 50000 );
		set_spec_weigth( pab , L"ba" , 50000 );
	}

	ac.init_set_fin_weigth(1.0/2000);
	ac.prepare_alphabet_set();
}


static const char* def_file_json() { return "d:/temp/test_ac.json"; }

static bool test_comparefiles( const stringA & f1 , const stringA & f2 ){
		auto s1= filesystem::readstrfromfile( f1 );
		auto s2= filesystem::readstrfromfile( f2 );
		auto res= s1==s2;
		LOG(" %s (file 1) %s (file 2)", res?"OK":"BAD" , res?"=":"<>" );
		return res;
};

static void test_ac_ngramm_json(){
	auto fjson_templ = "d:/temp/test_ac_it0%x_%d.json";
	c_arithmetic_coding_ngramm ac;
	if ( ac.loadjson( 0 , def_file_json() ) )
		{ LOG( "%s" , ac.error.text ); throw ac.error; }

	int savefl[] = { t_AC::eBJO_saverateAsMap | t_AC::eBJO_WChar 
		, t_AC::eBJO_FloatTransition | t_AC::eBJO_WChar 
		,t_AC::eBJO_UseDefaultSumm | t_AC::eBJO_WChar 
	};
	LOG("json save/load test");
	for(size_t i=0;i<ARRAYLEN(savefl);i++) {
		auto fl = savefl[i];
		auto f1 = format( fjson_templ , fl , 1 );
		auto f2 = format( fjson_templ , fl , 2 );
		LOG("test_ac_ngramm_json: test %x flags", fl );
		c_arithmetic_coding_ngramm ac;
		if ( ac.loadjson( 0 , def_file_json() ) )	{ LOG( "%s" , ac.error.text ); throw ac.error; }
		ac.savetojson(0, f1 , fl );
		if ( ac.loadjson( 0 , f1 ) ) { LOG( "%s" , ac.error.text ); throw ac.error; };
		ac.savetojson(0, f2 , fl );
		test_comparefiles( f1, f2 );
	};
	if (1) {
		auto f1 = format( fjson_templ , 0x10001 , 55 );
		auto f2 = format( fjson_templ , 0x10001 , 56 );
		int  flags = t_AC::eBJO_WChar;
		LOG("--- test serialize t_AC: files( %s , %s ) ", f1, f2);
		ac.savetojson(0, f1 , flags );
		t_mem_serializer mems(10*1024);
		ac.serialize( mems );
		LOG("--- bin size=%d" , mems.buff.size() );

		c_arithmetic_coding_ngramm ac1;
		mems.seek(0);
		if (ac1.deserialize( mems )) {
			LOG("ERROR:deserialize ! %s ", ac1.error.text); return; 
		}
		ac1.savetojson(0, f2 , flags );

		test_comparefiles( f1, f2 );
		test_ac_data_encode_decode(ac1);

	}
};


static void acngramm_fulltest(c_arithmetic_coding_ngramm & ac , const stringA &jsfile){
	int savefl = t_AC::eBJO_DefaultMask;
	//savefl =  t_AC::eBJO_saverateAsMap | t_AC::eBJO_FloatTransition | t_AC::eBJO_WChar ;
	//savefl = t_AC::eBJO_UseDefaultSumm | t_AC::eBJO_saverateAsMap | t_AC::eBJO_WChar;
	LOG("save c_arithmetic_coding_ngramm to %s", jsfile );
	ac.savetojson(0, jsfile , savefl );
	test_ac_data_encode_decode(ac);
	test_ac_ngramm_json(); 
}
static void test_c_arithmetic_coding_ngramm(){
	c_arithmetic_coding_ngramm ac;
	stringA jsfile(def_file_json()); 
	if (1){ c_arithmetic_coding_ngramm ac;
		if ( ac.loadjson( 0 , "c:/temp/def6ab.json" )) 
			LOG("OK error test:%s",ac.error.text);
	}
	if (1){
	test_ac_ngramm_initAC(ac);
	acngramm_fulltest(ac , def_file_json());
	}
	if(1){
	if (ac.loadjson(0,get_ac_initjson_0data(0)) && (ac.error.error_id !=t_ac_base::ecErrTransitionNotSet) ) 
		throw 22;
	ac.error = t_error_info();
	ac.savetojson(0, jsfile+"-1.json"  );
	test_ac_data_encode_decode(ac);
	}


	//for (int i=) r_alphabet_ngramm
	//ac.init

};

}}}; //namespace tbgeneral::test::ArithmeticCoding{ 