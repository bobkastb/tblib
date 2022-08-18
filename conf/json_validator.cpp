#include "json_parser.h"
#include "tsys/tbfiles.h"
#include "gdata/t_format.h"




//------------- END HEADER ------------
#define FOREACH( It , St) for(auto It=St.begin();It!=St.end();It++)


namespace tbgeneral{
using jvte = json_validator;
using str_t = conf_variant::str_t;

struct json_tester_node;
typedef json_tester_node * p_json_tester_node;
struct json_tester_node{
	conf_variant::eVarType type;
	uint32 compatable_types;
	stringA compatable_types_list;
	bool mandatory;
	bool noempty; // for string, array,object
	darray<int> lengthrange;
	double range[2];
	darray<p_json_tester_node> sub_element;
	spmap<int> named_element;
	int anyname_element; // element with name "..." 
	json_tester_node() { 
			range[1]=-1;  
			anyname_element=0;
			compatable_types=0; 
			type= conf_variant::ejtNone;
			mandatory=false; 
			noempty=false; 
	}
	bool isObjectType() { return type== conf_variant::ejtObject || type== conf_variant::ejtArray; }
	~json_tester_node();
};
struct json_tester_ctx{
	json_tester_node * root;
	stringA stackpath; 
	t_error_info * err;
	void push( int index, const str_t & fname );
	void pop();

	int test_node(  json_tester_node* sch , conf_struct_ref  n );
	int test_variant( int index, const str_t & fname, json_tester_node* sch , const conf_variant* n );
	int test_variant( json_tester_node* sch , const conf_variant* n );
	void set_error(int err_id,const stringA & inf=0);
	int geterror() { return err->error_id; };
	json_tester_ctx() { ZEROMEM(*this); }
	int init( json_ctx * jo);
	int initschema( json_tester_node * vn , conf_struct_ref jn );
	int initschemavalue( json_tester_node * vn , conf_struct_ref  jn );
	void reset() { stackpath=""; }
	void clear() { delete root; root=0; }; 
	~json_tester_ctx() { clear(); }
};

void json_tester_ctx::push( int index, const str_t & fname ){
	stackpath << "/";
	if (index<0) {  stackpath << fname; } else { stackpath << format("%d",index); }
};
void json_tester_ctx::pop(){
	auto s=stackpath.data(), c=s+ stackpath.size()-1;
	for (; s<=c; c--)
		if (*c=='/') { stackpath.resize(c-s); return; }
};

static stringA printSimpleVariant( const conf_variant* n ) {
	return format("[%s] = '%s' ", n->type , n->toString()); //TODO:toJStr - not need
}

inline stringA f_tpname( uint t) { return conf_variant::getvartype_name(conf_variant::eVarType(t)); }


int json_tester_ctx::test_variant( json_tester_node* sch , const conf_variant* n ){
	if (!sch->isObjectType() && !( sch->compatable_types & (1<< n->type) )) { 
		//TODO: typelist
		set_error(jvte::jveUncompatableTypes , format("expected %s, but met %s", sch->compatable_types_list, f_tpname(n->type)) ); return -1; }
	switch (sch->type) {
		case conf_variant::ejtBool: return 0;
		case conf_variant::ejtInt: case conf_variant::ejtFloat:
			// check range
			return 0;
		case conf_variant::ejtStr:
			if (sch->noempty && (n->toStr().size()==0)) { 
				set_error(jvte::jveDisableEmpptyValue ); }
			return 0;
		case conf_variant::ejtObject: case conf_variant::ejtArray:
			auto jn = n->toNode();
			uint tp = !jn.empty() ? jn->stype : n->type;
			if (sch->type!= tp ) {  
				set_error(jvte::jveUncompatableTypes , format("expected %s, but met %s", f_tpname(sch->type), f_tpname(tp) ) ); return -1; }
			return test_node( sch , jn );
	}
	return -1;
};
int json_tester_ctx::test_variant( int index, const str_t& fname, json_tester_node* sch , const conf_variant* n ){
	push(index,fname) ;
	auto res = test_variant( sch , n );
	pop();
	return res;
};

int json_tester_ctx::test_node( json_tester_node* sch , conf_struct_ref n ){
	if (sch->type != n->stype ) { set_error(jvte::jveUncompatableTypes ); return geterror(); }
	if (sch->type == conf_variant::ejtArray ) {
		int index=0;
		//FOREACH( p , n->a_data) {  
		for( auto pv : *n ) {
			bool tested=false; 
			auto p = &pv.Value();
			FOREACH( _s , sch->sub_element ) { auto nsch=*_s;
				if ( nsch->compatable_types & (1<< p->type) ) { test_variant( index, 0,  nsch , p ); tested=true; break; }
			}; 
			index++;
			if (!tested) set_error(jvte::jveUncompatableTypes , format("/%d  %s",0,printSimpleVariant( p ) ) ); // TODO: uncompatable types  0???
		}
	} else {
		FOREACH( pp , sch->named_element ){
			auto field=pp->first; auto nsch= sch->sub_element[pp->second];
			auto d_v =  n->get_ptr(field);
			if (!d_v)  { if (nsch->mandatory) { set_error(jvte::jveOutOfMandatory , field );}; continue; }
			test_variant( -1, field,  nsch , d_v );
		}
		if (sch->anyname_element >0) {
			auto nsch= sch->sub_element[sch->anyname_element]; int checks=0;
			for( auto p : *n){ 	
				if (exists(sch->named_element, p.Key())) { continue; }
				checks++;
				test_variant( -1, p.Key() ,  nsch , &p.Value());
			}
			if (!checks) { if (nsch->mandatory) { set_error(jvte::jveOutOfMandatory ,"..." );};  }
		}
	}
	return 0;
};

void json_tester_ctx::set_error(int err_id,const stringA & inf){
	const char * erridt="";
	switch (err_id) {
	case jvte::jveUncompatableTypes : erridt="uncompatable types"; break;
	case jvte::jveDisableEmpptyValue : erridt="disable empty value"; break;
	case jvte::jveOutOfMandatory : erridt="expected mandatory parameter"; break;
	case jvte::jveRangeError : erridt="out of range"; break;
	case jvte::jvelUndefinedType : erridt="undefined type"; break;
	case jvte::jvelInvalidFormat2Type: erridt="invalid format for 'type' defenition"; break;
	};
	err->pushf(12060,"error:[%d] %s:(path)%s : %s\n" ,err_id, erridt , this->stackpath, inf );
};

int json_validator::validate_json( json_ctx * jo ){
	auto tst= (json_tester_ctx*) (ctx);
	if (!tst || !tst->root) { throw "json validator not load!"; }
	err.reset();
	tst->reset();
	tst->test_node( tst->root , jo->root );
	return err.error_id;
};

int json_validator::validate_file( const stringA & data ){
	err.reset();
	auto buff = filesystem::readstrfromfile(data);
	if (!buff.size()) 
		return err.pushf( 12068 , "error load file: %s " , data );
	return validate_data(buff);

};
int json_validator::validate_data( const stringA & data ){
	err.reset();
	json_ctx jo;
	jo.parsedata( data );
	if (jo.error.error_id) 
		return err.pushf( err.error_id , "validator load data error:%s" , jo.error.text  );	
	validate_json(&jo);
	return err.error_id;
};

int json_validator::load_schema( const stringA & data ){
	err.reset();
	json_ctx jo ;
	jo.parse_DataOrFile(data);
	if (jo.error.error_id) {
		return err.pushf( jo.error.error_id, "error parse json stream: %s  file:%s" , jo.error.text , data ); 
	}
	auto tctx= new json_tester_ctx;
	tctx->err = &err;
	tctx->init( &jo );
	if (err.error_id) return err.error_id;
	ctx= tctx;
	return err.error_id;
};
json_tester_node::~json_tester_node(){
	FOREACH( p , sub_element) 
		delete p[0];
	sub_element.resize(0);
};

int json_tester_ctx::init( json_ctx * jo){
	root=new json_tester_node;
	initschemavalue( root , jo->root  );
	return err->error_id;
};

int json_tester_ctx::initschemavalue( json_tester_node * vn , conf_struct_ref jn ){
	vn->type = jn->stype;
	vn->compatable_types = 1 << vn->type;
	if (jn->stype == conf_variant::ejtArray) {
		int index=0;
		for( auto pv : *jn ){
			auto vsn = new json_tester_node;
			vn->sub_element.push_back( vsn );
			push(index,0);
			initschema( vsn , pv.Value().toNode() );
			pop();
			index++;
		}	
	} else {
		vn->sub_element.push_back( 0 ); 
		for( auto pr : *jn ){
			auto & key = pr.Key();
			auto vsn = new json_tester_node;
			vn->sub_element.push_back( vsn );
			if (key == "...") {
				vn->anyname_element=static_cast<int>(vn->sub_element.size()-1);
			} else 
				vn->named_element[key] = static_cast<int>(vn->sub_element.size()-1);
			push(-1,key);
			initschema( vsn , pr.Value().toNode() );
			pop();
		}
	}
	return 0;
};

template <class E> darray<E> get( const conf_struct_ref  & jn , stringA key ){
	darray<E> res;
	auto la= jn->get<conf_struct_ref>(key);
	if (la.empty() || la->stype!=conf_variant::ejtArray ) { return res; }
	for( auto p : *la ){
		auto v = p.Value().get<E>(); 
		res.push_back(v);
	}
	return res;
}

int json_tester_ctx::initschema( json_tester_node * vn , conf_struct_ref jn ){
	conf_struct_ref valn;
	valn= jn->get<conf_struct_ref>("object");
	if (!valn.empty()) initschemavalue( vn , valn );
	vn->mandatory = jn->getdef<bool>("mandatory" , false);
	vn->lengthrange = get<int>( jn , "length");

	auto tv = jn->get_ptr("type");
	if (!tv && !vn->type) { 
		set_error(jvte::jvelUndefinedType ); return 1; }
	if (tv) {
		//auto tp = (tv->isObjectType()) ? tv->toNode()->stype : tv->type ;
		auto tp =tv->type;
		if (tp == conf_variant::ejtArray) {
			uint tm=0; auto n = tv->toNode();
			for( auto pv : *n ) { 
				auto p=&pv.Value();
				tm |= 1<< p->type; 
				vn->compatable_types_list<<f_tpname(p->type); 
				vn->type = conf_variant::eVarType( p->type );
			}
			vn->compatable_types = tm;
		} else if (tp == conf_variant::ejtObject) {
			set_error(jvte::jvelInvalidFormat2Type );
		} else {
			vn->compatable_types = 1<< tp;
			vn->type = conf_variant::eVarType( tp );
		}  
		vn->noempty = (tv->type == conf_variant::ejtStr) && ( tv->toStr()!="" );
	};
	if (!vn->compatable_types_list.size()) { vn->compatable_types_list<< f_tpname(vn->type); }
	// TODO: range 
	return err->error_id;
};

json_validator::~json_validator(){
	auto tst= (json_tester_ctx*) (ctx);
	delete tst;
	ctx=0;
};

//-----------------------------

void cfg_cmp_context_t::init(const stringA& i_paths, const stringA& i_keys, const stringA& start_comment) {
	auto ra = split_str(i_paths, ",");
	for (auto& f : ra)
		ignore_path[f] = 1;
	ra = split_str(i_keys, ",");
	for (auto& f : ra)
		ignore_keys[f] = 1;
	ra = split_str(start_comment, "|");
	ZEROMEM(comment_firstchars);
	for (auto& f : ra) {
		auto fc = f[0]; if (fc > 127 || fc < 0) continue;
		comment_firstchars[fc] = 1;
	}


};

bool cfg_cmp_context_t::isIgnored(const stringA& path, const stringA& key) {
	return exists(ignore_path, path) || exists(ignore_keys, key);
}
bool cfg_cmp_context_t::is_commentkey(const stringA& key) {
	switch (key[0]) {
	case '/':case '-': return true;
	}
	return false;
};
int cfg_cmp_context_t::cmpv(const conf_variant& vf, const conf_variant& vb) {
	//if (vf.type!=vb.type)   return 
	//ejtNone, ejtBool, ejtInt, ejtFloat, ejtStr, ejtObject, ejtArray
	conf_variant::eVarType unitype[] = { conf_variant::ejtNone, conf_variant::ejtBool, conf_variant::ejtInt, conf_variant::ejtInt, conf_variant::ejtStr };
	if (unitype[vf.type] != unitype[vb.type])
		return err.pushf(1, "value type not match");
	auto reseq = false;
	switch (vf.type) {
	case conf_variant::ejtBool: reseq = vf.d.b == vb.d.b; break;
	case conf_variant::ejtInt: reseq = vf.toInt() == vb.toInt() || vf.toFloat() == vb.toFloat(); break;
	case conf_variant::ejtFloat: reseq = vf.toFloat() == vb.toFloat() || vf.toFloat() == vb.toInt(); break;
	case conf_variant::ejtStr: reseq = vf.toStr() == vb.toStr(); break;
	}
	if (!reseq)
		return err.pushf(2, "value not match");
	return 0;
};
int cfg_cmp_context_t::cmp_json(const stringA& json_l, const stringA& json_r) {
	json_ctx jown_l, jown_r;
	if (jown_l.parse_DataOrFile(json_l)) return err.push("1.invalid json");
	if (jown_r.parse_DataOrFile(json_r)) return err.push("2.invalid json");
	return cmp(jown_l.root, jown_r.root, "");
}

int cfg_cmp_context_t::cmp(const conf_struct_ref& nf, const conf_struct_ref& nb, const stringA& cpath = "") {
	if (nf->stype != nb->stype)
		return err.push(3, "parent node type not match");
	size_t i = -1; int cmpres = 0;
	auto newpath = cpath + (cpath.empty() ? "" : "/"); auto sz = newpath.size();
	for (auto& cnf : *nf) {
		i++;
		auto k = cnf.Key();
		if (!k.empty() && is_commentkey(k)) continue;
		newpath.resize(sz); newpath.append(k);
		if (isIgnored(newpath, k)) { continue; }
		auto vb = k.empty() ? nb->getvar(i) : nb->getvar(k);
		auto vf = cnf.Value();
		if (vb.type == conf_struct_t::ejtNone && vf.type)
			return err.pushf(4, "node not exists. path:%s", newpath);
		if (vf.isObjectType()) {
			if (vf.type != vb.type)
				return err.pushf(5, "child node-object uncompatable type. path:%s", newpath);
			cmpres = cmp(vf.toNode(), vb.toNode(), newpath);
		}
		else cmpres = cmpv(vf, vb);
		if (cmpres)
			return err.pushf(6, "child value not equal. path:%s", newpath);
	}
	return 0;
};



};// namespace tbgeneral


namespace tbgeneral{ namespace test{

void test_json_validator(){
	auto schema = "D:/work/General/filial/mylib/filial_lib/settings_schema.json";
	auto file = "c:/temp/test1.json";
	json_validator jv;
	if (jv.load_schema( schema )){
		c_printf("error at load schema: %s\n", jv.err.text );
	} else if (jv.validate_file(file)) {
		c_printf("json validate errors:\n %s", jv.err.text );
	} else {
		c_printf("validate OK '%s'\n", file );
	}
	c_printf("test_json_validator .. end");
}

}}
