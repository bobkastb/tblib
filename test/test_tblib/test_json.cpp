/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "conf/json_parser.h"
#include "test_variant_h.h"



//*************** TEST 
namespace tbgeneral {
	namespace test {



		/*
		static stringA prepare_json_text( const stringW & d){
			auto sa = stringA(d);
			sa = replacechars( sa, "`" , "\"" );
			return sa;
		};
		*/

		/*
		int compareTextFilesToWithinSpaces(const stringA& lf, const stringA& rf) {
			//replacechars
			static stringA spacecharsset(" \t\n\r");
			auto lfbuff = filesystem::readstrfromfile(lf.c_str());
			auto rfbuff = filesystem::readstrfromfile(rf.c_str());
			lfbuff = replacechars(lfbuff, spacecharsset, "");
			rfbuff = replacechars(rfbuff, spacecharsset, "");
			return strcmp(lfbuff, rfbuff);
		}

		void test_json_parser() {
			stringA  infile = "c:/temp/test2.json";
			stringA outfile = infile + ".out";
			json_owner jo;
			jo.parsefile(infile.c_str());
			if (jo.error.error_id) {
				printf("on parse file <%s>: %s ", infile.c_str(), jo.error.text.c_str());
			}
			else {
				jo.savefile(outfile.c_str());
				int res = compareTextFilesToWithinSpaces(infile, outfile);
				printf("test_json_parser: compare in & out json = %s", (res == 0 ? "OK" : "FAIL"));
			}
			printf("test_json_parser ... end\n");
		}
		void test_json_parser_ev() {
			stringA  infile = "c:/temp/settings_sw.json";
			stringA  outfile = infile + ".out";
			json_owner jo;
			jo.parsefile(infile);
			if (jo.error.error_id) {
				printf("on parse file <%s>:\n %s \n", infile.c_str(), jo.error.text.c_str());
			}
			else {
				jo.savefile(outfile.c_str());
			}
			printf("test_json_parser_ev ... end\n");
		}
		*/

	}
}; // namespace tbgeneral{ namespace test{


namespace tbgeneral { namespace test_ns {
static const char* jsontxt0 = u8R"const(
{
"options":[
	{"keys":["c","cfg"], "type":"filename",	"help":"json файл конфигурации" },
	{"keys":["u","update"], "type":true ,	"help":"только перезапись готовой конфигурации в exe файлы" },
	{"keys":["m","make"], "type":true ,		"help":"создание статистических частотных таблиц символов из текстов" },
	{"keys":["f","force"], "type":true ,	"help":"обязательное создание (заново) статистических частотных таблиц символов из текстов" },
	{"keys":["id","id_config"], "type":"ne","help":"идентификатор конфигурации" },
	{"keys":["p","pass"], "type":"ne",		"help":"пароль на доступ к конфигурации" } 
] 
,"Всякое": {
		"intv":10 ,"floatv":10.111110 ,"boolT":true ,"boolF":false 
	}
,"Arrays" :{
	"int":[1,2,3,4,5,6,7,8,9,0]
	,"str":["1","2","3","4","5","6","7","8","9","0"]
	,"float":[1.100000,2.100000,3.100000,4.100000,5.100000,6.100000,7.100000,8.100000,9.100000,0.100000]
	}
}
)const";

static const char* jsontxt_arr1 = u8R"const(
{ 
	"options":[ 1,2,3,4 ]
}
)const";

conf_struct_ref _list( const darray<conf_variant> & l ){
	auto jn = conf_struct_t::newnode(conf_variant::ejtArray);
	jn->a_data = l;
	//return json_variant( jn );
	return jn ;
}
conf_struct_ref _obj( const darray<conf_struct_t::r_property> & l ){
	auto jn = conf_struct_t::newnode(conf_variant::ejtObject);
	jn->named_props = l;
	//return json_variant( jn );
	return jn;
}
int cmpvalue(test_state& err , int index, const conf_variant & n ,  const conf_variant& eta  ){
	if (n.type != eta.type) 
		return err.pushl( __LINE__ , " i(%d) invalid json value type" , index);
	bool diff=true;
	switch (eta.type) {
	case conf_variant::ejtBool:	diff= n.d.b != eta.d.b; break;
	case conf_variant::ejtInt:		diff= n.d.i != eta.d.i; break;
	case conf_variant::ejtFloat:	diff= n.d.f != eta.d.f; break;
	case conf_variant::ejtStr:		diff= n.d.str != eta.d.str; break;
	case conf_variant::ejtObject: case conf_variant::ejtArray:
		return cmpnode( err, index*100+1 , n.toNode() , eta.toNode() );
	}
	if (diff) return err.pushl( __LINE__ , " i(%d) invalid json value" , index);
	return 0;
}
int cmpnode(test_state& err , int index, const conf_struct_ref& n , const conf_struct_ref& eta  ){
	if (eta->stype!=n->stype) return err.pushl( __LINE__ , " i(%d) invalid json root node type" , index);
	if (n->a_data.size() != eta->a_data.size() ) 
		return err.pushl( __LINE__ , " i(%d) invalid json a_data size " , index);
	if (n->named_props.size() != eta->named_props.size() ) 
		return  err.pushl( __LINE__ , " i(%d) invalid props size " , index);
	for (size_t i=0; i<eta->a_data.size(); i++ ) 
		if (cmpvalue( err, index*100+1, n->a_data[i] , eta->a_data[i] )) return 1;
	for (size_t i=0; i<eta->named_props.size(); i++ ) {
		if ( n->named_props[i].key != eta->named_props[i].key ) 
			return err.pushl( __LINE__ , " i(%d) invalid json key" , index);
		if (cmpvalue( err, index*100+2, n->named_props[i].data , eta->named_props[i].data )) return 1;
	}
	for (size_t i=0; i<eta->named_props.size(); i++ ) {
		auto & ep = eta->named_props[i];
		auto nv = n->get_ptr( ep.key );
		//if ( nv != &n->named_props[i].data, sizeof(nv) )  
		if ( nv != &n->named_props[i].data)  
			return err.pushl( __LINE__ , " i(%d) invalid json index" , index);
	}
	return 0;
}
static int test_json0(test_state& err) { 
	json_ctx jown;
	auto r=jown.parsedata(jsontxt0);
	if (r) return err.pushl( __LINE__ , " json parse error:%s", jown.error.text.c_str() );
	if (jown.root->stype!= conf_variant::ejtObject) return err.pushl( __LINE__ , " invalid json root node type");
	auto eroot= _obj({ 
		{"options" , _list( { 
			_obj( { {"keys",_list({"c","cfg"})},	{"type","filename"}, {"help", u8"json файл конфигурации"}	} )
			,_obj( { {"keys",_list({"u","update"})},{"type",true},		 {"help", u8"только перезапись готовой конфигурации в exe файлы"}	} ) 
			,_obj( { {"keys",_list({"m","make"})},  {"type",true},		 {"help", u8"создание статистических частотных таблиц символов из текстов"}	} ) 
			,_obj( { {"keys",_list({"f","force"})},  {"type",true},		 {"help", u8"обязательное создание (заново) статистических частотных таблиц символов из текстов"}	} ) 
			,_obj( { {"keys",_list({"id","id_config"})}, {"type","ne"},	 {"help", u8"идентификатор конфигурации"}	} ) 
			,_obj( { {"keys",_list({"p","pass"})}, {"type","ne"},	 {"help", u8"пароль на доступ к конфигурации"}	} ) 
			})
		},{u8"Всякое" , _obj( { {"intv",10},{"floatv",10.11111},{"boolT",true} , {"boolF",false} } )
		},{"Arrays" ,_obj( { 
			{"int", _list({1,2,3,4,5,6,7,8,9,0}) }
			,{"str",_list({"1","2","3","4","5","6","7","8","9","0"})}
			,{"float",_list({1.1,2.1,3.1,4.1,5.1,6.1,7.1,8.1,9.1,0.1})}
			})
		}
	});	
	//auto da = jown.root->getproparr();
	//
	if (cmpnode(err, 0 , jown.root , eroot) ) return 1;

	return 0; 
}

static int test_json_for_c(test_state& err , const conf_struct_ref& n ) {

	for ( auto pr : *n ) {
		auto pv = &pr.Value();
		if (n->stype == conf_variant::ejtArray ) {
			if ( pv != &n->a_data[ pr.Index() ] ) return err.pushl( __LINE__ , " invalid json for");
		} else {
			auto np = &n->named_props[ pr.Index() ];
			if ( &pr.Key() != &np->key ) return err.pushl( __LINE__ , " invalid json for");
			if ( pv != &np->data ) return err.pushl( __LINE__ , " invalid json for");
		}
		if (pv->isObjectType() && test_json_for_c(err, pv->toNode())) 
			return 1;
	}
	return 0;
}
static int test_json_for(test_state& err) { 
	json_ctx jown;
	auto r=jown.parsedata(jsontxt0);
	return test_json_for_c( err,  jown.root );
}
static int test_json_load_store(test_state& err) { 
	json_ctx jown;
	auto srcs = stringA(jsontxt0);
	auto r=jown.parsedata(srcs);
	auto txt = jown.save();
	txt = replacechars( txt ,   "	\n ","");
	srcs = replacechars( srcs , "	\n ","");
	if (txt!=srcs)
		return err.pushl( __LINE__ , " invalid json load store");
	return 0;
}

template <typename Etype> int cmparr( const darray<Etype> & l , const darray<Etype> & r  ) {
	if ( l.size()!= r.size()) return 1;
	for (size_t i=0;i<l.size();i++) if (l[i]!=r[i]) return int(2+i);
	return 0;
}
static int test_json_get(test_state& err) { 
	json_ctx jown;
	auto r=jown.parsedata(jsontxt0);
	//return test_json_for_c( err,  jown.root );
	auto ars = jown.root->getnode("Arrays");
	auto nas = ars->getnode("str");
	auto ia = ars->get_array<int>("int");
	auto fa = ars->get_array<float>("float");
	auto sa = ars->get_array<stringA>("str");
	if ( cmparr( ia , darray<int>{1,2,3,4,5,6,7,8,9,0} ) )
		err.pushl( __LINE__ , " invalid json get arr");
	if ( cmparr( fa , darray<float>{1.100000,2.100000,3.100000,4.100000,5.100000,6.100000,7.100000,8.100000,9.100000,0.100000} ) )
		err.pushl( __LINE__ , " invalid json get arr");
	if ( cmparr( sa , darray<stringA>{"1","2","3","4","5","6","7","8","9","0"} ) )
		err.pushl( __LINE__ , " invalid json get arr");
	return 0;
}

static int test_json_arrload(test_state& err) {
	json_ctx jown;
	auto r = jown.parsedata(jsontxt_arr1 );
	return 0;
}

void foreach_confv(const conf_struct_ref& n, f_foreach_conv_variant forv) {
	for (auto& it : *n) {
		auto& v = it.Value();
		if (!forv(n, it.Key(), it.Value())) break;
		if (v.isObjectType())
			foreach_confv(v.toNode(), forv);
	}
}

static int test_slice_test(test_state& err) {
	darray<stringA> jtexts = { jsontxt0 , jsontxt_arr1 };
	for (auto& srctxt : jtexts) {
		json_ctx jown;
		int errs = 0;
		auto rndf = [&](const conf_struct_ref& own, const stringA& k, const conf_variant& v) {
			if ( !k.empty() && k.fstorage != srctxt.fstorage) {
				errs++;  //return false;
			}
			if (v.type == conf_variant::ejtStr) {
				auto s = v.toStr();
				if ( !s.empty() && s.fstorage != srctxt.fstorage)
					errs++;
			}
			return true;
		};
		auto r = jown.parsedata(srctxt);
		if (r) return err.pushl(__LINE__, " json parse error:%s", jown.error.text.c_str());
		foreach_confv(jown.root, rndf);
		if (errs)
			return err.pushl(__LINE__, " count not slices:%d", errs );
	}

	return 0;
}

// Need test for json_validator

mDeclareTestProc(test_json,esr_TestNeedUpdate){
//int test_json() {
	//std::cout << u8" JSON: нужен тест envexpand и проверка что все строки внутри - слайсы!!\n ";
	
	if (call_test(test_json_arrload, "test_json_arrload")) return 1;
	if (call_test(test_json0, "test_json0")) return 1;
	if (call_test(test_json_for, "test_json1")) return 1;
	if (call_test(test_json_load_store, "test_json_load_store")) return 1;
	if (call_test(test_json_get, "test_json_get")) return 1;
	if (call_test(test_slice_test, "test_slice_test")) return 1;
	
	return 0;
}

}};

