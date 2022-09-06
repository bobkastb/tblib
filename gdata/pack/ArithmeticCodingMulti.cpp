#include <gdata/tb_map.h>
#include "ArithmeticCoding.h"
#include "tsys/tb_log.h"
#include "gdata/t_format.h"

// ---------- END HEADER -------------
namespace tbgeneral{

template <class Tac> void printACparams(Tac & ac){
	LOG(" Top_value %d Max_frequency %d Min_frequency %d" , ac.Top_value , ac.Max_frequency , ac.Min_frequency );
	LOG("First_qtr %d Half %d Third_qtr %d" ,ac.First_qtr, ac.Half , ac.Third_qtr );
	LOG("cHigh=%d  cLow=%d  cValue=%d  bits_to_follow=%d ", ac.cHigh , ac.cLow , ac.cValue, ac.bits_to_follow );
	//LOG("Start alpha - %d" ,  ac.start_alpha->index );
	
	stringA buff;
	for (uint i=1;i<ac.wfreqs.size();i++) {
		auto d=ac.wfreqs[i]-ac.wfreqs[i-1];
		buff << format("%d ", d );
		//if (i%10 == 0) {buff << "\n";}
	}
	LOG("%s",buff);
}
void printACparams(c_arithmetic_coding_m & ac){
	
	LOG(" Top_value %d Max_frequency %d Min_frequency %d" , ac.sv.Top_value , ac.sv.Max_frequency , ac.sv.Min_frequency );
	LOG("First_qtr %d Half %d Third_qtr %d" ,ac.sv.First_qtr, ac.sv.Half , ac.sv.Third_qtr );
	LOG("cHigh=%d  cLow=%d  cValue=%d  bits_to_follow=%d ", ac.wv.cHigh , ac.wv.cLow , ac.wv.cValue, ac.wv.bits_to_follow );
	LOG("Start alpha - %d" ,  ac.start_alpha->index );
	FOREACH( pa , ac.alphabet_s ) {
		LOG("alphabet %d  wf.size=%d" , pa->index , pa->wfreqs.size() );
	}
	
	stringA buff;
	for (uint i=1;i<ac.wfreqs.size();i++) {
		auto d=ac.wfreqs[i]-ac.wfreqs[i-1];
		buff << format("%d ", d );
	}
	LOG("%s",buff);
}

};

namespace tbgeneral{

using c_char = c_arithmetic_coding_m::c_char;
using t_char = c_arithmetic_coding_m::t_char;


c_arithmetic_coding_m::r_alphabet::r_alphabet(){
	terminal_char = -1; 
	index = -1;
};


bool c_arithmetic_coding_m::is_eof_char( c_char symbol ){
	auto t=curr_alpha->terminal_char;
	return (t==symbol && t>=0);
};

int c_arithmetic_coding_m::decode_tchar( c_char c , t_char & tc ){
	if (c < int( curr_alpha->code2alpha.size()) ) {
		if (lastsym_isspecial) { pusherror( ecErrSequenceOfSpecialSymbol ); }
		set_alphabet( curr_alpha->code2alpha[c] );
		lastsym_isspecial=true;
		return -1;
	} else {
		lastsym_isspecial=false;
		tc = curr_alpha->code2char[ c ];
		if (curr_alpha->ei.OneSymbolPerWord) 
			set_alphabet( prev_alpha->index , false );
		return 0;
	}
	return -1;
}; 

int  c_arithmetic_coding_m::encode_tchar( t_char tc , c_char & cc ){
	if ((next_alfa != curr_alpha->index) && (next_alfa>=0))
			set_alphabet( next_alfa , false );
	auto c= get_def( curr_alpha->char2code, tc ,c_char(eErrorCode));
	if (c!=eErrorCode) {
		cc = c;
		if ( curr_alpha->ei.OneSymbolPerWord ) 
			next_alfa= prev_alpha->index;
		return 0;
	} else {
		auto ai = get_def( char2alpha , tc , -1 );
		if (ai<0) { 
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

int c_arithmetic_coding_m::encode_endchar( c_char & cc ){
	if ((next_alfa != curr_alpha->index) && (next_alfa>=0))
			set_alphabet( next_alfa , false );
	if (curr_alpha->terminal_char<0) { 
		auto ai= alpha_has_terminal_char; 
		cc  = curr_alpha->alpha2code[ai];
		next_alfa = ai;
		return 1;
	}
	cc= curr_alpha->terminal_char;
	return 0;
}; 

/*
darray<c_char> c_arithmetic_coding_m::tchar2code( const darray<t_char> & ca  ){
	darray<c_char> res; res.reserve( ca.size() );
	auto palpha = start_alpha;
	auto lprev_alpha = start_alpha;
	FOREACH( pc , ca ){
		//palpha->char2code	
		auto c= get_def(palpha->char2code,*pc,c_char(eErrorCode));
		auto ai = c==eErrorCode ? char2alpha[*pc] : palpha->index ;
		if (ai!=palpha->index ) {
			lprev_alpha = palpha;
			auto trc = palpha->alpha2code[ai];
			palpha = &alphabet_s[ai];
			res.push_back( trc );
		}
		c_char ccode;
		if (!get_val(palpha->char2code,*pc,ccode) ) {
			pusherror(ecErrUnknownCharacter); break;	}
		res.push_back( ccode );
		if ( palpha->ei.OneSymbolPerWord ) {
			palpha = lprev_alpha;
		}
	}
	auto ai= alpha_has_terminal_char; 
	if (ai!=palpha->index) { 
		res.push_back( palpha->alpha2code[ai] );
		palpha = &alphabet_s[ai];
	}
	res.push_back( palpha->terminal_char );
	return res;
}; 
darray<t_char> c_arithmetic_coding_m::code2tchar( const darray<c_char> & ca ){
	darray<t_char> res; res.reserve( ca.size() );
	auto palpha = start_alpha; auto lprev_alpha = start_alpha;
	bool terminalpop=false;
	FOREACH( pc , ca ){
		if (*pc < int( palpha->code2alpha.size()) ) {
			lprev_alpha = palpha;
			palpha = &alphabet_s[palpha->code2alpha[*pc]];
			continue;
		}
		if (*pc==palpha->terminal_char){ terminalpop=true; continue; }
		if (terminalpop) { pusherror(ecErrDuplicateTerminal); break; }
		res.push_back( palpha->code2char[*pc] );
		if ( palpha->ei.OneSymbolPerWord ) 
			palpha = lprev_alpha;
		
	}
	//{ stringA b; b<<res[0]; }

	return res;
};
*/

void c_arithmetic_coding_m::prepare_encode_decode(){
	//error_id = 0;
	c_arithmetic_coding_base::prepare_encode_decode();
	//if (!start_alpha) pusherror(11);
	lastsym_isspecial = false;
	next_alfa = start_alpha->index;
	prev_alpha=curr_alpha=start_alpha;
	set_alphabet( start_alpha->index ); // set wfreqs &  prev_alpha
};

void c_arithmetic_coding_m::set_alphabet( i_alpha index , bool ispush ){
		if (ispush ) prev_alpha = curr_alpha;
		curr_alpha = &alphabet_s[ index ];
		wfreqs = curr_alpha->wfreqs;
}

/*
void c_arithmetic_coding_m::update_model(c_char symbol){
	if (symbol< int(curr_alpha->code2alpha.size()) && symbol>=0 ) {
		// смена алфавита
		if (lastsym_isspecial) { pusherror(ecErrSequenceOfSpecialSymbol); }
		set_alphabet( curr_alpha->code2alpha[symbol] );
		lastsym_isspecial=true;
	} else {
		lastsym_isspecial=false;
		if (curr_alpha->ei.OneSymbolPerWord) {
			set_alphabet( prev_alpha->index , false );
		}
	}
}; 

*/

void c_arithmetic_coding_m::prepare_alphabet_set(  ){ // после добавления всех алфавитов
	struct alpha_extra_info{
		c_arithmetic_coding_m::r_alphabet * alpha;
		double min_r;
		double summ_r;
	};
	if (alpha_ready) return;
	auto acnt= alphabet_s.size();
	darray<alpha_extra_info> aei; 
	int mwi=-1; double w=0 ,wsum=0; 
	std::map<r_alphabet_ei::t_id,i_alpha> map_a_id;
	bool useusertransit=true;
	// подсчитываем среднюю вероятность символа, делим на среднюю длину слова - это есть вероятность перехода
	// попутно подсчитываем минимальную вероятность символа для определения вероятности терминала
	FOREACH(a, alphabet_s) { 
		if (mwi<0 || w<a->ei.weight) { w=a->ei.weight; mwi=a->index; }; 
		wsum+=a->ei.weight;
		alpha_extra_info nei={a,0,0};
		aei.push_back(nei); auto ei = &aei[aei.size()-1];
		ei->min_r= a->ei.rates[0].freq;
		FOREACH( pc, a->ei.rates ){ if (pc->freq<ei->min_r) { ei->min_r=pc->freq; }; ei->summ_r+= pc->freq; }
		if (!a->ei.OneSymbolPerWord)
			FOREACH( t , a->ei.transit) { 
				ei->summ_r += t->rate; }
		useusertransit &= (a->index>0);
		map_a_id[ a->ei.id ] = a->index;
	}
	alpha_has_terminal_char = mwi; // терминал в алфавите с максимальным весом
	start_alpha = &alphabet_s[mwi];// стартовый алфавит - алфпвит с максимальным весом
	char2alpha.clear();
	//start_alpha->terminal_char
	// normalize weight
	FOREACH(ei, aei) { auto a= ei->alpha; a->ei.weight /=  wsum; 	}
	FOREACH(ei, aei) { auto a= ei->alpha; 
		double tr_m[100]={0}; 
		FOREACH( t , a->ei.transit ) {   tr_m[ map_a_id[t->to_id] ] = t->rate; }

		size_t szspec = !a->ei.OneSymbolPerWord ? acnt-1 : 0;
		size_t a_t_count= !a->ei.OneSymbolPerWord ? acnt : 0;
		size_t hasterm_sz= alpha_has_terminal_char==a->index ? 1 : 0;
		double basespfreq= ( ei->summ_r / a->ei.rates.size() ) ;
		a->char2code.clear();
		a->wfreqs.resize(0);
		a->wfreqs.reserve( a->ei.rates.size() + szspec + hasterm_sz );
		a->alpha2code.resize(a_t_count);
		a->code2alpha.resize(szspec);
		a->code2char.resize(a->ei.rates.size() + szspec + hasterm_sz );
		t_code sumwfreqs=0;
		a->wfreqs.push_back(0);
		int stc=0;
		for (uint ai=0;ai<a_t_count;ai++) {
			a->alpha2code[ai] = ( ai!=(uint)a->index ? stc : eErrorCode );
			if (ai==uint(a->index)) continue; 
			a->code2alpha[stc] = ai; 
			a->code2char[stc] =eErrorChar; 
			double vs =  useusertransit ? tr_m[ai] : ( basespfreq * alphabet_s[ai].ei.weight );
			sumwfreqs +=  alignRate( vs / ei->summ_r );
			a->wfreqs.push_back(sumwfreqs);
			stc++;
		}
		if (hasterm_sz) {
			a->code2char.push_back( eTerminalChar );
			a->char2code[eTerminalChar]=stc;  
			a->terminal_char = stc;
			sumwfreqs +=  alignRate( ei->min_r/10/ ei->summ_r );
			a->wfreqs.push_back( sumwfreqs );
			stc++;
		}		
		FOREACH( p , a->ei.rates ) { 
			if (exists(a->char2code, p->ch)) { pusherror(ecErrAlphaDuplicateChar); continue; }
			a->char2code[p->ch]=stc;  
			a->code2char[stc] = p->ch;
			sumwfreqs += alignRate( p->freq / ei->summ_r );
			a->wfreqs.push_back(sumwfreqs);
			i_alpha c2a;
			if ( !get_val(char2alpha , p->ch, c2a ) || c2a!=start_alpha->index)
				char2alpha[p->ch]=a->index;
			stc++;
		}
		//int ee=0;
	}
	alpha_ready = true;
	// initialize vars before code/decode	
};

void c_arithmetic_coding_m::add_alphabet(  const r_alphabet_ei & exi ){
	alpha_ready=false;
	r_alphabet na;
	na.terminal_char = -1;	
	na.index = static_cast<i_alpha>( alphabet_s.size());
	na.ei= exi;
	int n_tsz = 0;
	FOREACH( t , na.ei.transit) if (t->to_id!=0) n_tsz++;
	if ( na.ei.transit.size() != size_t(n_tsz)) { throw "Invalid transit size!"; }
	alphabet_s.push_back( na );
};

c_arithmetic_coding_m::c_arithmetic_coding_m(){
	start_alpha = 0;
	alpha_has_terminal_char=-1;
	alpha_ready=false; // алфавиты подготовленны
	curr_alpha=prev_alpha = 0;
	next_alfa=-1;
	lastsym_isspecial=false;
};

/*
	c_arithmetic_coding_m();

	c_char char2code( t_char c , i_alpha & ia );
	t_char code2char( c_char c , i_alpha ia );
	t_char code2char( c_char c ); // с текущим алфавитом

*/



} // namespace tbgeneral



