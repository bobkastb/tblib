#include "conf_variant.h"
#include "gdata/t_format.h"
#include "parse/t_path.h"
#include "tsys/tbfiles.h"




namespace tbgeneral {

	struct t_expand_ctx :public conf_parser_defs_based {
		t_error_info error;
		conf_parser_ctx_based* jo=0;
		std::map<stringA, int> errlist;

		int foreachnode_getexpkeys(conf_struct_ref jn) {
			FOREACH(v, jn->a_data) {
				if (v->isObjectType()) { foreachnode_getexpkeys(v->toNode()); };
			}
			FOREACH(v, jn->named_props) {
				auto k = v->key; auto s = k.cbegin(), e = k.cend();
				if (s[0] == '%' && e[-1] == '%') {
					s++; e--;
					auto newk = k.slice(s, e);
					if (v->data.type == conf_variant::ejtStr) {
						jo->locenv.set(newk, v->data.toStr());
					}
					else {
						jo->error.error_id = jeEVMustBeString;
						jo->error.text << "environment var must have string type : " << newk << " !";
						return jo->error.error_id;
					}
				}
				if (v->data.isObjectType()) { foreachnode_getexpkeys(v->data.toNode()); };
			}
			return 0;
		}

		int expand_json_variant(conf_variant& v) {
			if (v.type != conf_struct_t::ejtStr) return 0;
			int cnterr = 0;
			auto& vstr = v.refStr();
			auto unkcnt = jo->locenv.expand(vstr, vstr);
			if (unkcnt) {
				auto da = LocalEnv::split_env(vstr);
				FOREACH(x, da) errlist[*x]++;
				cnterr += static_cast<int>(da.size());
			};
			return cnterr;
		}

		int foreachnode_expand(conf_struct_ref jn) {
			FOREACH(v, jn->a_data) {
				if (v->isObjectType()) {
					foreachnode_expand(v->toNode());
				}
				else expand_json_variant(*v);
			}
			auto disable_expand_key = stringA("<!disable_expand_environment>");
			if (jn->get<bool>(disable_expand_key))
				return 0;
			FOREACH(vv, jn->named_props) {
				auto v = &vv->data;
				if (v->isObjectType()) {
					foreachnode_expand(v->toNode());
				}
				else expand_json_variant(*v);
			}
			return 0;
		}
	}; // struct t_expand_ctx


	int conf_parser_ctx_based::expand_all_env_vars() {
		t_expand_ctx ectx;  ectx.jo = this;
		if (datafilename.size()) {
			locenv.set("filedir", get_file_dir(datafilename));
		};
		ectx.foreachnode_getexpkeys(root);
		if (!locenv.closure()) { error.error_id = jeInvalidClosureEV;  error.text << "invalid closure for environment vars"; return error.error_id; }
		ectx.foreachnode_expand(root);
		if (ectx.errlist.size()) {
			error.error_id = jeUnknownEnvVars;  error.text << "unknown environment variable: ";
			FOREACH(x, ectx.errlist) {
				error.text << x->first << " ";
			}
		}
		return error.error_id;
		//jeInvalidClosureEV	jeUnknownEnvVars
	};


	int conf_parser_ctx_based::parsefile(const stringA& filename) {
		datafilename = filename;
		auto buff = filesystem::readstrfromfile(datafilename);
		if (buff.empty()) { return error.pushf(jeFileNotRead, " Error read file %s", datafilename); }
		return parsedata(buff);
	};
	int conf_parser_ctx_based::parse_DataOrFile(const stringA& filename) {
		if (FilenameIsData(filename)) {
			return parsedata(filename);
		}
		else {
			return parsefile(filename);
		}
	};

};