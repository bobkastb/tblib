#include "tbcfg_parse.h"
#include "parse/t_parse.h"
#include <math.h>

namespace tbgeneral{namespace tbcfg_ns{

enum { char_LE = 10, char_LF = 13, char_SPACE = ' ' };
static bool char_inSYM(char c) { return (c >= '0' && c <= '9') || ((byte)c > 128) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'); }
static bool char_inSYMe(char c) { return (c >= '0' && c <= '9') || ((byte)c > 128) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c == '-'); }



}} // namespace tbgeneral{

namespace tbgeneral {namespace tbcfg_ns{

//conf_struct_ref load_tbconfig_ini_list(const char* text, size_t textsz) {
conf_struct_ref load_tbconfig_ini_list(const stringA & text) {
	conf_struct_ref res, subl;
	subl= res = conf_struct_t::newnode(conf_struct_t::ejtObject);
	

	auto se = text.cend();
	const char* ss = text.cbegin(), * ls, * le, * s; // *se=text+textsz
	while (ss<se) {
		for (ls = ss, le = ls; le<se ; le++) {
			if (*le == 0) { ss = le; break; }
			if (*le == char_LE || *le == char_LF) { ss = le; for (; (*ss) && (*ss == char_LE || *ss == char_LF); ss++); break; }
		};
		for (; *ls <= char_SPACE && ls < le; ls++);
		if (ls == le) continue;
		if (!(char_inSYMe(*ls) || (*ls == '['))) continue;
		for (s = ls; s < le && (*s != '[') && (*s != '='); s++);
		if (*s == '[') {
			const char* e, * s1;
			for (e = s; e < le && (*e != ']'); e++);
			if (*e != ']') continue;
			for (; s < e && (!char_inSYMe(*s)); s++);
			for (s1 = s; s1 < e && (char_inSYMe(*s1)); s1++);
			if (s >= s1) continue;

			auto key = text.slice( s, s1 );
			subl = conf_struct_t::newnode(conf_struct_t::ejtObject);
			res->push_back(key , subl);
			//res->
		}
		else if (*s == '=') {
			const char* e, * s1;
			if (subl.empty()) continue;
			for (s1 = ls; s1 < s && (char_inSYMe(*s1)); s1++);
			if (s1 == ls) continue;
			for (s++; s < le && (*s <= ' '); s++);
			for (e = le; s < e && (e[-1] <= ' '); e--);
			auto key = text.slice(ls, s1);
			auto val = text.slice(s, e);
			subl->push_back(key, val );
		};

	};
	return res;
}


}};

namespace tbgeneral {namespace tbcfg_ns{

struct hoconT_parse_ctx :public conf_parser_defs_based {

	enum {
		jeExpectedCS = 13001,
		jeExpectedCA = 13002,
		jeUnExpectedClose = 13003,
		jeExpectedOS = 13004,
		jeInvalidLexem = 13005,
		jeExpectedData = 13006,
		jeUnExpectedEOF = 13007,
		jeExpectedSeparator = 13008,
		jeExpectedAssign = 13009,
		jeExpectedField = 13010,

		//jeEVMustBeString,
		//jeInvalidClosureEV,
		//jeUnknownEnvVars,
		//jeFileNotRead
	};

	//using str_t = conf_variant::str_t;
	//using key_t = conf_variant::key_t;
	enum eLexems{
		LEXInvalid=0,LEXEndSymbol, 
		LEXBool, LEXStr,LEXInt ,LEXFloat, LEXOpenObj,LEXCloseObj,LEXOpenArr,LEXCloseArr,LEXSeparator,LEXAssign
		,LEXIdent
	};
	
	//json_ctx * owner;
	conf_struct_ref  root , cnode;
	darray<conf_struct_ref> nstack;
	const char * data,*datastart,*end; 
	stringA storage;
	LocalEnv * locenv;
	//int sz;
	t_error_info error;

	int linesnumber; const char *linestart;
	//jsstr curr_lexem;
	conf_variant curr_lex_variant;
	conf_variant::str_t  currstr_forlexem;
	const char* pstartlexem;
	eLexems currLexemID;

	hoconT_parse_ctx() { ZEROMEM(*this); }
	void set_error_id( int err); 
	int parse();
	void pushnode(conf_variant::eVarType jtype, const key_t & key );
	void pushnode(conf_struct_ref newn , const key_t& key);
	int popnode(conf_variant::eVarType jt ); // array or object
	eLexems parseLexem(  );
	eLexems WaitStringLexem( char endchar );
	eLexems WaitNumericLexem( );
	eLexems WaitLiteralLexem(  );
	eLexems WaitNumericLexem_Hex( );
	void WaitLineComment();
	str_t MakeStr(const char* s, const char* e) { return  storage.slice(s, e); }
	//void sprint(stringA & buff, int level);

	str_t get_current_lexem_info();
private:
	eLexems WaitStrAfterLiteral( const char * litstart  );
	eLexems parseLexem_(  ); // private
};

hoconT_parse_ctx::str_t hoconT_parse_ctx::get_current_lexem_info(){
//	jsstr currstr_forlexem;
//	eLexems currLexemID;
	switch (currLexemID) {
	case LEXInvalid:  return currstr_forlexem;
	case LEXEndSymbol:  return "EOF";
	default: 	return currstr_forlexem;
	}
}; 

void hoconT_parse_ctx::set_error_id( int err) {
	error.error_id= err;
	//linesnumber++; linestart
	const char* sinf="";
	switch ( error.error_id ) {
	case jeExpectedCS:  sinf="Expected close scope '}' "; break;
	case jeExpectedCA:  sinf="Expected close array ']' "; break;
	case jeUnExpectedClose: sinf="Unexpected close char"; break;
	case jeExpectedOS : sinf="expected open scope '{' "; break;
	case jeInvalidLexem: sinf="invaid lexem "; break;
	case jeExpectedData: sinf="expected data lexem (string,int,object ...etc) "; break;
	case jeUnExpectedEOF: sinf="unexpected EOF stream"; break;
	case jeExpectedSeparator: sinf="expected separator ',' "; break;
	case jeExpectedAssign: sinf="expected assign ':' "; break;
	case jeExpectedField: sinf="expected object field name "; break;
	case jeInvalidClosureEV: sinf="invalid closure for environment vars"; break;
	case jeUnknownEnvVars : sinf="unknown environment variable"; break;
	}
	auto lpos = nsparse::t_linesinfo::calc( datastart, pstartlexem );
		//get_linesinfo( this , pstartlexem  );
	error.text = format("json syntax error:at [line:%d pos:%d] (id:%d): %s . Last Lexem is <", 
			lpos.line+1  , lpos.col+1 , error.error_id, sinf );
	error.text << get_current_lexem_info() << ">";
	
}
void hoconT_parse_ctx::WaitLineComment(){
	data=nsparse::goto_nextline( data , end );
}
uint reverse_bytes(uint32_t x){
	x = x>>24 | ((x>>8) & 0xFF00) | ((x<<8) & 0xFF0000) | x<<24;
	//for (;x && ((x & 0xFF)==0); x>>=8); 
	return x;
}; 

hoconT_parse_ctx::eLexems hoconT_parse_ctx::WaitStrAfterLiteral( const char * litstart  ){
	auto type_sz =data-litstart; 
	if (type_sz > 4) return LEXInvalid;
	auto ch=*data; data++;
	auto res = WaitStringLexem( ch );

	// define string type
	auto cod= *((uint32_t*)litstart);
	cod = cod  << ((4-type_sz)*8);
	cod = reverse_bytes(cod);
	switch (cod) {
		case 'E' : {
			auto& refstr = curr_lex_variant.refStr();
			locenv->expand(refstr,refstr); }; break;
		default: return LEXInvalid;
	}
	return res;
};

hoconT_parse_ctx::eLexems hoconT_parse_ctx::WaitStringLexem( char endchar   ){
	for (auto s=data;!error.error_id && data<end; ) {	auto ch=*data; data++;
		if (ch=='\\') { 
			data++; continue; }
		if (ch==endchar) {
			currstr_forlexem= storage.slice(s,data-1);
			auto nstr= nsparse::unescape_std_str( currstr_forlexem );
			curr_lex_variant = conf_variant( nstr );
			return LEXStr;
		}
	}
	return LEXInvalid;
};
hoconT_parse_ctx::eLexems hoconT_parse_ctx::WaitNumericLexem_Hex( ){
	auto s=data;
	data+=2;
	uint64_t ivalue=0;
	if (s[0]!='0' && s[1]!='x') return LEXInvalid;
	for ( ; (!error.error_id && data<end) ; data++ ) {
		auto ch = *data;
		if (ch>='0' && ch<='9') { ivalue=(ivalue<<4)+(ch-'0');
		}else if (ch>='A' && ch<='F') { ivalue=(ivalue<<4)+(ch-'A'+10);
		}else if (ch>='a' && ch<='f') { ivalue=(ivalue<<4)+(ch-'a'+10);
		}else break;
	};
	currstr_forlexem= storage.slice(s,data);
	curr_lex_variant = conf_variant( ivalue );
	return data>s+2 ? LEXInt : LEXInvalid;
};

hoconT_parse_ctx::eLexems hoconT_parse_ctx::WaitNumericLexem( ){
	//static char* termset="}]	\x0a\x0d"
	//auto st=data;
	int64 n=0; double cntdeg=0; double nf; int tp=0; int sign=1 , signE=1;
	auto s=data;
	if (data[0]=='0' && data[1]=='x') {
		return WaitNumericLexem_Hex();
	}; 
	if (*data=='-') { sign=-1; data++;}
	while (!error.error_id && data<end) {	auto ch=*data; data++;
		if (ch>='0' && ch<='9') { n=n*10+(ch-'0'); cntdeg++;
		} else if (ch=='.') { 
			if (tp!=0) { return LEXInvalid; }
			tp=1; nf=double(n);n=0; cntdeg=0;
		} else if (ch=='e' || ch=='E') {
			if (tp!=1) { return LEXInvalid; }
			nf = nf+ n/pow(10,cntdeg);
			if (*data=='-') { signE=-1; data++; 
			} else if (*data=='+') { signE=1; data++; }
			n=0;
			tp=2;
		} else { data--;break; }
	}
	currstr_forlexem= storage.slice(s,data);
	switch ( tp ){
	case 0: n*=sign; curr_lex_variant = conf_variant( int(n) ); return LEXInt ;
	case 1: nf =sign*(nf+ n/pow(10,cntdeg)); curr_lex_variant = conf_variant( nf ); return LEXFloat;
	case 2:	nf=sign*nf*pow(10,signE*n); curr_lex_variant = conf_variant( nf ); return LEXFloat;
	}
	return LEXInvalid;
};

hoconT_parse_ctx::eLexems hoconT_parse_ctx::WaitLiteralLexem( ){
	const stringA clexems[]={  "false" , "true" };
	auto s=data;
	for (; !error.error_id && data<end ; data++) {	auto ch=*data; 
		if (ch=='"' || ch=='\'') { return WaitStrAfterLiteral(s); }
		if (!nsparse::char_inSYMe(ch)) break;
	}
	currstr_forlexem= storage.slice(s,data);
	for (uint i=0;i<ARRAYLEN(clexems);i++) {
		if (0==tbgeneral::f_stricmp( s , clexems[i].data() , data-s, clexems[i].size() )) {
			curr_lex_variant = conf_variant( bool(i==1) );
			return LEXBool;
		}
	}
	curr_lex_variant = conf_variant( currstr_forlexem );

	return LEXIdent;
};
hoconT_parse_ctx::eLexems hoconT_parse_ctx::parseLexem(  ){
	pstartlexem=data;
	currstr_forlexem = str_t();
	currLexemID = parseLexem_();
	if (currstr_forlexem.empty() ) 
		currstr_forlexem= storage.slice(pstartlexem,data);
	return currLexemID;
}
hoconT_parse_ctx::eLexems hoconT_parse_ctx::parseLexem_(  ){
	while (!error.error_id && data<end) {
		auto ch=*data; data++;
		switch (ch) {
		case '"':case '\'': return WaitStringLexem(ch);
		case '{': return LEXOpenObj;
		case '}': return LEXCloseObj;
		case '[': return LEXOpenArr;
		case ']': return LEXCloseArr;
		case ';': case ',': return LEXSeparator;
		case '=': case ':': return LEXAssign;
		case '/':{if (*data=='/') WaitLineComment();else return LEXInvalid;	break;}
		case '#': WaitLineComment();break;
		case '-': data--; return WaitNumericLexem();
		case 0x0A: linesnumber++; linestart=data;
		case ' ': case '	':  case 0x0D: break;
		default:
			if (ch>='0' && ch<='9') {
				data--; return WaitNumericLexem();
			} else if ( nsparse::char_inSYM(ch) ) {
				data--; return WaitLiteralLexem();
			} else {
				return LEXInvalid;
			}
		}; // switch (ch)
	}
	return LEXEndSymbol;
};

void hoconT_parse_ctx::pushnode(conf_struct_ref newn , const key_t& key ){
	if (cnode.data()) { 
		cnode->push_back( key , conf_variant(newn) ); 
	};
	cnode= newn;
	nstack.push_back( newn );
};
void hoconT_parse_ctx::pushnode( conf_struct_t::eVarType jtype, const key_t & key ){
	auto nn = conf_struct_t::newnode(jtype);
	pushnode( nn , key );
};

int hoconT_parse_ctx::popnode(conf_struct_t::eVarType jt ){
	if ( cnode->stype !=jt ) { set_error_id(jt== conf_struct_t::ejtArray? jeExpectedCS :jeExpectedCA ); return error.error_id; }
	conf_struct_ref n;
	if (!nstack.pop_back(n)) { set_error_id(jeUnExpectedClose); return error.error_id; };
	if (nstack.size()>0) { cnode = nstack[nstack.size()-1]; } else { cnode= conf_struct_ref(); }
	return error.error_id;
}; // array or object

int hoconT_parse_ctx::parse(){
	enum etState {stWaitData=1, stWaitField , stWaitSeparator , stWaitDataNC , stWaitFirst };
	key_t lkey;
	etState  state=stWaitData;
	error=t_error_info();
	linesnumber=0; linestart=data;

	if ( LEXOpenObj != parseLexem( ) ) {
		set_error_id(jeExpectedOS); // вначале ожидается только '{'
	}else {
		pushnode(conf_struct_t::ejtObject , "");
		root = cnode;
		state=stWaitField;
	};
	while (!error.error_id && data<end) {
		auto lid = parseLexem( );
		if (lid==LEXEndSymbol) { break; }
		if (lid==LEXInvalid) { set_error_id(jeInvalidLexem);  break; }
		if (lid==LEXCloseObj || lid==LEXCloseArr) {
			if (state==stWaitDataNC) { set_error_id(jeExpectedData); break; }
			//TODO: end object-data
			if ( popnode( lid==LEXCloseObj? conf_struct_t::ejtObject : conf_struct_t::ejtArray ) ) break;
			if (cnode.empty() && (parseLexem( )!=LEXEndSymbol))  { set_error_id(jeUnExpectedEOF); break; }
			state=stWaitSeparator;
			continue;
		}

		switch (state) {
			case stWaitFirst: break; //TODO:
			case stWaitData: case stWaitDataNC:
				switch( lid ) {
				case LEXOpenObj: 
					pushnode(conf_struct_t::ejtObject , lkey );state=stWaitField; break;
				case LEXOpenArr: 
					pushnode(conf_struct_t::ejtArray , lkey ); state=stWaitData; break;
				case LEXBool:case LEXStr: case LEXInt: case LEXFloat: case LEXIdent:
					cnode->push_back( lkey , curr_lex_variant );
					state=stWaitSeparator; 
					break;
				default: set_error_id(jeExpectedData); break;// unexpected lexem
				};
				lkey=key_t();
				break;
			case stWaitSeparator: 
				if (lid == LEXSeparator) {
					state= (cnode->stype == conf_struct_t::ejtArray )  ? stWaitDataNC : stWaitField;
				} else set_error_id(jeExpectedSeparator); // expected separator
				break;
			case stWaitField: 
				if (lid==LEXStr || lid== LEXInt || lid==LEXIdent ) {
					lkey =(lid== LEXInt) ? currstr_forlexem : curr_lex_variant.toStr();
					lid = parseLexem( );
					switch (lid) {
						case LEXAssign: state= stWaitDataNC;break;
						case LEXOpenObj: pushnode(conf_struct_t::ejtObject , lkey );state=stWaitField; break;
						case LEXOpenArr: pushnode(conf_struct_t::ejtArray , lkey ); state=stWaitData; break;
						default:set_error_id(jeExpectedAssign); // expected ':'
					} 
					
				} else set_error_id(jeExpectedField);   // expected 'field'
				break;
		}; 
		
	}; // while 
	return error.error_id;
};



}}; // namespace tbgeneral {namespace tbcfg_ns{


/* hocon
*  отличия от json не велики.
* 1) WaitLiteralLexem - любой литерал допустим
* 2) Новая лексема LEXIdent
* 3) оператор LEXAssign перед { или [
*/ 

namespace tbgeneral {
int hocon_parse_ctx::parsedata(const stringA& _data) {
	clear();
	data = _data;
	tbcfg_ns::hoconT_parse_ctx ctx;
	ctx.storage = data;
	ctx.locenv = &this->locenv;
	ctx.error = t_error_info();
	ctx.datastart = ctx.data = data.cbegin();
	ctx.end = data.cend();
	ctx.parse();
	error = ctx.error;
	root = ctx.root;

	//if (error.error_id == 0 && 0 != root->exists("<!expand_environment>")) { expand_all_env_vars();	}
	//TODO: make error info
	return error.error_id;

};
bool hocon_parse_ctx::FilenameIsData(const stringA& fn) {
	return (find_str( fn, "{").size() != 0);
};

bool ini_parse_ctx::FilenameIsData(const stringA& fn) { return (find_str(fn, "[").size() != 0);  };
int ini_parse_ctx::parsedata(const stringA& data) {
	root = tbcfg_ns::load_tbconfig_ini_list(data);
	return 0;
};


} // namespace tbgeneral{
