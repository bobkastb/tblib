#pragma warning( disable : 26812 )

#include "ArithmeticCoding.h"


// END HEADER

namespace tbgeneral{

using c_char = c_arithmetic_coding_base::c_char;
using t_char = c_arithmetic_coding_base::t_char;
using t_code = c_arithmetic_coding_base::t_code;


c_arithmetic_coding_base::c_arithmetic_coding_base(){
	sv.set_CodeLen(DEF_CODE_LEN);
	prepare_work_values();
	alpha_ready=false; // алфавиты подготовленны

};

void c_arithmetic_coding_base::pusherror( int e ){
	error.push( e , "");
}; 

t_code c_arithmetic_coding_base::alignRate( double v ) const{
	t_code mf= sv.Min_frequency;
	auto iv = t_code(v*sv.Max_frequency);
	return   iv< mf ? mf : iv ;
}

void c_arithmetic_coding_base::prepare_encode_decode(){
	//error_id = 0;
	prepare_alphabet_set();

	
	prepare_work_values();
//	lastsym_isspecial = false;
//	curr_alpha =0;
//	prev_alpha=curr_alpha=start_alpha;
//	set_alphabet( start_alpha->index ); // set wfreqs &  prev_alpha
};

void c_arithmetic_coding_base::decode_start( const  darray<byte> & input ){
	prepare_encode_decode();
	wv.bitstream.prepare_pop( input );
	wv.cValue=0; for (uint i=0;i<sv.CODE_LEN;i++) 
			wv.cValue =wv.cValue*2 + wv.bitstream.pop();
};
void c_arithmetic_coding_base::encode_start(){
	prepare_encode_decode();
	wv.bitstream.prepare_push(1024);
};

void c_arithmetic_coding_base::bit_plus_follow(int bit)
{   wv.bitstream.push( bit);
	for (;wv.bits_to_follow>0;wv.bits_to_follow--) 
		wv.bitstream.push(!bit);
}

void c_arithmetic_coding_base::encode_end(){
	// Вывод двух битов опpеделяющих чеpвеpть лежащую в текущем интеpвале 
	wv.bits_to_follow += 1;           
	bit_plus_follow( wv.cLow<sv.First_qtr ? 0 : 1 );
	wv.bitstream.finish_push();
};

static t_code mul_and_div_i(t_code m1, t_code m2, t_code d){
	return (uint64(m1)*m2) / d;
}

double c_arithmetic_coding_base::getchar_weight( c_char symbol ){
	auto summfreqs = wfreqs[wfreqs.size()-1];
	return double(wfreqs[symbol+1]-wfreqs[symbol])/summfreqs;
};


void c_arithmetic_coding_base::encode_char( c_char symbol ){
	t_code range;                    // Шиpина текущего кодового интеpвала    
	auto high=wv.cHigh, low=wv.cLow;

	range = (high-low)+1;    
	auto summfreqs = wfreqs[wfreqs.size()-1];
	// Сужение интеpвала кодов  до выделенного для symbol
	high = low + mul_and_div_i(range ,wfreqs[symbol+1] , summfreqs)-1; 
	low = low +  mul_and_div_i(range ,wfreqs[symbol] , summfreqs );
	//printf("lh=[ %d  %d ] ch:%c (%d) int:%d \n", low , high , code2char[symbol] , code2char[symbol] , symbol );
	for (;;) {                     /* Цикл по выводу битов  */
		//printf("lh=%f %f %s\n",low/float(Top_value),high/float(Top_value), high<Half?"A":low>=Half?"C": low>=First_qtr && high<Third_qtr? "B":"X" );

		if (high<sv.Half) {           // Если в нижней половине исходного интеpвала то вывод 0
			bit_plus_follow(0); 
		} else if (low>=sv.Half) {	// Если в веpхней, то вывод 1, а затем  убpать известную у   гpаниц общую часть 
			bit_plus_follow(1);   
			low -= sv.Half;       
			high -= sv.Half;      
		}  else if (low>=sv.First_qtr && high<sv.Third_qtr) {
			// Если текущий интеpвал содеpжит сеpедину исходного, то вывод еще одного обpатного бита а сейчас убpать общую часть   
			wv.bits_to_follow +=1; 
			low -= sv.First_qtr;   
			high -= sv.First_qtr;  
		}	else 
			break;                // Иначе выйти из цикла  
		// Расшиpить текущий pабочий кодовый интеpвал
		low = 2*low;              
		high = 2*high+1; 
	}
	wv.cHigh=high; wv.cLow=low;
};

c_char c_arithmetic_coding_base::decode_char(){
	t_code range;                    // Шиpина интеpвала      
	t_code cum;                      // Hакопленная частота   
	c_char symbol;                   //  Декодиpуемый символ  
	auto summfreqs = wfreqs[wfreqs.size()-1];
	auto high=wv.cHigh, low=wv.cLow , value=wv.cValue;
	range = (high-low)+1;
	//cum = mul_and_div_i(value-low+1,summfreqs-1 , range);  // Hахождение значения накопленной частоты для value 
	cum =(uint64(value-low+1)*summfreqs-1)/range;
	for (symbol = 0; wfreqs[symbol+1]<=cum; symbol++) {};
	high = low + mul_and_div_i(range , wfreqs[symbol+1] , summfreqs)-1;                  
	low = low +  mul_and_div_i(range , wfreqs[symbol] , summfreqs );
	for (;;) {                     // Цикл отбpасывания битов
		if (high<sv.Half) {           // Расшиpение нижней половины
			// ничего 
		} else if (low>=sv.Half) {	// Расшиpение веpхней половины после вычитание смещения Half
			value -= sv.Half;         
			low -= sv.Half;           
			high -= sv.Half;
		} else if (low>=sv.First_qtr && high<sv.Third_qtr) { // Расшиpение сpедней половины
			value -= sv.First_qtr;
			low -= sv.First_qtr;
			high -= sv.First_qtr;
		} else break;

		low = 2*low;               // Увеличить масштаб   интеpвала  
		high = 2*high+1; 
		value = ( 2*value+ wv.bitstream.pop()) & sv.MASK_tcode;  //  Добавить новый бит Правильно заменить & на %Top_Value

	}
	wv.cHigh=high; wv.cLow=low; wv.cValue=value;
	return symbol;

};



}; //namespace tbgeneral{

namespace tbgeneral{

void c_arithmetic_coding_c::prepare_alphabet_set(  ){
	if (alpha_ready) return;
	double minf=freqs[0].freq, sum=0; for (uint i=0;i<freqs.size();i++){ auto f=freqs[i].freq; sum+=f; if ( minf>f) { minf=f; } }
	// add terminal
	auto term_fr=minf/10;
	//sum += term_fr;
	darray<t_code> wfreqs;
	wfreqs.resize(0); wfreqs.reserve(freqs.size()+1);
	t_code summfreqs= alignRate(term_fr/sum);
	wfreqs.push_back( 0 );
	for (uint i=0;i<freqs.size();i++)  {
		wfreqs.push_back( summfreqs );
		summfreqs += alignRate(freqs[i].freq/sum);
	}
	wfreqs.push_back( summfreqs ); // для облегчения декодирования (symbol+1)
	this->wfreqs = wfreqs;

	code2char.resize(freqs.size()+1);
	code2char[ eTerminalChar ]=0;
	char2code.clear();
	for (uint i=0;i<freqs.size();i++){ 
		char2code[freqs[i].ch]=i+1; 
		code2char[i+1]=freqs[i].ch;
	};
	alpha_ready = true;
}; // после добавления всех алфавитов

int c_arithmetic_coding_c::decode_tchar( c_char ic, t_char & tc ){
		if (ic >= (c_char)code2char.size()) { pusherror(ecErrRangeError); return -1;	}
		tc = code2char[ic];
		return 0;
}; 

int c_arithmetic_coding_c::encode_tchar( t_char tc , c_char & cc ){
		cc= get_def(char2code, tc, c_char(eErrorCode) );
		if (cc==eErrorCode) { pusherror(ecErrUnknownCharacter); return -1; }
		return 0;
}; 
int c_arithmetic_coding_c::encode_endchar( c_char & cc ){
	cc = (c_char)eTerminalChar;
	return 0;
}; 

stringW c_arithmetic_coding_base::DefaultIgnoreNG2sequence( const stringW & sw){
	static bool maps_ready;
	static byte maps[256]={0};
	if (!maps_ready) { 	auto l=DefaultIgnoreNG2sequence_List();
		FOREACH(p,l)  maps[byte(*p)]=1; 
		maps_ready= true;
	};
	stringW res; res.resize(sw.size()); auto dp = res.data();
	wchar last=' ';
	FOREACH(p,sw) {
		if (*p==' '){ 
			if (size_t(last)<sizeof(maps) && maps[last]!=0 ) 
			{ last = *p; continue; }
		}
		last = *p;
		*dp=*p; dp++;
	};
	res.resize( dp-res.data() );
	return res;
};
stringA c_arithmetic_coding_base::DefaultIgnoreNG2sequence_List(){
	static stringA res;
	if (!res.size()) res=" ,.!:?;-\n\a	";
	return res;
};

/*

darray<c_char> c_arithmetic_coding_c::tchar2code( const darray<t_char> & ca  ){
	darray<c_char> res; res.reserve( ca.size()+1);
	FOREACH( pc , ca ){
		auto c= get_def(char2code,*pc,c_char(eErrorCode));
		if (c==eErrorCode) { pusherror(ecErrUnknownCharacter); break; }
		res.push_back( c );
	};
	res.push_back( (c_char)eTerminalChar );
	return res;
}; 
darray<t_char> c_arithmetic_coding_c::code2tchar( const darray<c_char> & ca ){
	darray<t_char> res; res.reserve(ca.size());
	FOREACH( pc , ca ){
		if (*pc==eTerminalChar) { break; }
		if (*pc > (c_char)code2char.size()) { pusherror(ecErrRangeError); break;	}
		res.push_back( code2char[*pc] );
	};
	return res;
}; 

*/

}; //namespace tbgeneral{



