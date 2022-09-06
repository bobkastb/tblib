#include "conf_variant.h"
#include "gdata/tb_numbers.h"
#include "gdata/t_format.h"
#include "parse/t_path.h"
#include "tsys/tbfiles.h"

#define bibb(s) throw s
//-------------



namespace tbgeneral {

using key_t =	conf_variant_types::key_t;

template<typename Et> void destruct_type( Et * v ){ new (v) Et;}
template<typename Et> void construct_type( Et * v ){ v->~Et(); }


conf_variant::conf_variant() { 
	ZEROMEM(*this);
}

void conf_variant::init(byte tp) { 
	type=tp; opt=0;  
	ZEROMEM(d);
	switch (type) {
	case ejtStr:    construct_type( &d.str ); break;
	case ejtObject:
	case ejtArray:	construct_type( &d.node ); break;
	}
} 
void conf_variant::clear(){
	switch (type) {
	case ejtStr:    destruct_type( &d.str ); break;
	case ejtObject:
	case ejtArray:	destruct_type( &d.node ); break;
	}
	ZEROMEM(d);
};
void conf_variant::assign(const conf_variant& cv){
	clear();
	init( cv.type );
	switch (type) {
	case ejtStr:     d.str= cv.d.str;  break;
	case ejtObject:
	case ejtArray:	d.node = cv.d.node; break;
	default : memcpy( &d , &cv.d , sizeof(d) );
	}
};
conf_variant::conf_variant( const conf_struct_ref & v ){
	//init( ejtObject ); 
	init( v->stype ); 
	d.node = v; 
};

int64 conf_variant::toInt() const{
	switch (type) {
	case ejtBool: return d.b?1:0; 
	case ejtInt: return d.i; 
	case ejtFloat: return int64( d.f );
	default: bibb("conf_variant:invalid convert");
	}
};
double conf_variant::toFloat() const{
	switch (type) {
	case ejtInt: return static_cast<double> ( d.i ); 
	case ejtFloat: return d.f;
	default: bibb("conf_variant:invalid convert");
		//, ejtStr, ejtObject, ejtArray 
	}
};
conf_variant::str_t conf_variant::toString() const {
	switch (type) {
	case ejtArray : return "[Array]";
	case ejtObject: return "[Object]";
	default: return toStr();
	}
};
conf_variant::str_t& conf_variant::refStr() {
	static str_t zeros; //TODO:!!
	if (type != ejtStr) { return zeros;}
	return d.str;
};
conf_variant::str_t conf_variant::toStr() const{
	switch (type) {
	case ejtBool: return d.b ? "true":"false"; 
	case ejtInt: return itoa_t(d.i); 
	case ejtFloat: return format("%f" , d.f);
	case ejtStr: return d.str;
	case ejtArray: case ejtObject: bibb("conf_variant:invalid convert");
	default: return "";
	}
};
bool conf_variant::toBool() const{
	switch (type) {
	case ejtBool: return d.b; break;
	case ejtInt: return d.i!=0; break; 
	default: bibb("conf_variant:invalid convert");
		//, ejtStr, ejtObject, ejtArray 
	}
};
stringW conf_variant::toStrW() const{ 
	return toStr();
};
conf_struct_ref conf_variant::toNode() const{
	switch (type) {
	case ejtArray: case ejtObject: return d.node ;
	default: return conf_struct_ref();
	}
};

//---------------------------------------


const conf_struct_t::iterator conf_struct_t::begin() const{ return iterator(this,0);};
const conf_struct_t::iterator conf_struct_t::end() const{ return iterator(this,size()); };
conf_struct_t::iterator conf_struct_t::begin(){ return iterator(this,0); };
conf_struct_t::iterator conf_struct_t::end(){ return iterator(this,size());};
const key_t & conf_struct_t::iterator::Key() const{
	static key_t nullkey;
	return nod->stype == ejtArray  ? nullkey : nod->named_props[index].key ;
}; 
const conf_variant & conf_struct_t::iterator::Value() const{
	return nod->stype == ejtArray ? nod->a_data[index] : nod->named_props[index].data;
};


const int conf_struct_t::find_index(const key_t& key) const{
	for(size_t i=0;i<named_props.size();i++) {
		if ( 0==stricmp( named_props[i].key , key) )  { return i; }
	}
	return -1;
};
/*
const conf_struct_t::r_property* conf_struct_t::find_prop(const key_t& key) const{
	auto i = find_index( key );
	return i>=0 ? &named_props[i] : 0;
};
*/
conf_variant conf_struct_t::getvar(size_t index) const { 
	auto p = get_ptr(index);
	return p ? *p : conf_variant();
};
conf_variant conf_struct_t::getvar(key_t key)const { 
	auto p = get_ptr(key);
	return p ? *p : conf_variant();
};


const conf_variant* conf_struct_t::get_ptr(const key_t& key) const{ 
	auto i = find_index( key );
	return i>=0 ? &named_props[i].data : 0;
};
const conf_variant* conf_struct_t::get_ptr(size_t index) const {
	if (index>size()) return 0;
	return stype==ejtArray ? &a_data[index] : &named_props[index].data;
};
conf_variant* conf_struct_t::get_ptr(size_t index) {
	if (index>size()) return 0;
	return stype==ejtArray ? &a_data[index] : &named_props[index].data;
};
conf_variant* conf_struct_t::get_ptr(const key_t& key){
	auto i = find_index( key );
	return i>=0 ? &named_props[i].data : 0;
};


const conf_variant& conf_struct_t::get_ref(size_t index) const{ 
	return stype == ejtArray ? a_data[index] : named_props[index].data;
};
const conf_variant& conf_struct_t::get_ref(const key_t& key) const{
	return *get_ptr(key); //TODO:? 
};

conf_struct_t* prep_conf_path(const conf_struct_t* self, const key_t& path, key_t& lastName) {
	auto sp = split_str(path, '/');
	auto last_i = sp.size() - 1;
	auto xn = const_cast<conf_struct_t*> (self); // const_cast<conf_struct_t*> (self);
	for (size_t i = 0; (i < last_i) && (xn); i++)
		xn = xn->getnode(sp[i]).data();
	lastName = sp[last_i];
	return xn;
}

conf_variant* conf_struct_t::get_path_ptr(const key_t& path) {
	key_t lastKey;
	auto xn = prep_conf_path(this, path, lastKey);
	return xn ? xn->get_ptr(lastKey) : 0 ;
};
conf_variant conf_struct_t::get_path(const key_t& path) const{
	key_t lastKey;
	auto xn = prep_conf_path(this, path, lastKey);
	return xn ? xn->getvar(lastKey) : conf_variant() ;
};

int conf_struct_t::set_path(const key_t& path, conf_variant newv ) {
	key_t lastKey;
	auto xn = prep_conf_path(this, path, lastKey);
	if (xn) xn->set(lastKey, newv);
	return xn ? 0 : -1;
};


size_t conf_struct_t::push_back( const conf_variant& v){
	a_data.push_back(v);
	return a_data.size()-1;
};
size_t conf_struct_t::push_back(const key_t& key , const conf_variant& v){
	if ( stype==ejtArray) a_data.push_back(v);
	else named_props.push_back(r_property{key,v});
	return size()-1;
};

// Установить значение
void conf_struct_t::set(const key_t& key, const conf_variant& v){
	auto ptr = get_ptr( key );
	if (!ptr) push_back( key , v); 
	else *ptr = v;
};
void conf_struct_t::set(size_t index, const conf_variant& v){
	auto ptr = get_ptr( index );
	if (!ptr) throw "range error";
	*ptr = v;
};


conf_struct_ref conf_struct_t::newnode(eVarType jt ){
	auto r = conf_struct_ref::CreateNew( );
	r->stype = jt;
	return r;
};
	// создать копию узла
conf_struct_ref conf_struct_t::copy() const{
	auto res = newnode();
	res->copy(this);
	return res;
};
	// Скопировать в себя заданный узел 
void conf_struct_t::copy(const conf_struct_t * from){
	stype= from->stype;
	a_data.copy_from( from->a_data );
	named_props.copy_from( from->named_props );
};
// очистка внутренностей! size -> 0
void conf_struct_t::erase(){}; 
void conf_struct_t::erase(const key_t& key){
	auto i = find_index( key );
	if (i>=0) named_props.erase(i,1);
};

const conf_variant* conf_struct_t::get_demand(const key_t& key, eVarType vtype) const{
	auto p =  get_ptr(key);
	return (!p || p->type != vtype) ? 0 : p;
};

void conf_parser_ctx_based::new_root() {
	//clear();
	root = conf_struct_t::newnode();
};
conf_struct_ref conf_parser_ctx_based::get(const stringA& path) const {
	return root.data() ? root->get_path<conf_struct_ref>(path) : conf_struct_ref();
};
void conf_parser_ctx_based::clear() {
	root = conf_struct_ref();
};


}; // namespace tbgeneral {


//-----------


