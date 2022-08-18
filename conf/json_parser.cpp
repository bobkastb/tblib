#define _CRT_SECURE_NO_WARNINGS
#include "json_parser.h"


// ------ END HEADER --------------
#include <gdata/tbt_locale.h>
#include "gdata/tb_vector_cs.h"
#include <math.h>
#include "tsys/tbfiles.h"
#include "gdata/t_format.h"
#include "parse/t_path.h"
#include "parse/t_parse.h"
#include "gdata/tb_numbers.h"


#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif

//#define FOREACH( It , St) for(auto It=St.begin();It!=St.end();It++)

namespace tbgeneral{
using str_t = stringA;

#define bibb(s) throw s

//static const char* variant_type_names[] = { "None", "Bool",  "Int" , "Float" , "Str", "Object", "Array" };
static stringA variant_type_names[] = { "None", "Bool",  "Int" , "Float" , "Str", "Object", "Array" };
stringA conf_variant_types::getvartype_name(eVarType tp) {
	//static stringA getvartype_name(eVarType tp);
	return variant_type_names[tp];
}


// ************ Экранирование символов ***********
// \b  Backspace (ascii code 08)
// \f  Form feed (ascii code 0C)
// \n  New line
// \r  Carriage return
// \t  Tab
// \"  Double quote
// \\  Backslash character
bool json_screening_test( const stringA & d ){
	auto r= memchr(d.data() , '\\', d.size()); return r!=0; 
}
stringA json_ctx::json_encode_screening( const stringA & d ){
	static cBitSetC<256> specchars("\\\"\x9\r\n\x0C\x08");
	int cntin=0; for (auto s=d.cbegin() ;s<d.cend();s++) { cntin += int( specchars.test(byte(*s))); };
	if (!cntin) { return d; }
	stringA res; res.reserve(d.size());
	for (auto s=d.cbegin();s<d.cend();s++) {
		switch (s[0]) {
		case '\\': res<<"\\\\";break;
		case '"': res<<"\\\"";break;
		case '\x09': res<<"\\t";break;
		case '\r': res<<"\\r";break;
		case '\n': res<<"\\n";break;
		case '\x0C': res<<"\\f";break;
		case '\x08': res<<"\\b";break;
		default: res <<s[0];
		}
	}
	return res;
}
stringA json_decode_screening( const stringA& d ){
	if (!json_screening_test(d)) { return d; }
	stringA res; res.reserve(d.size());
	for (auto s=d.cbegin() ;s<d.cend();s++) { 
		if (*s!='\\') { res<<*s; continue; }
		s++;
		switch (s[0]) {
		case 't': res<<'\x09';break;
		case 'r': res<<'\r';break;
		case 'n': res<<'\n';break;
		case 'f': res<<'\x0C';break;
		case 'b': res<<'\x08';break;
		default : res << s[0];
		}
	}
	return res;
};

bool json_ctx::FilenameIsData( const stringA & fn){
	return (find_str(fn , "{").size()!=0);
};





//static const char * variant_type_names[]={"None", "Bool",  "Int" , "Float" , "Str", "Object", "Array" };





struct json_parse_ctx :public json_ctx_defs {
	//using str_t = conf_variant::str_t;
	//using key_t = conf_variant::key_t;
	enum eLexems{
		LEXInvalid=0,LEXEndSymbol, 
		LEXBool, LEXStr,LEXInt ,LEXFloat, LEXOpenObj,LEXCloseObj,LEXOpenArr,LEXCloseArr,LEXSeparator,LEXAssign
	};
	
	json_ctx * owner;
	conf_struct_ref  root , cnode;
	darray<conf_struct_ref> nstack;
	const char * data,*datastart,*end; 
	//int sz;
	t_error_info error;

	int linesnumber; const char *linestart;
	//jsstr curr_lexem;
	conf_variant curr_lex_variant;
	conf_variant::str_t  currstr_forlexem;
	const char* pstartlexem;
	eLexems currLexemID;

	json_parse_ctx() { ZEROMEM(*this); }
	void set_error_id( int err); 
	int parse();
	void pushnode(conf_variant::eVarType jtype, const key_t & key );
	void pushnode(conf_struct_ref newn , const key_t& key);
	int popnode(conf_variant::eVarType jt ); // array or object
	eLexems parseLexem(  );
	eLexems WaitStringLexem( );
	eLexems WaitNumericLexem( );
	eLexems WaitLiteralLexem(  );
	str_t MakeStr(const char* s, const char* e) { return  owner->data.slice(s, e); }
	void sprint(stringA & buff, int level);

	str_t get_current_lexem_info();
private:
	eLexems parseLexem_(  ); // private

};





int json_ctx::parsedata(const stringA & d) {
	clear();
	data = d; 
	json_parse_ctx ctx;
	ctx.owner = this;
	ctx.error=t_error_info();
	ctx.datastart= ctx.data = data.cbegin();
	ctx.end = data.cend();
	ctx.parse();
	error = ctx.error;
	root = ctx.root;
	if ( error.error_id==0 &&  0!=root->exists("<!expand_environment>") ){
		expand_all_env_vars();
	}
	//TODO: make error info
	return ctx.error.error_id;
};




int json_ctx::savefile(const stringA & filename , json_print_options * opts){
	auto buff = save( opts );
	if (error.error_id) return error.error_id;
	if( 0==filesystem::writestr2file( filename , buff ) )
		return error.pushf(11126, "error on save to file %s" , filename );
	return 0;

};


void json_ctx::fixup_option( json_print_options * opts , bool doset){
	if (!opts) return; 
	key_t lnm;
	FOREACH (pk , opts->shortopts) {
		auto pv = root->get_path_ptr(pk->first);
		//auto n= getpre( pk->first , lnm ); if (!n)  continue; 
		//auto pv =  n->get_ptr(lnm); if (!pv) continue;
		if (!pv) continue;
		auto opt=  &pk->second;
		pv->opt = doset ? byte( opt->mask ) : 0; 
		auto sn = pv->toNode(); 
		if (!sn.empty()) 
			sn->print_option = doset ? opt : 0; 
		//if (n)  n->print_option = doset ? &pk->second : 0; 
	}

};

stringA json_ctx::save(json_print_options * opts){
	stringA buff; 
	if (root.empty()) return buff;  
	fixup_option( opts , true );
	buff.reserve( data.size() ); 
	json_print( root , buff,0 , 0 );
	fixup_option( opts , false );
	return buff;
};


bool json_ctx::testNoPrint(const conf_variant& var) {
	return var.opt & conf_variant::ejtNoPrint;
}

void json_ctx::json_print_simple(const conf_variant& var , stringA & buff ) {
//stringA json_variant::toJStr()const{ // только для простых типов
	switch (var.type) {
	case conf_variant::ejtBool: buff << (var.d.b ? "true" : "false"); return;
	case conf_variant::ejtInt: return format_to(buff , "%d", var.d.i); return;
	case conf_variant::ejtFloat: return format_to(buff , "%f", var.d.f); return;
	case conf_variant::ejtStr: buff << '"' << json_encode_screening(var.d.str) << '"'; return;  // TODO: Экранировка!
	}
}

//stringA json_variant::sprint( ){	stringA buff; sprint(buff,0); return buff; };
void json_ctx::json_print(const conf_variant & var, stringA& buff, int level, json_print_options* opts) {
//void json_variant::sprint(stringA & buff, int level, json_print_options * opts){
	if (var.isObjectType() ) {
		json_print ( var.toNode() ,  buff, level +1 , opts );
	} else {
		json_print_simple( var , buff );
	}
};

stringA json_ctx::json_print(const conf_struct_ref& node) {
	stringA buff;
	json_print(node, buff, 0, 0);
	return buff;
};

void json_ctx::json_print(const conf_struct_ref& node , stringA& buff, int level, json_print_options* opts) {

//void json_node::sprint(stringA & buff, int level,json_print_options * opts){
	auto eoln='\n';
	auto startalgn = getSpaceAlignedString(level*2);
	auto nextalign = getSpaceAlignedString((level+1)*2);
	json_print_option po(1);
	if (node->print_option) 
		po = *node->print_option;
	if (node->stype == conf_struct_t::ejtObject ) {
		buff << "{";
		int cntinline=1; bool nl=true; char sep=0;
		for (uint i=0;i< node->named_props.size();i++,cntinline++) {
			auto p=&node->named_props[i];
			if (testNoPrint(p->data)) { continue; }
			if (sep)  buff<<','; 
			sep=1;
			if (nl) buff<<eoln;
			buff<<(nl ? nextalign : " ")<<'"'<<json_encode_screening(p->key)<<"\":";
			//exists
			json_print( p->data , buff ,  level+1 , opts );
			//if (i<named_props.size()-1) { buff<<','; }
			nl= 0==(cntinline % po.values_per_line);
		}
		if (!nl) buff<<eoln;
		buff << startalgn << "}";
	} else if ( node->stype == conf_struct_t::ejtArray ) {
		// test if sample array (int,bool,nums,string)  <  20
		auto a_cnt= node->a_data.size(); //, a_end=a_cnt-1;
		int cot =0;  for (uint i=0;i<a_cnt;i++ ) cot+=int(node->a_data[i].isObjectType());
		char sep=0;
		if (cot>0) {
			buff << "[";
			for (uint i=0;i<a_cnt;i++ ) {
				if (sep)  buff<<','; 
				sep=1;
				buff<<eoln<<nextalign;
				json_print( node->a_data[i] , buff , level+1 , opts );
				//if (i<a_end) { buff<<','; }
				//buff<<eoln;
			}
			buff << eoln << startalgn << "]";
		} else {
			buff << "[";
			auto stsz=buff.size(); uint limit=140;
			for (uint i=0;i<a_cnt;i++ ) {
				if (sep)  buff<<','; 
				sep=1;
				if (buff.size()>stsz + limit ) {
					buff<<eoln<<nextalign;
					stsz=buff.size();
				}
				json_print( node->a_data[i] , buff , level+1 , opts );
				//if (i<a_end) { buff<<','; }
			}
			buff <<  "]";
		}
	}

};



/*
int json_owner::parsefile(const wchar * filename){
	datafilename = filename;
	auto buff = filesystem::readstrfromfile(filename);
	if (buff.fdata==0) { return error.pushf(jeFileNotRead , " Error read file %s", stringA(filename)); }
	//TODO:error
	return parsedata(buff);
};
*/


str_t json_parse_ctx::get_current_lexem_info(){
//	jsstr currstr_forlexem;
//	eLexems currLexemID;
	switch (currLexemID) {
	case LEXInvalid:  return currstr_forlexem;
	case LEXEndSymbol:  return "EOF";
	default: 	return currstr_forlexem;
	}
}; 

void json_parse_ctx::set_error_id( int err) {
	error.error_id= err;
	//linesnumber++; linestart
	const char* sinf="";
	switch ( error.error_id ) {
	case json_ctx::jeExpectedCS:  sinf="Expected close scope '}' "; break;
	case json_ctx::jeExpectedCA:  sinf="Expected close array ']' "; break;
	case json_ctx::jeUnExpectedClose: sinf="Unexpected close char"; break;
	case json_ctx::jeExpectedOS : sinf="expected open scope '{' "; break;
	case json_ctx::jeInvalidLexem: sinf="invaid lexem "; break;
	case json_ctx::jeExpectedData: sinf="expected data lexem (string,int,object ...etc) "; break;
	case json_ctx::jeUnExpectedEOF: sinf="unexpected EOF stream"; break;
	case json_ctx::jeExpectedSeparator: sinf="expected separator ',' "; break;
	case json_ctx::jeExpectedAssign: sinf="expected assign ':' "; break;
	case json_ctx::jeExpectedField: sinf="expected object field name "; break;
	case json_ctx::jeInvalidClosureEV: sinf="invalid closure for environment vars"; break;
	case json_ctx::jeUnknownEnvVars : sinf="unknown environment variable"; break;
	}
	auto lpos = nsparse::t_linesinfo::calc( datastart, pstartlexem );
		//get_linesinfo( this , pstartlexem  );
	error.text = format("json syntax error:at [line:%d pos:%d] (id:%d): %s . Last Lexem is <", 
			lpos.line+1  , lpos.col+1 , error.error_id, sinf );
	error.text << get_current_lexem_info() << ">";
	
}
json_parse_ctx::eLexems json_parse_ctx::WaitStringLexem(  ){
	for (auto s=data;!error.error_id && data<end; ) {	auto ch=*data; data++;
		if (ch=='\\') { 
			data++; continue; }
		if (ch=='"') {
			//if (data[-2]=='\\') continue;
			//избавиться от экранировки! 
			currstr_forlexem= owner->data.slice(s,data-1);
			auto nstr= json_decode_screening( currstr_forlexem );
			curr_lex_variant = conf_variant( nstr );
			return LEXStr;
		}
	}
	return LEXInvalid;
};
json_parse_ctx::eLexems json_parse_ctx::WaitNumericLexem( ){
	//static char* termset="}]	\x0a\x0d"
	//auto st=data;
	int64 n=0; double cntdeg=0; double nf; int tp=0; int sign=1 , signE=1;
	auto s=data;
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
	currstr_forlexem= owner->data.slice(s,data);
	switch ( tp ){
	case 0: n*=sign; curr_lex_variant = conf_variant( int(n) ); return LEXInt ;
	case 1: nf =sign*(nf+ n/pow(10,cntdeg)); curr_lex_variant = conf_variant( nf ); return LEXFloat;
	case 2:	nf=sign*nf*pow(10,signE*n); curr_lex_variant = conf_variant( nf ); return LEXFloat;
	}
	return LEXInvalid;
};

json_parse_ctx::eLexems json_parse_ctx::WaitLiteralLexem( ){
	const stringA clexems[]={  "false" , "true" };
	auto s=data;
	for (; !error.error_id && data<end ; data++) {	auto ch=*data; 
	if (!((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'))) { break; }
	}
	currstr_forlexem= owner->data.slice(s,data);
	for (uint i=0;i<ARRAYLEN(clexems);i++) {
		if (0==tbgeneral::f_stricmp( s , clexems[i].data() , data-s, clexems[i].size() )) {
			curr_lex_variant = conf_variant( bool(i==1) );
			return LEXBool;
		}
	}
	return LEXInvalid;
};
json_parse_ctx::eLexems json_parse_ctx::parseLexem(  ){
	pstartlexem=data;
	currstr_forlexem = str_t();
	currLexemID = parseLexem_();
	if (currstr_forlexem.empty() ) 
		currstr_forlexem= owner->data.slice(pstartlexem,data);
	return currLexemID;
}
json_parse_ctx::eLexems json_parse_ctx::parseLexem_(  ){
	while (!error.error_id && data<end) {
		auto ch=*data; data++;
		switch (ch) {
		case '"': return WaitStringLexem();
		case '{': return LEXOpenObj;
		case '}': return LEXCloseObj;
		case '[': return LEXOpenArr;
		case ']': return LEXCloseArr;
		case ',': return LEXSeparator;
		case ':': return LEXAssign;
		case '-': data--; return WaitNumericLexem();
		case 0x0A: linesnumber++; linestart=data;
		case ' ': case '	':  case 0x0D: break;
		default:
			if (ch>='0' && ch<='9') {
				data--; return WaitNumericLexem();
			} else if (ch>='a' && ch<='z') {
				data--; return WaitLiteralLexem();
			} else {
				return LEXInvalid;
			}
		}; // switch (ch)
	}
	return LEXEndSymbol;
};

void json_parse_ctx::pushnode(conf_struct_ref newn , const key_t& key ){
	if (cnode.data()) { 
		cnode->push_back( key , conf_variant(newn) ); 
	};
	cnode= newn;
	nstack.push_back( newn );
};
void json_parse_ctx::pushnode( conf_struct_t::eVarType jtype, const key_t & key ){
	auto nn = conf_struct_t::newnode(jtype);
	pushnode( nn , key );
};

int json_parse_ctx::popnode(conf_struct_t::eVarType jt ){
	if ( cnode->stype !=jt ) { set_error_id(jt== conf_struct_t::ejtArray? jeExpectedCS :jeExpectedCA ); return error.error_id; }
	conf_struct_ref n;
	if (!nstack.pop_back(n)) { set_error_id(jeUnExpectedClose); return error.error_id; };
	if (nstack.size()>0) { cnode = nstack[nstack.size()-1]; } else { cnode= conf_struct_ref(); }
	return error.error_id;
}; // array or object

int json_parse_ctx::parse(){
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
				case LEXBool:case LEXStr: case LEXInt: case LEXFloat: 
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
				if (lid==LEXStr || lid== LEXInt ) {
					if (lid== LEXInt) 
						lkey = currstr_forlexem;
						else lkey = curr_lex_variant.toStr();
					lid = parseLexem( );
					if (lid==LEXAssign) {state= stWaitDataNC;
					} else set_error_id(jeExpectedAssign);  // expected ':'
				} else set_error_id(jeExpectedField);   // expected 'field'
				break;
		}; 
		
	}; // while 
	return error.error_id;
};


// набор строк для выравнивания текста в начале строки пробелами




}; // namespace tbgeneral{


//-------------------------------- OUT stream-------------------
namespace tbgeneral {
ojsonstream& operator << (ojsonstream& s, double v) {
	char buff[128]; sprintf(buff, "%f", v);
	s.push_datastr(buff);
	return s;
};

ojsonstream& operator << (ojsonstream& s, int32_t v) {
	char buff[128]; sprintf(buff, "%d", v);
	s.push_datastr(buff);
	return s;
}
ojsonstream& operator << (ojsonstream& s, uint32_t v) {
	char buff[128]; sprintf(buff, "%u", v);
	s.push_datastr(buff);
	return s;
}

void ojsonstream::pushbin(const char_type* data, size_t sz) {
	if (scope.id == stateCLOSE)
		makeerror("tojson: stream closed!");
	push(data, sz);
};
void ojsonstream::before() {
	if (scope.id == stateARR || waittag == tagName) {
		if (scope.datacnt) pushbin(", ");
	}
};
void ojsonstream::endData() {
	waittag = scope.id == stateARR ? tagData : (waittag == tagName) ? tagData : tagName;
	scope.datacnt++;
};
void ojsonstream::endName() {
	pushbin(":"); waittag = tagData;
};
void ojsonstream::push_strtype(const char_type* data, size_t strsize) {
	before();
	pushbin("\"");	push(data, strsize); pushbin("\"");
	if (waittag == tagData) endData();
	else endName();
};
void ojsonstream::push_datastr(const char_type* data, size_t strsize) {
	before();
	if (waittag != tagData) makeerror("tojson:parametr name expected!");
	push(data, strsize);
	endData();
};
void ojsonstream::init() {
	//pushbin("{"); scope.id = stateObj; scope.datacnt = 0;  waittag = tagName;
	waittag = tagData;
	setScope(scB_OBJ);
};
void ojsonstream::close() {
	if (scope.id != stateCLOSE) setScope(scE_OBJ);
	if (scope.id != stateCLOSE)
		makeerror("tojson: stream not closed!");
};
void ojsonstream::push_etext(text_special_e t) {
	char buff[4] = { 0 };
	buff[0] = (char)t;
	pushbin(buff);
};

void ojsonstream::setScope(scope_types nsc) {
	if (scope.id == stateNone && nsc != scB_OBJ)
		makeerror("json: closed!");
	if (nsc == scB_ARR || nsc == scB_OBJ) {
		if (waittag != tagData) makeerror("tojson:unexpected new scope!");
		before();
		stack.push_back(scope);
		scope.datacnt = 0;
	}
	switch (nsc) {
	case scB_OBJ:	pushbin("{"); scope.id = stateObj; waittag = tagName; break;
	case scB_ARR:	pushbin("["); scope.id = stateARR; waittag = tagData; break;
	case scE_OBJ:
		if (waittag != tagName || scope.id != stateObj) makeerror("tojson:invalid close scope!");
		pushbin("}");
		break;
	case scE_ARR:
		if (waittag != tagData || scope.id != stateARR) makeerror("tojson:invalid close scope!");
		pushbin("]");
		break;
	}
	if (nsc == scE_ARR || nsc == scE_OBJ) {
		stack.pop_back(scope);
		if (scope.id == stateNone)  scope.id = stateCLOSE;
		waittag = tagData;
		endData();
	}
};


}; // namespace tbgeneral{


