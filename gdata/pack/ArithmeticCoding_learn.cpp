
#include "tsys/tbfiles.h"

#include "gdata/tbt_locale.h"
#include "tsys/tb_log.h"
#include "ArithmeticCoding_learn.h"


//---------------
//--------------- END HEADER
//---------------

namespace tbgeneral{

int t_ac_text_learn::init0(){
	*this = t_ac_text_learn();
	ZEROMEM( specchars );
	stringA spch= c_arithmetic_coding_base::DefaultIgnoreNG2sequence_List(); // (" ,.!:?;-\n\a	");
	FOREACH ( pc , spch) { specchars[byte(*pc)]=*pc; }
	rate_endchar = c_arithmetic_coding_ngramm::default_EndCharWeigth;
	return error.error_id;
};

template<class TArray> void fillbin_array( TArray & st , int8 val ){
	memset( st.data(), val , st.size()*sizeof(typename TArray::value_type) );
}
template<class TArray> void initbin_array( TArray & st , size_t sz , int8 val ){
	st.resize( sz);
	memset( st.data(), val , st.size()*sizeof(typename TArray::value_type) );
}


int t_ac_text_learn::init1(){
	//darray<wchar> code2char;
	//darray<uint16> char2code; // TODO:! нужно убрать это!
	//darray<uint32> g_char2alpha;  // TODO:! нужно убрать это!
	char2code.resize(0x10000);
	c_alphabets = static_cast<int>( ar_alpha.size() );
	g_char gcc = 1;
	FOREACH( ab , ar_alpha) {
		if (!ab->chars.size()) return error.pushf(21103,"invalid char list fo alpha id %d", ab->index);
		if (ab->maxNgram > 3) return error.pushf(21103,"invalid ngram fo alpha id %d", ab->index); }
	int maxcharscount=1;
	FOREACH( ab , ar_alpha) {  maxcharscount += static_cast<int>(ab->chars.size()); };
	initbin_array(g_char2alpha , maxcharscount , -1);
	initbin_array(code2char , maxcharscount , -1);
	

	FOREACH( ab , ar_alpha) {
		ab->l_first_char = 1+c_alphabets;
		ab->l_first_alpha =  1;
		ab->alphasize = static_cast<int>(ab->chars.size() );
		ab->tr_size = ab->l_first_char + ab->alphasize; 

		initbin_array(ab->gchar2lchar , maxcharscount , -1);
		initbin_array(ab->lcode2wchar , ab->tr_size , 0);
		//darray<bool> charset;
		l_char lc = ab->l_first_char;
		FOREACH( pw , ab->chars) {
			//if (*pw==0) { throw 1; }
			auto wc = *pw; g_char gc = char2code[wc];
			if (gc==0) {
				char2code[wc] = gc = gcc; 
				g_char2alpha[gc] = ab->index;
				code2char[gc] = wc;
				gcc++;
			}
			ab->gchar2lchar[gc] = lc; 
			ab->lcode2wchar[lc] = *pw;
			lc++;
		}
		for (int i=0,szm=1;i<ab->maxNgram;i++) {
			ab->maps[i].resize( szm * ab->tr_size );
			ab->maps_size[i] = static_cast<int>(ab->maps[i].size());
			szm *=ab->alphasize;
			ab->sizeof_stk = szm; // max of szm 
		};
	};
	code2char.resize(gcc);
	return 0;
};


int t_ac_text_learn::InitFrom(conf_struct_ref jr ){
	init0();
	{auto jn = jr->getnode("learn"); if(!jn.empty()) jn->get("path",source_filepath); }
	if (!source_filepath.size()) return error.push(21101,"node 'learn/path' not found in config");
	jr->get("outfile" , result_filepath);  if (!result_filepath.size()) return error.push(21102,"node 'outfile' not found in config");
	{ auto jn=jr->getnode("ac_params"); if (!jn.empty()) {
		ac_params = json_ctx::json_print( jn );
		jn->get("rate_endchar" , rate_endchar );
	}}
	auto nabl = jr->getnode("alphabets"); 
	if (nabl.empty() || nabl->size()==0) return error.push(21102,"invalid node 'alphabets'");
	ar_alpha.resize( nabl->size() );
	for( auto pjnab : *nabl) { auto jab = pjnab.Value().toNode();
		int ida=0; jab->get("id",ida);
		if (ida<1 || ida>(int)ar_alpha.size()) return error.pushf(21103,"invalid alpha id %d", ida);
		auto ab = &ar_alpha[ida-1]; 
		ab->index=ida-1;
		jab->get("lang",ab->name);
		jab->get("name",ab->name);
		jab->get("alphabet",ab->chars);
		jab->get("ngramm",ab->maxNgram);
		jab->get("Single",ab->OneSymbolPerWord);
		if (ab->maxNgram==0) ab->maxNgram=1;
	}
	init1();
	return error.error_id;
};

int t_ac_text_learn::InitFrom( const stringA & jsontext   ){
	json_ctx jo;
	if ( jo.parse_DataOrFile( jsontext ) ) return error.push( jo.error );
	InitFrom( jo.root );
	return error.error_id;
};

int t_ac_text_learn::fordata( const stringA & data ){
	auto ws = utf8_to_wide( data );
	//tbgeneral::f_strtolower( ws.data() , ws.size());
	auto s=ws.data() ; auto e=s+ws.size();
	t_alpha* ab=0; // is local var!!
	uint stk=0, stk_ofs; wchar last=0; l_char lc;
	for( ; s< e; last=*s,s++) {
		if (*s==' ') { if (size_t(last)<sizeof(specchars) && specchars[last]!=0 ) 
				continue; }
		totalchars++;
		if (totalchars % 100000) { printworkpercent( e- ws.data(), ws.size() ); }
		auto cc= char2code[*s]; 
		if (cc==0) { 
			badchars++; continue; }

		#define INC_MAPSCODE( _lc ) \
			stk_ofs = stk * ab->tr_size;\
			ab->maps[0][_lc]++; \
			if (ab->cnt_cotinues>1 && ab->maxNgram > 1) {\
				ab->maps[1][stk_ofs % ab->maps_size[1] +_lc ]++;\
				if (ab->cnt_cotinues>2 && ab->maxNgram > 2) \
					ab->maps[2][stk_ofs % ab->maps_size[2] +_lc ]++;\
			}

		if (ab==0 || (lc=ab->gchar2lchar[cc])==-1) {
			c_alpha ai = g_char2alpha[cc];
			if (ab!=0) { auto ai_c = ai+ab->l_first_alpha;
				INC_MAPSCODE( ai_c );
			}
			ab = &ar_alpha[ai];
			ab->cnt_cotinues = 0;
			stk = 0;
			lc=ab->gchar2lchar[cc];
		}
		ab->cnt_cotinues++;
		INC_MAPSCODE(lc);
		stk = (stk * ab->alphasize + (lc - ab->l_first_char) ) % ab->sizeof_stk ;
	}

	return error.error_id;
};


int t_ac_text_learn::forfile( const stringA & fnm ){
	auto data= filesystem::readstrfromfile( fnm );
	if (!data.size()) { errored_files++; return error.pushf( 1105," empty file %s", fnm ); }

	fordata( data );

	handled_files++;
	LOG("CharsFreqTable> [ files:%d totalchars:%d ] handle file %s", handled_files ,uint(totalchars), fnm );
	return error.error_id;
};


int t_ac_text_learn::rndfiles(const stringA & dirname){
	//crsys::direxist()
	filesystem::c_directory d;
	if (!d.open( dirname )) { return error.pushf(1104,"cann`t open source_filepath %s",dirname); }
	filesystem::c_directory::c_entry e;
	while(1) {
		if (!d.getentry( e )) break;
		//efat_FILE=1,efat_DIR
		auto fullname= dirname + "/" + e.fn;
		switch ( e.type.b.etype ) {
		case filesystem::efat_FILE :  
			forfile( fullname );
			break;
		case filesystem::efat_DIR :  
			if (e.fn =="." || e.fn=="..") continue;
			rndfiles(fullname); 
			break;
		};
	};
	return error.error_id;
};

int t_ac_text_learn::init_ac( c_arithmetic_coding_ngramm  & ac ){
	ac.reset(static_cast<int>(ar_alpha.size()) );
	FOREACH( pa , this->ar_alpha) {
		r_alphabet_ngramm ab;
		auto pach = pa->getchars();
		ab.initialize( pa->getid() , pa->maxNgram , this->c_alphabets , pach );
		ab.OneSymbolPerWord = pa->OneSymbolPerWord;
		ab.name = pa->name;
		//auto alsz = pa->alphasize; 
		auto strsz=pa->tr_size;
		int seq=0;
		for (int ng=0;ng< pa->maxNgram;ng++) {
			auto m=pa->maps[ng];
			for (int lseq=0;lseq<(int)m.size();lseq += strsz ) {
				auto dt = pa->maps[ng].slice(lseq , strsz );
				auto a_tr = dt.slice( pa->l_first_alpha , this->c_alphabets );
				auto a_chars= dt.slice( pa->l_first_char , pa->alphasize );
				ab.init_set_trans(seq , 0, a_tr , a_chars );
				seq++;
			}
		};
		ac.add_alphabet( &ab );
	}
	ac.init_set_fin_weigth( this->rate_endchar ); //TODO:
	//TODO: set params
	ac.prepare_alphabet_set();
	if (ac.err()) return error.push(ac.error);
	return error.error_id;
};

int t_ac_text_learn::save_srcsetting(conf_struct_ref ns){
		ns->set( "totalchars", int(totalchars));
		ns->set( "badchars", int(badchars));
		ns->set( "totalfiles", handled_files);
		ns->set( "errored_files", errored_files);
		ns->set( "count_symbols", (uint) code2char.size() );
		ns->set( "srcdir", source_filepath );
		return 0;
};

int t_ac_text_learn::saveresult_buff( stringA & outbuff , int saveopt){
	using t_AC = c_arithmetic_coding_ngramm;
	t_AC ac;
	if (init_ac( ac )) return error.error_id;
	json_ctx jo; 
	jo.new_root();
	save_srcsetting( jo.root->set_node("source") );
	if (saveopt==-1) saveopt=t_AC::eBJO_DefaultMask | t_AC::eBJO_UseInputData;
	ac.savetojson( jo.root , saveopt );
	outbuff = jo.save();
	return error.error_id;

};

int t_ac_text_learn::saveresult(const stringA & _outfile , int saveopt){
	stringA buff;
	result_filepath = _outfile.size() ? _outfile : result_filepath;
	if (saveresult_buff( buff , saveopt )) return error.error_id;
	if( 0==filesystem::writestr2file( result_filepath , buff ) )
		return error.pushf(11126, "error on save to file %s" , result_filepath );
	LOG("%s save freq table to %s", ( error.error_id?"ERROR: on ":"") ,  result_filepath  );
	return error.error_id;

};


int t_ac_text_learn::make( const stringA & _path , const stringA & outfile  ){

	//if (init()) { return error.error_id;}
	if (error.error_id) return error.error_id;
	auto path =   _path.size() ? _path : source_filepath;
	source_filepath = path;
	if (0==filesystem::direxist( path )) {
		rndfiles( path );
	} else if (0==filesystem::fileexist( path )) {
		forfile( path );
	} else { 
		error.pushf(10326,"t_ac_text_learn::make file or directory not exists -> %s", path )	;
	}
	if (totalchars<10000) return error.pushf(1103,"total chars count in files %d<10000", int(totalchars) ); 
	//if (finish()) return error.error_id;
	if (outfile.size()) if (saveresult(outfile)) 
		return error.error_id;
	return error.error_id;
};

}; //namespace tbgeneral{





namespace tbgeneral{ namespace test{ namespace ArithmeticCoding{


static const wchar_t *  jsontest_alphabets=L"{\
'learn':{'path': './data/text/en_ru/lt1.txt' },\
'outfile':'./data/freqs/testac_result.json',\
'ac_params':{'start_alpha':1,'alpha_has_terminal_char':1,'minimum_frequency':1, 'rate_endchar':0.0005 },\
'alphabets':[\
	{	'id':1,\
		'lang':'ru',\
		'symbols':36,\
		'alphabet':' ,.абвгдеёжзийклмнопрстуфхцчшщъыьэюя', \
		'ngramm':2}\
	,{ 	'id':2,\
		'lang':'en', \
		'symbols':29, \
		'alphabet':' ,.abcdefghijklmnopqrstuvwxyz',\
		'ngramm':2}\
	,{ 	'id':3,\
		'lang':'ru.UCASE',\
		'symbols':33,  \
		'alphabet':'АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ',\
		'Single':true}\
	,{ 	'id':4,\
		'lang':'en.UCASE',\
		'symbols':26, '1/26':0.03846,\
		'alphabet':'ABCDEFGHIJKLMNOPQRSTUVWXYZ',\
		'Single':true}\
	,{ 	'id':5,\
		'lang':'Цифры',\
		'symbols':14, '1/14' : 0.07142,\
		'alphabet':' .:-0123456789'}\
	,{ 	'id':6,	\
		'lang':'Другие символы',\
		'symbols':39, '1/39':0.02564,\
		'alphabet':' (){}[]&*+-/=<>^|~!,.:;?«»\\'#$%@§©·№_±\\\\',\
		'Single':true}\
		]}";	

stringA get_ac_initjson_0data( int id  ){
	auto sa = stringA(jsontest_alphabets);
	sa = replacechars( sa, "'" , "\"" );
	return sa;
}


void test_ac_ngramm_learn() {
	using t_AC = c_arithmetic_coding_ngramm;
	t_ac_text_learn lrn;
	auto sa = get_ac_initjson_0data(0);
	if ( lrn.InitFrom( sa ) ) { LOG("%s", lrn.error.text); return; };
	if ( lrn.make("c:/temp/lt1.txt") ) 
		{ LOG("%s", lrn.error.text); return; };
	//return;
	if ( lrn.saveresult("c:/temp/lt1.json" , t_AC::eBJO_FloatTransition | t_AC::eBJO_DefaultMask) ) 
		{ LOG("%s", lrn.error.text); return; };
} 


}}}; //namespace tbgeneral::test{ 