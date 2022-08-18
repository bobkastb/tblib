#include "tbconf.h"
#include "tsys/tbfiles.h"
#include <string>
#include <stdlib.h>
#include <errno.h>
#include "tsys/tb_sysfun.h"
#include "gdata/t_format.h"
#include "gdata/tb_numbers.h"
#include "parse/t_path.h"
#include "parse/t_parse.h"

warning_MSVC( disable , 4996 )
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-result"
#endif



//namespace tbgeneral {namespace tbconf{
namespace tbgeneral {

using strtype = stringA;
enum {
	charseq_x0=0x7830,	
	charseq_ER=0x4552,	
	charseq_CSV=0x00435356,	
};


//static size_t readfiletostring( const char * fn , strtype & out);
static size_t readfiletostring(const param_stringA& fn, strtype& out);
//std::string & operator << (std::string & s , const std::string & x) { s.append(x); return s;}
//std::string & operator << (std::string & s , const char * x) { s.append(x); return s;}



size_t tbconfig::getsizeofval(  char* nm  ){ 
		auto r= &eget(nm);
		return 0!=r ? r->_value.size() : 0;
};

bool tbconfig::getvalue( const char* nm , const char* & value){
	auto r= &eget(nm);
	value = 0!=r ? (const char *) r->_value.cbegin() : 0;
	return 0!=r;
};
bool tbconfig::getvalue( const char* nm , strtype & value ){
	auto r= &eget(nm);
	value= 0!=r ?  r->_value : 0;
	return 0!=r;
};
bool tbconfig::getvalue( const char* nm  , int & value ){
	auto  r= &eget(nm);
	value=0;if (!r) return false;
	int er= atoi_t( r->_value ,value );
	if (er) 
		RAISE( "bad int in params");
	return true;
};
bool tbconfig::getvalue( const char* nm  , uint & value ){
	int v;
	bool res=getvalue(nm , v ); 
	if (res) { value = v; };
	return res;
};
bool tbconfig::getvalue( const char* nm  , uint64 & value ){
	int v;	
	bool res=getvalue(nm , v ); 
	if (res) { value = v; };

	return res;
};
bool tbconfig::getvalue( const char* nm  , bool & value ){
	int v;
	bool res=getvalue(nm , v ); 
	if (res) { value =  v!=0; };
	return res;
};

bool tbconfig::getvalue( const char* nm  , listptr & value ){
	auto r= &eget(nm);
	value=0!=r ? r->_sublist : listptr();
	return value.data()!=0 ;
};

int tbconfig::getvalue_int(  char* nm   ){
	int res; if (!getvalue(nm,res)) RAISE( "bad int in params");
	return res;
};


const tbconfig::relem & tbconfig::eget_(const keytype & key ) const{
	const relem * e=0;
	for (size_t i=0;i<list.size();i++) {
		if (0==tbgeneral::stricmp( key , list[i]._key )) return list[i];
	};
	int ki , en=atoi_t( key ,ki ); 
	if (en||ki<0||ki>=(int)list.size()) 
		return *e;
	return list[ki];
}; 
const tbconfig::relem & tbconfig::eget(const keytype & key ){
	const relem & x= eget_(key);
	return * (relem*) &x;
}; 

const tbconfig::relem & tbconfig::eget(const keytype & key ) const{
	return eget_(key);
}; 
//relem & get(const keytype & key ); 

const tbconfig::relem & tbconfig::get(const keytype & key ) const{
	listptr l;
	const tbconfig* cc=this;
	keytype k; const char * p=key.cbegin() , *pc;
	const relem * e=0 , *ze=0; bool end=false;
	for (;(!end) && (p);p=pc+1)	{ 
		pc=strchr( p , '/' ); end=!pc;
		if (end) pc=p+strlen(p); 
		if (pc<=p) continue;
		if (!cc) return *ze;
		k =  key.slice( p, pc ); 
		e = &cc->eget( k ); if (!e) return *e;
		l=e->_sublist; cc=l.data();
	};
	return *e;
	//if ()
}; 

// ---------------











//};
//namespace tbconf{

struct r_loadctx{
	//uint errcnt;
	void error(const strtype & s) { 
			const strtype es=s;  RAISE( "%s",es) ; 
		};
	};

enum enum_LEXEMS{ L_BAD=0 , 
	   L_SCLEFT , //{
	   L_SCRIGTH ,//}
	   L_ASSIGN , // =
	   L_COMA , // ,
	   L_PCOMA , // ;
	   L_STR , // ''
	   L_CTRSTR,  // X''
	   L_SQLEFT , //[
	   L_SQRIGTH //]
};
enum {char_LE=10 ,char_LF=13 , char_SPACE=' ' };

struct r_lexem { enum_LEXEMS id; 
		const char * data , * e; 
		uint linenum , linepos;  
		uint datatypeid;
		//strtype datar;
		bool has_escape;

		void push_bord( const char * up) {
			//if (up > data) datar.append(data, up - data);
			//data = up + 1;
			e = up;
		};

		//strtype gets() {return datar.size() ? datar : strtype( data , e>data ? size_t(e-data) : 0 );	}
		void reset() {
			data = e = 0; id = L_BAD; linenum = linepos = datatypeid = 0; has_escape = false;
			//datar=strtype(); 
		};
		r_lexem() {reset();};
	};

struct r_lexer{
	uint lstate;
	r_lexem * cl;
};

struct r_parser{
	r_loadctx ctx;
	r_lexem m;
	const char * linestart , * starttext;
	stringA fdata;

	void setdata(const stringA& data) {
		fdata = data;
		starttext = data.cbegin();
		m.data = data.cbegin();
		m.e = data.cend();

	}


	void error(const strtype& s) { 
		 strtype ss(linestart , nsparse::goto_endofline(linestart,m.e) - linestart );
		 ctx.error( tbgeneral::format("Syntax error: %s at line %d pos %d \n grab:'%s' \n" , s , m.linenum , m.linepos , ss ));
	}

	bool charinSYM(char c) { return nsparse::char_inSYM(c); }
	bool wait_endstring( r_lexer & lex, char c );
	bool waitid( r_lexer & lex);
	void onlineend(){
		m.data=nsparse::goto_nextline(m.data,m.e );
		linestart =  m.data; m.linenum++;
	};
	void goto_eoln() { onlineend(); }
	void onescchar(r_lexer & lex);
	void fill_lineinfo( r_lexem & l ){
		l.linenum = m.linenum;
		l.linepos = std::max<int>(0, int(l.data - linestart) );
	};
	bool retlexem(r_lexer & lex , uint lid ){
		r_lexem lx; lx.id =(enum_LEXEMS) lid; lx.data = m.data; lx.e= lx.data+1; fill_lineinfo(lx);
		lex.cl[0] = lx; m.data++;
		return true;
	};
	void defineStrtype(r_lexer & lex);
	//void pushlexem(r_lexem & lx) { lx.push_bord(m.data); };
	bool getnexttag(  r_lexem & tag );
};


bool r_parser::wait_endstring(r_lexer & lex ,  char endchar ){ 
	r_lexem & lx= lex.cl[0];
//#define PUSHLEXEM() { if (m.data>lx.data) lx.datar.append( lx.data , m.data-lx.data); lx.data=m.data+1;} 
	
	//lx.datar.resize(0);
	m.data++;lx.data=m.data;
	//lx.id = L_STR; 
	fill_lineinfo( lx );
	for(;m.data<m.e;m.data++) {  char c=m.data[0];
		if (c==char_LE || c==char_LF) { 
			//lx.push_bord(m.data);  
			onlineend(); 
		} if (c=='\\') { 	
			lx.has_escape = true;
			//lx.push_bord(m.data); onescchar(lex);
			m.data++;
		} if (c==endchar) { 
			lx.push_bord(m.data);
			m.data++; 
			if (charinSYM(m.data[0])) error("unexpected char at end of data");		
			return true;
		} else continue;
	};
	return false; 
};
bool r_parser::waitid( r_lexer & lex){ r_lexem & lx= lex.cl[0];
	lx.id = L_STR; lx.data=m.data;
	for(;m.data<m.e;m.data++) {  char c=m.data[0];
		if (charinSYM(c)) {
		}else if (c=='\'' || c=='"' ){
			defineStrtype(lex);
			auto wr=wait_endstring(lex, c);
			lx.id=L_CTRSTR;
			return wr;
		}else { lx.e=m.data;
			fill_lineinfo( lx );
			return true;
		}; 
	};
	return false;
};

uint reverse_intchar(uint x){
	x = x>>24 | ((x>>8) & 0xFF00) | ((x<<8) & 0xFF0000) | x<<24;
	for (;x && ((x & 0xFF)==0); x>>=8); 
	return x;
}; 

void r_parser::defineStrtype(r_lexer & lex){
	const char * e= m.data , *st= lex.cl->data; 
	if (e<=st) { lex.cl->datatypeid=0; return; };
	if (st+4<e) st= e-4;
	size_t sz = e>st ? e - st : 0;
	uint tid = lex.cl->datatypeid;
	tid = (* (uint32*) st) & ( uint32(-1) >> ((4-sz)*8)  );
	tid = reverse_intchar(tid);
	lex.cl->datatypeid=tid;
};
bool r_parser::getnexttag(  r_lexem & tag ){
	r_lexer lex; lex.lstate = 0; lex.cl = &tag;	tag.reset(); 
	
	for(;m.data<m.e;m.data++){ char c=m.data[0];
		switch (c) {
		case char_LE:case char_LF: 
					onlineend(); m.data--; break;
		case '\'' : case '"': 
					lex.cl->id = L_STR; return wait_endstring( lex, c );   
		case '=':	return retlexem(lex,L_ASSIGN); 
		case ';':	return retlexem(lex,L_PCOMA); 
		case ',':	return retlexem(lex,L_COMA); 
		case '{':	return retlexem(lex,L_SCLEFT); 
		case '}':	return retlexem(lex,L_SCRIGTH); 
		case '/':	if (m.data[0]=='/') {goto_eoln(); m.data--; }  break;
		case '#':	goto_eoln();m.data--; break;
		default:	if (charinSYM(c)) return waitid( lex );
		};
	};
	return false;
	};

struct r_listparser{
	r_parser * p;
	r_lexem  lexem; 
	listptr res;
	uint celstate;
	tbconfig::relem cel;
	 
	strtype currlexem2str() { 
		auto s= p->fdata.slice(lexem.data, lexem.e);
		return s;
		//lexem.gets();
	}
	void setcellval() { cel._value = currlexem2str(); celstate=3; }
	void closecell() {
		if (celstate==0) return;
		if (celstate==2) p->error("expected value ! "); 
		if (celstate==1) { cel._value=cel._key; cel._key=""; };
		res->push_back( cel );
		cel = tbconfig::relem();
		celstate=0;
	};
};

listptr load_tbconfig_list(  r_parser & p  ){
	r_listparser ctx; 
	ctx.res.create(); 
	ctx.celstate=0; ctx.p=&p;
	
	while (p.getnexttag(ctx.lexem)) {
		switch (ctx.lexem.id){
			case L_ASSIGN:	if (ctx.celstate!=1) 
								p.error("unexpected assign '='! "); 
							ctx.celstate=2; 
							break;	
			case L_COMA: case L_PCOMA: ctx.closecell(); break;
			case L_SCLEFT:  
							if (ctx.celstate>1) p.error("unexpected start list '{'! "); 
							ctx.cel._sublist=load_tbconfig_list(p); 
							ctx.celstate=3; ctx.closecell();  
							break;
			case L_SCRIGTH: ctx.closecell(); 
							return ctx.res; 
			case L_CTRSTR:  if (ctx.celstate==1) p.error("expected end of cell ';'! or assign '=' "); 
							ctx.cel.tag= ctx.lexem.datatypeid;
							ctx.cel._value = ctx.currlexem2str(); ctx.celstate=3;
							break;
			case L_STR:		switch (ctx.celstate) {
								case 0:{ ctx.cel._key = ctx.currlexem2str(); ctx.celstate=1; };  break;
								case 1:p.error("expected end of cell ';'! or assign '=' ");   break;
								case 2:{ ctx.cel._value = ctx.currlexem2str(); ctx.celstate=3; };break;
								default: p.error("unexpected str const ! ");
							};
							break;
		};
	}; // while 
	p.error("expected end of list '}'! ");
	return listptr();
};
//****************************************************
struct r_linkctx{
	bool result;
	listptr basel; 
	uint cntupdates;
	strtype errlist;
	strtype cfgdir;
	r_linkctx() { cntupdates=0;result=true; }
	void error( uint errid , strtype r) { 
		result = false;
		switch (errid) {
		case 1: errlist << "unresolved reference "<<r; break;
		case 2: errlist << "data error "<<r; break;
		};
		errlist <<" \n";
	};
	void rndsblist(listptr l);
	void reparsenode( tbconfig::relem & e );
	void parsedata( tbconfig::relem & e );
	int readbfile( const strtype & name , strtype & data );
	strtype expand_env_str(const strtype & v);
};



strtype r_linkctx::expand_env_str(const strtype & v){
	strtype res=v;
	res = replace_str( res , "%prg%" ,  filesystem::get_programm_dir() );
	res = replace_str( res , "%search%" , cfgdir );
	return res;
};
int r_linkctx::readbfile( const strtype & name , strtype & data ){
	strtype nm = expand_env_str(name );
	return (int)readfiletostring( nm , data );

};
void r_linkctx::parsedata( tbconfig::relem & ee ){
	const char * s = ee._value.cbegin() , *e=ee._value.cend() , *ss;
	const char * c09x = "0123456789abcdefABCDEF" ; int err=0; 
	strtype buff;
	 

	for (;s<e;) {
		s = strpbrk( s , c09x ); if (s>=e||!s) break;
		if ( *(short*)s == charseq_x0 ) s+=2; // 'x0'
		ss =s+strspn(s,c09x); if (ss>=e) {err=1; break; }
		size_t esz=ss-s;
		if (esz>16 ||  0==(0x00114 & (1<<esz) ) ) {err=1; break; }
		char * es; uint d=strtoul(s,&es,16);
		buff.append( (char*)&d , esz/2 );
		s=ss;
	};
	if (err) error( 2 , ee._key );
	ee._value = buff;
};
void r_linkctx::reparsenode( tbconfig::relem & e ){
	if (!e._value.size()) return; 
	tbconfig::relem * fe ;
	switch (e.tag) { 
		case 'D': { parsedata(e); }; e.tag=0; break;
		case 'F': {	uint sz=readbfile(e._value , e._value );
					e.tag= sz ? 0 : charseq_ER ; // file read
				  };break; 	
		case 'E':case charseq_CSV:{ // expand file name
					e._value= expand_env_str( e._value );
					e.tag=0;
				  };break; 	
		case 'L': { e.tag=0; // cicles warning
					fe =(tbconfig::relem *) &basel->get(e._value); 
					if (fe) { if (fe->tag) reparsenode(*fe);
						e._value = fe->_value;
						e._sublist = fe->_sublist;
					} else error( 1 , e._value );
				  }; break;
	};
	
};
void r_linkctx::rndsblist(listptr l){ 
	tbconfig & lc = *l; 
	if (!&lc) return;
	if (lc.itag) return; 
	lc.itag=1; // cicle warning
	for (size_t i=0;i<lc.size();i++) {
		if (lc[i]._sublist.data()) rndsblist( lc[i]._sublist );
		else if (lc[i].tag) reparsenode(lc.list[i]);
	};
};
bool tbconfig_relinks(listptr l , r_linkctx & ctx){
	ctx.basel = l;
	ctx.rndsblist(l);
	return true;
};

//****************************************************


listptr load_tbconfig_fromtxt( const stringA & text ,const stringA & file){
	r_parser p;r_lexem  lexem; listptr res;
	p.setdata( text );
	if ( find_str( text, "{").size()==0 ) return res;
	if (!p.getnexttag(lexem) || (lexem.id !=L_SCLEFT))   return res;
	res = load_tbconfig_list( p );
	r_linkctx ctx;
	ctx.cfgdir = (file.size() ? tbgeneral::get_file_dir( file ) : "");
	tbconfig_relinks( res, ctx );
	
	if (!ctx.result) {
		//printf("error at link config:\n %s", ctx.errlist);
		RAISE(ctx.errlist.c_str() );
	}

	return res;
};
static size_t readfiletostring( const param_stringA & fn , strtype & out){//TODO:!-> filesystem
	auto res= filesystem::readstrfromfile(fn );
	if (res.size()) out=res;
	return out.size();
};
listptr load_tbconfig( const stringA & fn ){ 
	listptr res; strtype buff;
	if (!readfiletostring( fn , buff )) return res;
	return load_tbconfig_fromtxt( buff ,fn );
};

//------------------------------ INI FILE ----------------------------------


listptr load_tbconfig_ini_list( const char * text , size_t textsz ){
	listptr res  , subl;
	res.create();
	const char * ss=text ,  *ls,*le , *s; // *se=text+textsz
	while (*ss) {
		for (ls = ss, le=ls;  ; le++) {
			if (*le==0) { ss=le; break;}
			if (*le==char_LE || *le==char_LF) { ss=le; for (; (*ss)&&(*ss==char_LE || *ss==char_LF) ; ss++); break; }
		};
		for (;*ls<=char_SPACE && ls<le;ls++); 
		if (ls==le) continue;
		if (! (nsparse::char_inSYMe(*ls) || (*ls=='[') )) continue;
		for (s=ls;s<le && (*s!='[') && (*s!='=');s++); 
		if (*s=='[') { 
			 const char *e,*s1;
			 for(e=s;e<le && (*e!=']');e++); 
			 if (*e!=']') continue; 
			 for (;s<e && (!nsparse::char_inSYMe(*s));s++); 
			 for (s1=s;s1<e && (nsparse::char_inSYMe(*s1));s1++); 
			 if (s>=s1) continue;
			 tbconfig::relem sub;
			 sub._key.assign( s , s1-s );
			 sub._sublist.create(); subl=sub._sublist;
			 res->push_back(sub);
			 //res->
			}
		else if (*s=='=') { 
			const char *e,*s1;
			if (!subl.data()) continue;
			for (s1=ls;s1<s && (nsparse::char_inSYMe(*s1));s1++);  
			if (s1==ls) continue;
			for (s++;s<le && (*s<=' ');s++); 
			for (e=le;s<e && (e[-1]<=' ');e--);
			tbconfig::relem sub;
			sub._key.assign( ls , s1-ls );
			sub._value.assign( s , e-s );
			subl->push_back(sub);
		};

	};
	return res;
}

listptr load_tbconfigINIF_fromtxt( const stringA & text ,const stringA & file){
	listptr res;
	res = load_tbconfig_ini_list( text.cbegin() , text.size() );
	
	if (!res.data()) {
		RAISE(" configuration not loaded (%s)",file);
		//RAISE(ctx.errlist.c_str());
	}

	return res;
};

listptr load_tbconfigINIF( const stringA & fn )
{
	listptr res; strtype buff;
	if (!readfiletostring( fn , buff )) return res;

	return load_tbconfigINIF_fromtxt( buff ,fn );
};

listptr tbconfig::parse_tbconf( const stringA & dataOrfile ,  LocalEnv * env ){
	auto isData = 0<= dataOrfile.find('{');
	return isData ? load_tbconfig_fromtxt(dataOrfile,0) : load_tbconfig(dataOrfile) ;
};



} // namespace tbgeneral
//}};//namespace tbconf