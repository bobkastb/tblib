/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "conf/tbcfg_parse.h"
#include "test_variant_h.h"

namespace tbgeneral { namespace test_ns {

static const char * test_tbconf_text_111 =
u8R"const(
{
	metainfo {
			an_codes{ 	
				list{IPOUT=1; // возможность получения адреса области испытуемого кода
					SELF_R=2; SELF_RW=4; SELF_RWE=8; 
					FS_30=0x10; PEB=0x20; DLLHDR=0x40; DLLNAMES=0x80;
					DLLC_R=0x100; DLLC_E=0x200;
					FS_0=0x1000; FS_ANY=0x2000; 	
				};	
			};		
			registers{ type=bitnum;
				list[
				AL,CL,DL,BL,AH,CH,DH,BH,
				AX,CX,DX,BX,SP,BP,SI,DI,
				EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI,
				ES,CS,SS,DS,FS,GS,
				EFLAGS;
				];	
			};
			opcodes{ list{ ANY=1; EQU=2;}; }
			
	};
	
	finder {
			startoffsetForChecks=0;
			enable_emulator=1;
			tester_exe [ "%prg%/testing_code.exe" ];	//*
			xg_checking=0;	
	};
	preanalyzer{
		MinimumNormalCMD=3; // пропуск к эмулятору
		MaximumPreviewCMD=3;
		MaxCntUseUndefinedRegs=0;
		PreDefinedRegs { SP,ESP,ES,CS,SS,DS,FS,GS };
		//2. Маска отсекаемых типов команд 
	};
	emulator{
		//Changed_EFlags=
		FPU_CW=0x27f;
		mincmdcnt=500; // минимальное количество выполненых команд для испытуемого кода
		maxcmdcnt=500; // максимальное количество выполненых команд для испытуемого кода *
		PreDefinedRegs { BP,EBP,SP,ESP, 	ES,CS,SS,DS,FS,GS };
		
		stack{ //* TODO:
				ESP=0x1000; // смещение от вершины стека 
				framescount=0;	// количество фреймов "надуманных" процедур в стеке
				framesize=0;	// размер одного стекового фрейма "надуманной" процедуры
		};
		
		memory{
			programm_image[ E"%prg%/finder.memimage.dmp" , E"d:/work/General/ExploitSearcher/images/finder.memimage.dmp"     ];
			CodeBeing_leftright_addition=0x2000; // добавка в к испытуемому коду слева и справа
			CodeBeing_LoadAddress=0x20000000; // предпочтительный адрес загрузки испытуемого кода
			StackSize=0x100000;				// предпочтительный адрес стека
			StackBase=0x1000000;
		};
		
		analyzer{
			MinimumCmdCnt=10; // для начала подсчета очков

			scores [ 
				{100;	IPOUT;};	{100;	SELF_R;};	{500;	SELF_RW;};  {100;	SELF_RWE;};
				{300;	FS_30;};	{200;	PEB;};		{200;	DLLHDR;};   {200;	DLLNAMES;};
				{100;	DLLC_R;};	{1000;	DLLC_E;};	{200;	FS_0;};   	{100;	FS_ANY;};
				{500;	IPOUT;SELF_R;};		{300;	FS_30;PEB;};	{400;	FS_30;PEB;DLLHDR};
				{800;	SELF_RWE;FS_30;PEB;};
			];
			hypothesis{
				EQU	{IPOUT,SELF_RW} 
				ANY	{FS_30,FS_0,DLLC_R,DLLC_E,PEB,DLLHDR,DLLNAMES} 
			}
			solve_NeedTest{	}; // as hypothesis
		};	
	};	
}
)const";

static const char * test_hocon_text_0=
u8R"const(
{
	//comment 0
	esc_test: "fff\"uu\nhh" ;
	s_strs {	id1=str_id1; id2=str_id2; id3=12345; id4="тест строки"; };
	a_strs [	str_id1,str_id2,12345,"тест строки" ];
	e_strs [	E"--%prg%-ру-" , "no expand" ];
	x_int  [ 0x44, 0x55 , 88 ]; # comment1
}
)const";
//	x_int  [ 0x44, 0x55 , 88 ];

static stringA expE( const stringA & s ) {
	return expand_environment_string(s);
}
static conf_struct_ref getc_hocontest_0() {
	return _obj({ 
		{"esc_test", "fff\"uu\nhh" }
		,{"s_strs" , _obj({ {"id1","str_id1"}, {"id2","str_id2"}, {"id3",12345}, {"id4",u8"тест строки"}, })}
		,{"a_strs" , _list({ "str_id1","str_id2",12345 ,u8"тест строки"}) } 
		,{"e_strs" , _list({ expE(u8"--%prg%-ру-") , "no expand" }) } 
		,{"x_int" , _list({ 0x44, 0x55 , 88 }) } 
		});
}

static int test_load_0(test_state& err) { 
	hocon_parse_ctx hctx;
	if (hctx.parsedata( test_hocon_text_0 )) 
		return err.pushl(__LINE__, "parse error:%s", hctx.error.text );
	auto eta = getc_hocontest_0();
	if (cmpnode( err, 0 , hctx.root ,eta )) {
		c_printf(" line(%d) hocon cmp error ", __LINE__); 
		return 1;}
	return 0;
}

static int test_start_0(test_state& err) { 
	hocon_parse_ctx hctx;
	if (hctx.parsedata( test_hocon_text_0 )) 
		return err.pushl(__LINE__, "parse error:%s", hctx.error.text );

	return 0; 
}


mDeclareTestProc(test_hocon_parse,esr_TestNeedUpdate ){
//int test_parse() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	if (call_test(test_load_0, "test_load_0")) return 1;
	
	return 0;
}

}};
