#pragma once
#include "gdata/t_string.h"
#include "gdata/tb_map.h"
#include "gdata/tb_env.h"
#include "gdata/t_error.h"
#include "conf_variant.h"

namespace tbgeneral{


struct json_ctx_defs : public conf_parser_defs_based {
	using json_print_option = print_option_t;
	struct json_print_options {
		smap<json_print_option> shortopts;
	};
	//using key_t = conf_variant_types::key_t;
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


};

struct json_ctx : public json_ctx_defs , public conf_parser_ctx_based {

	//int parsefile(const stringA & filename);
	int savefile(const stringA & filename , json_print_options * opts=0 );
	stringA save( json_print_options * opts=0 );
	int parsedata(const stringA& data) override;
	bool FilenameIsData(const stringA& fn) override;

	static void json_print(const conf_struct_ref & node, stringA& buff, int level, json_print_options* opts);
	static stringA json_print(const conf_struct_ref& node);
	static void json_print(const conf_variant& var, stringA& buff, int level, json_print_options* opts);
	static void json_print_simple(const conf_variant& var, stringA & buff);
	static bool testNoPrint(const conf_variant& var);
	static stringA json_encode_screening(const stringA& d);

private:
	void fixup_option( json_print_options * opts , bool doset);
};



struct json_validator{
	enum {
		jveUncompatableTypes = 13104,
		jveDisableEmpptyValue = 13105,
		jveOutOfMandatory = 13106,
		jveRangeError = 13107,

		jvelUndefinedType = 13201,
		jvelInvalidFormat2Type = 13202
	};
	t_error_info err;
	void * ctx;
	int load_schema( const stringA & data );
	int validate_file( const stringA & data );
	int validate_data( const stringA & data );
	int validate_json( json_ctx * jo );
	~json_validator();

};

struct cfg_cmp_context_t {
	t_error_info err;
	spmap<int> ignore_path;
	spmap<int> ignore_keys;
	char comment_firstchars[128];
	//using json_variant = json_node::json_variant;

	void init(const stringA& i_paths, const stringA& i_keys, const stringA& start_comment);
	bool isIgnored(const stringA& path, const stringA& key);
	int cmp(const conf_struct_ref & nf, const conf_struct_ref& nb, const stringA& cpath);
	int cmpv(const conf_variant& nf, const conf_variant& nb);
	int cmp_json(const stringA& json_l, const stringA& json_r);
	bool is_commentkey(const stringA& key);
	int error(int code, const stringA& g) { err.push(code, g); return code; };
};


struct ojsonstream {
	enum scope_types { scNone = 0, scStart, scEnd, scB_ARR, scE_ARR, scB_OBJ, scE_OBJ };
	enum scope_state_e { stateNone = 0, stateARR, stateObj, stateCLOSE };
	enum text_special_e { etext_CR='\n' };
	enum tag_types { tagName = 1, tagData = 2 };
	using char_type = char;
	struct scope_state_t {
		scope_state_e id;
		int datacnt;
		scope_state_t() { datacnt = 0; id = stateNone; }
	};
	tag_types waittag;
	scope_state_t scope;
	darray<scope_state_t> stack;
	ojsonstream() { waittag = tagData; }
	void push_datastr(const char_type* data) { push_datastr(data, strlen_s(data)); };
	void push_strtype(const char_type* data, size_t strsize);
	void push_datastr(const char_type*, size_t strsize);
	void setScope(scope_types tg);
	void makeerror(const char* einf = "") { throw einf; }
	void close();
	void push_etext(text_special_e t);
protected:
	virtual void push(const char_type* data, size_t sz) = 0;
	void pushbin(const char_type* data) { pushbin(data, strlen_s(data)); };
	void pushbin(const char_type* data, size_t sz);
	void init();
	void endData();
	void endName();
	void before();
};

ojsonstream& operator << (ojsonstream& s, int32_t v);
ojsonstream& operator << (ojsonstream& s, uint32_t v);
ojsonstream& operator << (ojsonstream& s, double v);
inline ojsonstream& operator << (ojsonstream& s, int8_t v) { return s << int32_t(v); }
inline ojsonstream& operator << (ojsonstream& s, uint8_t v) { return s << uint32_t(v); }
inline ojsonstream& operator << (ojsonstream& s, int16_t v) { return s << int32_t(v); }
inline ojsonstream& operator << (ojsonstream& s, uint16_t v) { return s << uint32_t(v); }
inline ojsonstream& operator << (ojsonstream& s, float v) { return s << double(v); }

inline ojsonstream& operator << (ojsonstream& s, bool v) { s.push_datastr(v ? "true" : "false"); return s; }
inline ojsonstream& operator << (ojsonstream& s, const char* v) { s.push_strtype(v, strlen_s(v)); return s; }
inline ojsonstream& operator << (ojsonstream& s, const stringA& v) { s.push_strtype(v.data(), v.size()); return s; }
//inline ojsonstream& operator << (ojsonstream& s, const partstringA& v) { s.push_strtype(v.data(), v.size()); return s; }
inline ojsonstream& operator << (ojsonstream& s, const std::string& v) { s.push_strtype(v.data(), v.size()); return s; }
inline ojsonstream& operator << (ojsonstream& s, ojsonstream::scope_types v) { s.setScope(v); return s; }
inline ojsonstream& operator << (ojsonstream& s, ojsonstream::text_special_e v) { s.push_etext(v); return s; }


template<typename Et> ojsonstream& pusharray(ojsonstream& s,const Et* a, size_t sz) {
	s << s.scB_ARR;
	for (size_t i = 0; i < sz; i++) s << a[i];
	s << s.scE_ARR;
	return s;
}
template<typename Et, size_t Sz> ojsonstream& operator << (ojsonstream& s, const Et(&a)[Sz]) { return pusharray(s, a, Sz); }
//template<typename Et> ojsonstream& operator << (ojsonstream& s, const std::vector<Et>& a) { return pusharray(s, a.data(), s.size()); }
template<typename Et> ojsonstream& operator << (ojsonstream& s, const darray<Et>& a) { return pusharray(s, a.data(), a.size()); }

template<typename String_t> struct ojsonstream_string : public ojsonstream {
	String_t storage;
	String_t* ptr;
	ojsonstream_string() { ptr = &storage; init(); }
	ojsonstream_string(String_t& buff) { ptr = &buff; init(); }
	void push(const char_type* data, size_t sz) { ptr->append(data, sz); };
	String_t str() { close(); return *ptr; }
};
using strJsonstream = ojsonstream_string<stringA>;


}; // namespace tbgeneral{
