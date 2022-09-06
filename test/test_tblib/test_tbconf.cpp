/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "conf/json_parser.h"
#include "conf/tbconf.h"


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
				list{
				AL,CL,DL,BL,AH,CH,DH,BH,
				AX,CX,DX,BX,SP,BP,SI,DI,
				EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI,
				ES,CS,SS,DS,FS,GS,
				EFLAGS;
				};	
			};
			opcodes{ list{ ANY=1; EQU=2;}; }
			
	};
	
	finder {
			startoffsetForChecks=0;
			enable_emulator=1;
			tester_exe { "%prg%/testing_code.exe" };	//*
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
			programm_image{ E"%prg%/finder.memimage.dmp" , E"d:/work/General/ExploitSearcher/images/finder.memimage.dmp"     };
			CodeBeing_leftright_addition=0x2000; // добавка в к испытуемому коду слева и справа
			CodeBeing_LoadAddress=0x20000000; // предпочтительный адрес загрузки испытуемого кода
			StackSize=0x100000;				// предпочтительный адрес стека
			StackBase=0x1000000;
		};
		
		analyzer{
			MinimumCmdCnt=10; // для начала подсчета очков

			scores { 
				{100;	IPOUT;};	{100;	SELF_R;};	{500;	SELF_RW;};  {100;	SELF_RWE;};
				{300;	FS_30;};	{200;	PEB;};		{200;	DLLHDR;};   {200;	DLLNAMES;};
				{100;	DLLC_R;};	{1000;	DLLC_E;};	{200;	FS_0;};   	{100;	FS_ANY;};
				{500;	IPOUT;SELF_R;};		{300;	FS_30;PEB;};	{400;	FS_30;PEB;DLLHDR};
				{800;	SELF_RWE;FS_30;PEB;};
			};
			hypothesis{
				EQU	{IPOUT,SELF_RW} 
				ANY	{FS_30,FS_0,DLLC_R,DLLC_E,PEB,DLLHDR,DLLNAMES} 
			}
			solve_NeedTest{	}; // as hypothesis
		};	
	};	
}
)const";

const char * test_tbconf_text_0=
u8R"const(
{
	s_strs {	id1=str_id1; id2=str_id2; id3=12345; id4="тест строки"; };
	a_strs {	str_id1,str_id2,12345,"тест строки" };
	e_strs {	E"--%prg%-ру-" , "no expand" };
	x_int  { 0x44, 0x55 , 88 };
}
)const";

static conf_struct_ref _list( const darray<conf_variant> & l ){
	auto jn = conf_struct_t::newnode(conf_variant::ejtArray);
	jn->a_data = l;
	//return json_variant( jn );
	return jn ;
}
static conf_struct_ref _obj( const darray<conf_struct_t::r_property> & l ){
	auto jn = conf_struct_t::newnode(conf_variant::ejtObject);
	jn->named_props = l;
	//return json_variant( jn );
	return jn;
}
static stringA expE( const stringA & s ) {
	return expand_environment_string(s);
}
static conf_struct_ref get_test_dic_0() {
	return _obj({ 
		{"s_strs" , _obj({ {"id1","str_id1"}, {"id2","str_id2"}, {"id3",12345}, {"id4",u8"тест строки"}, })}
		,{"a_strs" , _list({ "str_id1","str_id2",12345 ,u8"тест строки"}) } 
		,{"e_strs" , _list({ expE(u8"--%prg%-ру-") , "no expand" }) } 
		,{"x_int" , _list({ 0x44, 0x55 , 88 }) } 
		});
}

static int cmp(test_state& err , int index, const listptr & tbc , const conf_struct_ref & nod ){
	size_t i=0;
	auto Objtype = conf_variant::ejtObject;
	for ( auto ne : *nod ) {
		auto & v= ne.Value(); auto & k= ne.Key();
		auto & ce = tbc->get(i);
		
		if (nod->isObject()) {
			if (k!= ce._key) 
				return err.pushl(__LINE__, "invalid key");
		} else {
			if (!ce._key.empty()) return err.pushl(__LINE__, "invalid key");
		}
		size_t ltp = ce._sublist.empty() ? 0 : Objtype;
		size_t rtp = v.isObjectType() ? Objtype : 0;
		if (ltp!=rtp) return err.pushl(__LINE__, "invalid type");
		if (ltp ) { 
			auto res = cmp( err, index*100+1 , ce._sublist , v.toNode() );
			if (res) return res;
		} else {
			if (v.type == conf_variant::ejtInt ) {
				int intv;
				if (!tbc->getval( i , intv )) return err.pushl(__LINE__, "invalid data type int");
				if ( intv != v.toInt() ) return err.pushl(__LINE__, "invalid int data ");
			} else if ( ce._value != v.toStr() )
				return err.pushl(__LINE__, "invalid data");
		}
		i++;
	}
	return 0;
}

static int test_start_0(test_state& err) { 
	auto cfg = tbconfig::parse_tbconf( test_tbconf_text_0 );
	if (cfg.empty()) 
		return err.pushl(__LINE__, "error load config");
	auto nod = get_test_dic_0();
	if (cmp( err , 0  , cfg , nod)) return 1;
	return 0; 
}


mDeclareTestProc(test_tbconf,esr_TestNeedUpdate){
//int test_tbconf() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	return 0;
}

//mRegisterTestProc(test_tbconf,esr_TestIsAStub);
//static tTestProcRegistrator rr(test_tbconf,"test_tbconf",esr_TestIsAStub);

}};
