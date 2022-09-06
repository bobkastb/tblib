#include "conf/tb_parsecmdl.h"
#include "gdata/t_string.h"

namespace tbgeneral {
	namespace test_ns {

		static const wchar* txt_cmd_options = L"{ \n\
'options':[\n\
	{'keys':['c','cfg'], 'type':'filename',	'help':'json файл конфигурации' },\n\
	{'keys':['u','update'], 'type':true ,	'help':'только перезапись готовой конфигурации в exe файлы' },\n\
	{'keys':['m','make'], 'type':true ,		'help':'создание статистических частотных таблиц символов из текстов' },\n\
	{'keys':['f','force'], 'type':true ,	'help':'обязательное создание (заново) статистических частотных таблиц символов из текстов' },\n\
	{'keys':['id','id_config'], 'type':'ne','help':'идентификатор конфигурации' },\n\
	{'keys':['p','pass'], 'type':'ne',		'help':'пароль на доступ к конфигурации' } \n\
] \n\
}";

		static void prn_opt(r_cmdline_options& cmdl, const char* nm) {
			auto h = cmdl.get(nm);
			if (h) printf("[%s]=%s\n", nm, join(h->a_value, ", ").c_str());
			else printf("nothing [%s]", nm);
		}

		void test_r_cmdline_options() {
			const char* args[] = { "0","-c=testfile.json", "-u" , "-m" , "-p=test" };
			darray<const char*> s_args(args, ARRAYLEN(args));
			printf("command line:%s ", join(s_args, " ").c_str());
			auto jsontxt = stringA(txt_cmd_options);
			r_cmdline_options cmdl;
			if (cmdl.set_as_json(jsontxt)) {
				printf("%s\n", cmdl.error.text.c_str()); return;
			}
			if (cmdl.parse_all(ARRAYLEN(args), args)) {
				printf("%s\n", cmdl.lastparse_result.error.text.c_str());
				return;
			};
			auto lpr = &cmdl.lastparse_result;
			printf("--values--\n");
			FOREACH(e, lpr->list) {
				stringA nm = e->opt ? e->opt->ids[0] : "";
				printf("id:%d key:%s value:%s \n", e->i_id, nm.c_str(), join(e->a_value, ", ").c_str());
			};
			printf("--get test--\n");
			prn_opt(cmdl, "update");
			prn_opt(cmdl, "cfg");
			prn_opt(cmdl, "pass");
			prn_opt(cmdl, "make");

		};

	}
}; // tbgeneral::test