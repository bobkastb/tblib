#include "tb_parsecmdl.h"
#include "json_parser.h"
#include <gdata/t_format.h>
#include <parse/t_parse.h>


namespace tbgeneral{
	parseline_result parsecmdline(const char* cmd) {
		parseline_result plr;
		size_t len = strlen_s(cmd); const char* cc = cmd, * ce = cmd + len;
		plr.buff.resize(len + 2); char* out = &plr.buff[0]; int st = 0; char* cpar = 0;
		for (; cc < ce; cc++) {
			char c = *cc;
			if (c == '"') { st ^= 1; continue; }
			if ((!st) && (c == ' ')) {
				if (cpar) {
					plr.list.push_back(cpar); cpar = 0; *out = 0; out++;
				};
				continue;
			}
			else if (!cpar) cpar = out;
			*out = c; out++;
		};
		if (cpar) { plr.list.push_back(cpar); cpar = 0; *out = 0; out++; };

		return plr;
	};

}; // namespace tbgeneral



namespace tbgeneral{


	r_cmdline_options::re_parseresult* r_cmdline_options::get(const stringA & optkey){
		auto opts = getdesc(optkey); if (!opts) return 0;
		auto ind = get_def( lastparse_result.idx , opts->internal_id , -1 );
		if (ind<0) return 0;
		return &lastparse_result.list[ind];
	}; 
	bool r_cmdline_options::get(const stringA & optkey , re_parseresult* & pr){
		auto r = get(optkey);
		pr=r;
		return r!=0;
	};
	bool r_cmdline_options::getvalue(const t_user_id optid, stringA& value) {
		auto r = get(optid);
		bool res = (r && r->a_value.size() > 0);
		if (res) value = r->a_value[0];
		return res;
	};

	r_cmdline_options::re_parseresult* r_cmdline_options::get(const t_user_id  optid ){
		auto opts = getdesc(optid); if (!opts) return 0;
		auto ind = get_def( lastparse_result.idx , opts->internal_id , -1 );
		if (ind<0) return 0;
		return &lastparse_result.list[ind];

	};
	bool r_cmdline_options::get(const t_user_id optid , re_parseresult* & pr){
		auto r = get(optid);
		pr=r;
		return r!=0;
	};
	r_cmdline_options::r_option * r_cmdline_options::getdesc(const t_user_id  optid){
		auto i = get_def(m_opts_id , optid , t_index(-1) );
		if (i<0) return 0;
		if (i>=int(l_opts.size())) return 0;
		return &l_opts[i];
	}; 
	r_cmdline_options::r_option * r_cmdline_options::getdesc(const stringA & optkey){
		auto i = get_def(m_opts_case , optkey , t_index(-1) );
		if (i<0) i=get_def(m_opts , optkey , t_index(-1) );
		if (i<0) return 0;
		if (i>=int(l_opts.size())) return 0;
		return &l_opts[i];
	};

//	{'keys':['c','cfg'], id:0 , 'type':'filename',	'help':'json файл конфигурации' },
	static int str2intID(const stringA & s) {
		auto sz = std::min(sizeof(int), s.size());
		int res = 0; auto ps = s.data() + sz - 1; auto pd = (byte*)&res;
		for (size_t i = 0; i < sz; i++, ps--, pd++)
			*pd = *ps;
		return res;
	}
	int r_cmdline_options::set_as_json( const stringA & _data , int flags ){
		//lastparse_result=r_parseresult();	auto r=&lastparse_result;
		stringA data=_data;
		if (flags & ef_SpecialJson) 
			data = replacechars( data ,"'","\"");
		json_ctx jo;
		jo.parsedata( data );
		if (jo.error.error_id)  return error.push(jo.error);
		auto n=jo.root->getnode("options");
		if (n.empty()) return error.push(9003,"Expected section 'options'");  

		auto a= n->toArray<conf_struct_ref>();
		t_index intid=0;
		l_opts.resize(0);

		FOREACH( np , a) { auto n = *np;
			r_option opt;
			auto kn  = n->getnode("keys"); if (kn.empty()) { return error.push(1801,"wait <keys>!"); }
			opt.ids = kn->toArray<stringA>();
			opt.user_id = str2intID(opt.ids[0]);
			n->get("id", opt.user_id);
			opt.flags |= otag_isArray;
			n->get("help", opt.help ); //if (!kn) { return 1801; }
			//n->get()
			opt.internal_id = intid;
			l_opts.push_back( opt );
			intid++;
		};
		m_opts.clear(); m_opts_id.clear();m_opts_case.clear();
		FOREACH( po ,  l_opts){
			FOREACH( kp , po->ids ) {
				m_opts[*kp] = po->internal_id;
				m_opts_case[*kp] = po->internal_id;
			}
			m_opts_id[po->user_id] = po->internal_id;
		}
		return 0;
	}; 

stringA r_cmdline_options::printOptions( ){
	stringA buff;
	FOREACH( po ,  l_opts){
		buff << format("-- keys:%s", join( po->ids , "," )); //TODO: здесь нужен оператор вывода в поток po->ids , тогда join не нужен
		buff << " [";
		if (po->flags & otag_NeedValue) buff<< "NeedValue ";
		buff << "], ";
		buff << "Desc:" << po->help;
		buff << "\n";
	}
	return buff;
};


struct parse_ctx{
	struct r_pack{ stringA nm; darray<stringA> val; r_cmdline_options::r_option* ti; 
		r_pack() { ti=0;} 
	};
	r_pack opt;
	darray<r_pack> all;
	r_cmdline_options * mctx;
	parse_ctx( r_cmdline_options * _mctx ) { mctx = _mctx; }
	void saveopt(){
		if (opt.nm.size() || opt.val.size()) { all.push_back(opt); }
		opt = r_pack();
	};
	void newopt( const stringA & o_name , const stringA & o_val ){
		saveopt();
		opt= r_pack(); opt.nm = o_name; 
		opt.ti = mctx->getdesc(o_name);
		if (o_val.size()) {
			opt.val.push_back(o_val); saveopt(); return; }
		if (!opt.ti) { saveopt(); return; }
		if (opt.ti && ((opt.ti->flags & r_cmdline_options::otag_isArray )==0)) { saveopt(); return; }

	};
	void addval( const  stringA & o_val ){
		if (!o_val.size()) { return; }
		opt.val.push_back(o_val); 
		if (opt.ti && ((opt.ti->flags & r_cmdline_options::otag_isArray )==0)) { saveopt(); return; }
	};
	void finish(){ saveopt(); };
};

int r_cmdline_options::parse_all(int argc , const char** argv ){
		parse_ctx ctx(this);
		lastparse_result = r_parseresult(); auto lpr = &lastparse_result;
		for (int i=1;i<argc;i++){
			auto pc=argv[i];
			if (pc[0]=='-' || pc[0]=='/') {
				stringA nv[2]; stringA pn=pc+1;
				split_str2( pn , '=' , nv[0] , nv[1] );
				ctx.newopt( nv[0] , nv[1] );
			} else ctx.addval(pc);
		}
		ctx.finish();
		int cnt=0;
		FOREACH( po , ctx.all ) {
			auto snm = stringA(po->nm);
			if (po->nm.size()) {
				if (!po->ti) { 
					lpr->error.pushf(9001,"Unknown option '%s'", snm );
					continue; }
			}
			re_parseresult epr;
			epr.opt = po->ti;
			epr.a_value = po->val;
			if(epr.a_value.size()==1) epr.value=epr.a_value[0];
			if(epr.opt) {
				if ((epr.opt->flags & otag_NeedValue) && (epr.a_value.size()==0)) 
					lpr->error.pushf(9002,"Need a value for option '%s'", snm );
			};
			epr.i_id = cnt;
			lpr->list.push_back( epr );
			cnt++;
		};
		//int i=0;
		FOREACH( ppr , lpr->list ) {
			auto opt= ppr->opt;
			if(opt) lpr->idx[ opt->internal_id ] = ppr->i_id;
		};
		return lpr->error.error_id;
	};

}; // namespace tbgeneral

// test 

