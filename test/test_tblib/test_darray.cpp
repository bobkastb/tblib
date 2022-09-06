
#include "test_darray.h"

//static std::map<size_t, std::string> get_DBG_ARR_names() 
t_operation_statistic::mapnames_t t_operation_statistic::get_DBG_ARR_names() {
	static std::map<size_t, std::string> res = { {_DBG_ARR_ASSIGNTHIS,"ASSIGN_THIS"} 
		, {_DBG_ARR_ASSIGN_DATA,"ASSIGN_DATA"}, {_DBG_ARR_INSERT,"INSERT"}  , {_DBG_ARR_ERASE,"ERASE"}
		, {_DBG_ARR_MAKEUNIQUE,"MAKEUNIQUE"}, {_DBG_ARR_MAKENEW,"MAKENEW"}, {_DBG_ARR_REPACK,"REPACK"}
		, {_DBG_ARR_RESIZE,"RESIZE"}, {_DBG_ARR_RESERVE,"RESERVE"}
	};

	return res;
}
	t_operation_statistic::~t_operation_statistic() { isinit=false; }
	t_operation_statistic::t_operation_statistic(size_t beginOPC, size_t endOPC, const mapnames_t& mn) {
		isinit = true;
		_count_op_alloc = 0;
		_count_op_free = 0;
		start_operation = beginOPC;
		countop.resize(endOPC * 16);
		mapop.resize(endOPC);
		mapNames = mn;
		reset();
	}
	void t_operation_statistic::reset() {
		memset(countop.data(), 0 , countop.size() * sizeof(countop[0]));
		memset(mapop.data(),  0 , mapop.size() * sizeof(mapop[0]));
	}
	int t_operation_statistic::checkFull(std::ostream& outdiag) {
		int errcnt = 0;
		for (size_t i = start_operation; i < mapop.size(); i++) {
			if ((mapop[i] & opMaskAll) == opMaskAll) continue;
			auto m = mapop[i] & opMaskAll; errcnt++;
			outdiag << "operclass[" << i << "]";
			if (mapNames.find(i) != mapNames.end()) outdiag << "(" << mapNames[i] << ")";
			outdiag << " bits(";
			for (size_t b = 0; b < opMaskSize; b++)
				if (0 == (m & (1 << b))) outdiag << b << ",";
			outdiag << ")\n";
		}
		return errcnt;
	}


void t_operation_statistic::onUseOperation(size_t opcode, size_t mask) {
	if (!isinit) return;
	if (opcode == _DBG_SHDRALLOC) {
		if (mask & 1) _count_op_free++; else _count_op_alloc++;
	}
	mapop[opcode] |= mask;
}

static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END , t_operation_statistic::get_DBG_ARR_names() );
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
#include "gdata/t_array.h"



//#include <format>
namespace tbgeneral { namespace test_ns {




struct tObjMapAlloc {
	using t_id = uintptr_t;
	static tObjMapAlloc g;
	t_id cntr;
	size_t count;
	std::map<t_id, int> omap;
	t_id onCreate() { auto v = cntr; cntr++;   return onCreate(v); }
	void onDestruct(const void* hp) { onDestruct((t_id)hp); };
	t_id onCreate(const void* hp) { return onCreate((t_id)hp);  }

	tObjMapAlloc() { cntr = 11; count = 0; }
	bool contains(t_id id) { 
		auto r = omap.find(id);
		//if (r == omap.end()) return false;
		//return r->second != 0;
		return ( r!=omap.end() && r->second!=0); 
	}
	void onDestruct(t_id id) { 
		if (!contains(id)) 
			throw "tObjMapAlloc:Invalid destruct";
		//return;
		omap[id] = 0;
		count--;
	};
	t_id onCreate(t_id id) {
		if (contains(id)) 
			throw "tObjMapAlloc:Invalid construct";
		//return id;
		omap[id] = 1;
		count++;
		return id;
	}
	size_t size() { return count;  }
};
tObjMapAlloc tObjMapAlloc::g;

int call_test(testf_type f, const char* pret) {
	test_state ts;
	auto oldAlObjCnt = tObjMapAlloc::g.size();
	auto v = f(ts);
	if (oldAlObjCnt != tObjMapAlloc::g.size())
		v = ts.pushe("tObjMapAlloc::No all destructor calls!");
	if (!v) return 0;
	if (!pret) pret = "";
	std::cout << "FAIL:" << pret << ":" << ts << "\n";
	return -1;
}


}} // namespaces

namespace tbgeneral {
using tObjMapAlloc = test_ns::tObjMapAlloc;
struct tstatistics {
	size_t cnt_Construct_0,
	cnt_Construct_D,
	cnt_Destruct,
	cnt_Assign;
	void reset() { ZEROMEM(*this); }
	tstatistics(size_t c0, size_t cC, size_t cD, size_t cA) { cnt_Construct_0 = c0; cnt_Construct_D = cC; cnt_Destruct = cD;  cnt_Assign = cA; }
};

struct MVst {
	tObjMapAlloc::t_id uid;
	int d;
	char val;
	static tstatistics stat;
	MVst() { stat.cnt_Construct_0++; d = 0; val = 0; uid = tObjMapAlloc::g.onCreate();	}
	MVst(const MVst& v) { d = v.d; val=0; stat.cnt_Construct_D++; uid = tObjMapAlloc::g.onCreate(); }
	~MVst() { stat.cnt_Destruct++; tObjMapAlloc::g.onDestruct(uid); }
	MVst& operator = (const MVst& v) { d = v.d; stat.cnt_Assign++; return *this; }
	bool test() const { return true;  }
	static const char* name() { return "MVst"; }
};
struct Bin_st {
	int d;
	char val;
	static tstatistics stat;
	bool test() const { return true; }
	static const char* name() { return "Bin_st"; }
};
struct nMVst {
	static tstatistics stat;
	int d;
	char val;
	int* ptr;
	nMVst() { d = 0; ptr = &d; val = 0; stat.cnt_Construct_0++; tObjMapAlloc::g.onCreate(this); }
	nMVst(const nMVst& v) { ptr = &d; val = 0; d = v.d;  stat.cnt_Construct_D++; tObjMapAlloc::g.onCreate(this); }
	~nMVst() { stat.cnt_Destruct++; tObjMapAlloc::g.onDestruct(this); }
	nMVst& operator = (const nMVst& v) { d = v.d; stat.cnt_Assign++; return *this; }
	bool test() const { return ptr==&d; }
	static const char* name() { return "nMVst"; }
};
struct nMVnCPst {
	static tstatistics stat;
	int d;
	char val;
	int* ptr;
	nMVnCPst() { d = 0; ptr = &d; val = 0; stat.cnt_Construct_0++; tObjMapAlloc::g.onCreate(this); }
	nMVnCPst(const nMVnCPst& v) { ptr = &d; val = 0; d = v.d;  stat.cnt_Construct_D++; tObjMapAlloc::g.onCreate(this); }
	~nMVnCPst() { stat.cnt_Destruct++; tObjMapAlloc::g.onDestruct(this); }
	//nMVst& operator = (const nMVst& v) { d = v.d; stat.cnt_Assign++; return *this; }
	bool test() const { return ptr == &d; }
	static const char* name() { return "nMVst"; }
};

#pragma pack(push , 1)

struct oDataSt {
	char val;
	oDataSt() { val = ' '; tObjMapAlloc::g.onCreate(this); }
	oDataSt(const oDataSt& v) { val = v.val;  tObjMapAlloc::g.onCreate(this); }
	~oDataSt() { tObjMapAlloc::g.onDestruct(this); }
	nMVst& operator = (const nMVst& v) { val = v.val;  }
	bool test() const { return true; }
	static const char* name() { return "oDataSt"; }
};

struct oBinSt {
	char val;
	bool test() const { return true; }
	static const char* name() { return "oDataSt"; }
};
#pragma pack(pop)


DEF_MOVE_METAINFO(MVst);
DEF_BINMETAINFO(Bin_st);
DEF_BINMETAINFO(oBinSt);
tstatistics MVst::stat = { 0,0,0,0 };
tstatistics nMVst::stat = { 0,0,0,0 };
tstatistics Bin_st::stat = { 0,0,0,0 };
tstatistics nMVnCPst::stat = { 0,0,0,0 };

} // namespace tbgeneral {


namespace tbgeneral { namespace test_ns {



static tstatistics stat_diff(const tstatistics& l, const tstatistics& r) { 
	tstatistics res = { 0,0,0,0 };
	res.cnt_Construct_0 = r.cnt_Construct_0 - l.cnt_Construct_0;
	res.cnt_Construct_D = r.cnt_Construct_D - l.cnt_Construct_D;
	res.cnt_Destruct = r.cnt_Destruct - l.cnt_Destruct;
	res.cnt_Assign = r.cnt_Assign - l.cnt_Assign;
	return res;
}

static int cmpstat(const tstatistics& l, const tstatistics& r, const tstatistics& res, test_state* err = 0) {
	auto diff = stat_diff(l, r);
	int ec = 0;
	if (diff.cnt_Construct_0 != res.cnt_Construct_0 ) ec |= 1;
	if (diff.cnt_Construct_D != res.cnt_Construct_D) ec |= 2;
	if (diff.cnt_Destruct != res.cnt_Destruct) ec |= 4;
	if (diff.cnt_Assign != res.cnt_Assign ) ec |= 8;
	if (err && ec != 0) 
		err->pushe("cmpstat code ", ec);
	return ec;
}

static int cmpstat(const tstatistics & l , const tstatistics& r , int c0 , int cC , int cD , int cA, test_state* err=0){
	auto diff= stat_diff(l, r);
	int ec = 0;
	if (diff.cnt_Construct_0 != c0) ec |= 1;
	if (diff.cnt_Construct_D != cC) ec |= 2;
	if (diff.cnt_Destruct != cD) ec |= 4;
	if (diff.cnt_Assign != cA) ec |= 8;
	if (err && ec != 0) err->pushe("cmp-statistc code ", ec);
	return ec;
}
template<typename ERec> int cmpstat_curr(const tstatistics& old, int c0, int cC, int cD, int cA, test_state* err = 0) {
	if (MI_ISBINARY(ERec)) return 0;
	return cmpstat(old, ERec::stat, c0, cC, cD, cA, err);
}
template<typename ERec> int cmpstat_c(test_state & err, int c0, int cC, int cD , int cA ) {
	if (MI_ISBINARY(ERec)) return 0;
	auto z = ERec::stat; z.reset();
	return cmpstat(z, ERec::stat, c0, cC, cD, cA, &err);
}
template<typename ERec> int cmpstat_c(test_state& err, const tstatistics& res) {
	if (MI_ISBINARY(ERec)) return 0;
	auto z = ERec::stat; z.reset();
	return cmpstat(z, ERec::stat, res, &err);
}

template<typename ERec> void init_rec_array_c(ERec * arr , size_t cnt , const char * vals="") {
	static int start=0;
	for (size_t i = 0; i < cnt; i++ ) { 
		arr[i].d = start; start++; 
		arr[i].val = *vals; if (*vals) vals++;
	};
}

template<typename ERec> int test_content_arr(test_state& err, const rc_array<ERec>& arr , const char * vals="") {
	auto data = arr.data();
	for (size_t i = 0; i < arr.size(); i++) {
		if (!data[i].test())
			return err.pushe("test array content", i);
		if (*vals && *vals!=data[i].val)
			return err.pushe("test array values", i);
	}
	return 0;
}

const char * stdArrayInitData = "0123456789ABCDEFGHIKLMNOPRSTQWYXZ";
static std::string makestr(int id, size_t sz) {
	auto v = std::string(stdArrayInitData);
	v.resize(sz);
	return v;
}

template<typename ERec>  void binassign(rc_array<ERec>& vec, const ERec* d, size_t sz) {
	auto vd = vec.data();
	sz = std::min(sz, vec.size());
	for (size_t i = 0; i < sz; i++) vd[i] = d[i];
}


#ifdef _MSC_VER
template<typename ERec> int test_t_stdvector(test_state& err) {
	auto &pstat = ERec::stat;
	err.head = ERec::name();
	ERec dd[24]; int szDD = (int) std::size(dd); init_rec_array_c(dd, szDD); int ins_cnt;
	std::vector<ERec> v;
	v.assign(dd, dd + szDD);
	ERec::stat.reset();
	v.assign(dd, dd + szDD-5); 
	if (cmpstat_c<ERec>(err, 0, 0, 5, szDD - 5)) 
		return err.pushl(__LINE__,"assign(dd)");
	v.assign(dd, dd + szDD ); ERec::stat.reset();
	v.insert(v.begin() + 2, dd, dd + 10);
	if (cmpstat_c<ERec>(err, 0, szDD+10, szDD, 0 )) return err.pushl(__LINE__, "assign(dd)");

	v.assign(dd, dd + szDD); v.reserve(szDD * 2); ERec::stat.reset();
	ins_cnt = 3; v.insert(v.begin() + 2, dd, dd + ins_cnt);
	if (cmpstat_c<ERec>(err, 0, ins_cnt*2, ins_cnt, szDD-2- ins_cnt)) 
		return err.pushl(__LINE__, "assign(dd)");
	// Здесь нет релокации и ins_cnt деструкторов вызвано зря. Оптимально - (0, ins_cnt, 0 , szDD-2))
	// вообще не ясно зачем тут деструктор??? для области вставки вызван деструктор а потом конструктор копирования, вместо того чтобы вызвать assign...
	
	v.assign(dd, dd + szDD); v.reserve(szDD * 2); ERec::stat.reset();
	ins_cnt = 3; v.insert(v.begin() + szDD, dd, dd + ins_cnt);
	if (cmpstat_c<ERec>(err, 0, ins_cnt , 0, 0)) return err.pushl(__LINE__, "assign(dd)");
	return 0;
}
#else 
template<typename ERec> int test_t_stdvector(test_state& err) {
	return 0;
}
#endif

template<typename ERec> int test_t_initialize(test_state& err) {
	auto& pstat = ERec::stat;
	err.head = ERec::name();
	bool ism = MI_ISMOVED(ERec);
	ERec dd[12*2]; auto szDD = (int) std::size(dd)/2;
	init_rec_array_c(dd, std::size(dd) );
	ERec::stat.reset();
	rc_array<ERec> d1(dd, szDD); if (cmpstat_c<ERec>(err, 0, szDD, 0, 0)) return err.pushe("init from array",1);
	d1.resize(4);		if (cmpstat_c<ERec>(err, 0, szDD, szDD-4, 0 )) return err.pushe("Resize(4)",2);
	d1.resize(0);		if (cmpstat_c<ERec>(err, 0, szDD, szDD , 0)) return err.pushe("Resize(0)",3);
	d1.clear(); ERec::stat.reset();
	d1.resize(szDD);    if (cmpstat_c<ERec>(err, szDD, 0, 0, 0)) return err.pushe("Resize new(..)", 4);
	d1.assign(dd, 4); 	if (cmpstat_c<ERec>(err, szDD, 0, szDD-4, 4)) return err.pushe("assign(dd,4)", 5);
		ERec::stat.reset();
	d1.assign(dd, szDD);  if (cmpstat_c<ERec>(err, 0, szDD - 4, 0 , 4)) return err.pushl(__LINE__, "assign(dd, szDD)");
		ERec::stat.reset();
	d1.assign(dd, szDD-4); if (cmpstat_c<ERec>(err, 0, 0, 4, szDD-4)) return err.pushl(__LINE__, "assign(dd, szDD-4)");
		ERec::stat.reset();
	d1.assign(dd, szDD+5); if (cmpstat_c<ERec>(err, 0, szDD+5 , szDD - 4, 0 )) return err.pushl(__LINE__, "assign(dd, szDD+5)");
	d1.clear(); d1.assign(dd, szDD); ERec::stat.reset();
	d1 = d1.slice(2, 5);
	d1.assign(dd, szDD); if (cmpstat_c<ERec>(err, 0, 0, 0, szDD)) return err.pushl(__LINE__, "slice(2,5).assign(dd, szDD)");
	d1 = d1.slice(2, 5); ERec::stat.reset();
	d1.assign(dd, szDD-4); if (cmpstat_c<ERec>(err, 0, 0, 4, szDD-4)) return err.pushl(__LINE__, "slice(2,5).assign(dd, szDD-4)");
	d1 = d1.slice(2, 5); ERec::stat.reset();
	d1.assign(dd, szDD); if (cmpstat_c<ERec>(err, 0, 4, 0, szDD - 4)) return err.pushl(__LINE__, "slice(2,5).assign(dd, szDD)");
	d1 = d1.slice(2, 5); ERec::stat.reset();
	d1.assign(dd, szDD+4); 
	if (cmpstat_c<ERec>(err, 0, szDD+4,szDD,0 )) return err.pushl(__LINE__, "slice(2,5).assign(dd, szDD+4) & realloc");

	ERec::stat.reset();
	return 0;
}



struct test_operations_t {
	int lineno;
	eOperationCode operationcode;
	size_t oflag;
	//eReserveCapacity preReserve;
	//eUseRefCount preUseRefs;
	//eTestRealloc test_isRealloc;
	std::vector<test_operations_loc_t> preSlice;
	std::vector<test_operations_loc_t> locarr;
	std::function<tstatistics()> fstNMV, fstMV;
};
std::string toString(const tstatistics& st) {  return format_ss("(0CDA= %d,%d,%d,%d )", st.cnt_Construct_0,st.cnt_Construct_D, st.cnt_Destruct,st.cnt_Assign ); }

std::string get_operation_presentation(eOperationCode operationcode, test_operations_loc_t loc , const char* opdata ) {
	std::string opinf;
	switch (operationcode) {
	case eop_Resize: opinf = format_ss("resize(%d)", loc.size);  break;
	case eop_Reserve: opinf = format_ss("reserve(%d)", loc.size);  break;
	case eop_Append: opinf = opdata ? format_ss("append(%s)", opdata) : format_ss("append([%d])", loc.size );  break;
	case eop_Insert: opinf = opdata ? format_ss("insert(%d,%s)", loc.start, opdata) : format_ss("insert(%d,[%d])", loc.start, loc.size); break;
	case eop_Erase: opinf = format_ss("erase(%d,%d)", loc.start, loc.size); break;
	case eop_Construct: opinf = format_ss("new(%d)",  loc.size); break;
	case eop_AssignIntersect:  opinf = format_ss("assignFThis(%d,%d)", loc.start, loc.size); break;
	case eop_Assign:  opinf = format_ss("eop_Assign([%d])",  loc.size); break;
	case eop_MakeUnique : opinf = format_ss("MakeUnique()"); break;
	case eop_InsertIntersect: opinf = format_ss("Insert-Intersect(%d,%d)", loc.start, loc.size);
	case eop_Trim: opinf = format_ss("Trim()");
	default: opinf = format_ss("unknown operation"); break;
	};
	return opinf;
};

template <typename ERec, size_t szDD=12> struct test_ctx_t {
	//enum { szDD = 12 };
	//using ERec = nMVst;
	ERec dd[szDD]; ERec ddbig[120];
	size_t* pOper_pos, * pOper_cnt;
	test_operations_loc_t* pslice;
	void init() {
		init_rec_array_c(ddbig, std::size(ddbig));
		init_rec_array_c(dd, szDD);
	}
	test_ctx_t(size_t* pOper_pos, size_t *pOper_cnt , test_operations_loc_t* pslice ) {
		this->pOper_pos = pOper_pos; this->pOper_cnt = pOper_cnt;
		this->pslice = pslice;
		init();
	}
	int uni_op_test(size_t lnum, test_state& err , const test_operations_t& tstOp) {
		auto& pstat = ERec::stat; // for debug view!
		bool ism = MI_ISMOVED(ERec);
		size_t oper_pos, oper_cnt;
		rc_array<ERec> d1, d2;
		int sli_start = tstOp.preSlice.size() ? 0 : -1;
		auto reserve_SZ = std::max<size_t>(szDD * 3, 128);
		for (int sl_i= sli_start ; sl_i < (int)tstOp.preSlice.size(); sl_i++ )
		for (auto _loc : tstOp.locarr) {
			auto g_count = tObjMapAlloc::g.count;
			oper_pos = _loc.start; oper_cnt = _loc.size; pOper_pos[0] = oper_pos; pOper_cnt[0] = oper_cnt;
			d1.clear();
			if (tstOp.oflag & efRESERVE) d1.reserve(reserve_SZ);
			if (tstOp.oflag & efCnstSeg)
				 d1.assign_cnstval(dd, szDD);
			else d1.assign(dd, szDD);
			if (tstOp.oflag & efREFS) d2 = d1;
			if (sl_i >= 0) 
				{ pslice[0] = tstOp.preSlice[sl_i];  d1 = d1.slice(pslice->start, pslice->size);  }
			auto st = d1.fstorage;
			ERec::stat.reset();
			switch (tstOp.operationcode) {
			case eop_Insert:  d1.insert(oper_pos, ddbig, oper_cnt);  break;
			case eop_Append:  d1.append(ddbig, oper_cnt);   break;
			case eop_Erase:  d1.erase(oper_pos, oper_cnt);   break;
			case eop_Resize:  d1.resize(oper_cnt);   break;
			case eop_Reserve:  d1.reserve(oper_cnt);   break;
			case eop_Construct: { rc_array<ERec> x(ddbig, oper_cnt); d2 = x; }; break;
			case eop_AssignIntersect: d1.assign( d1.data()+oper_pos , oper_cnt);   break;
			case eop_Assign: d1.assign(ddbig, oper_cnt);   break;
			case eop_MakeUnique: d1.makeunique();   break;
			case eop_InsertIntersect: d1.insert(oper_pos, &d1.at(oper_pos) , oper_cnt); break;
			}
			const char* errsign = 0;
			auto res_stat = ism ? tstOp.fstMV() : tstOp.fstNMV();
			auto fixstat = ERec::stat;
			if ((st != d1.fstorage) != ( (tstOp.oflag & efwRealloc) != 0)) errsign = "error realloc";
			else if (cmpstat_c<ERec>(err, res_stat )) errsign = "";
			d1.clear(); d2.clear();
			if (g_count != tObjMapAlloc::g.count) errsign = " remained undeleted object! ";
			if (errsign) {
				auto statInf = toString(fixstat);
				auto slinf = format_ss(" slice(%d [%d])", pslice->start, pslice->size);
				
				return err.pushf("line:(%d,%d) %s%s%s%s %s %s", tstOp.lineno, lnum 
					,get_operation_presentation( tstOp.operationcode , _loc, 0 ).c_str()
					,(tstOp.oflag & efRESERVE ? " Reserv" : ""), (tstOp.oflag & efREFS ? " Refs>1" : "")
					,(sl_i>=0? slinf.c_str() :"" )
					,(errsign=="" ? statInf.c_str(): "")
					,errsign);
			}
		}
		return 0;
	};
	int uni_op_test(size_t lnum, test_state& err, const std::vector<test_operations_t>& tstvec) {
		for (auto tst_op : tstvec) {
			auto e = this->uni_op_test(lnum, err, tst_op);
			if (e) return e;
		}
		return 0;
	};
}; // test_ctx_t


// тест перемещений. 

#define DST(c0, cC, cD, cA) [&]() { return tstatistics(c0, cC, cD, cA); }

template<typename ERec> int test_t_move(test_state& err) {
//	using ERec = nMVst;
	auto& pstat = ERec::stat;
	err.head = ERec::name();
	//bool ism = MI_ISMOVED(ERec);
	enum { szDD = 12 };
	//int szDD = 12
	size_t oper_pos,oper_cnt ;
	test_operations_loc_t sl;
	test_ctx_t<ERec, szDD> ctx(&oper_pos, &oper_cnt, &sl);
	std::vector<test_operations_t> operations;

	operations = {
 {0,eop_Construct, 0		 ,{},{ {0,szDD}},  DST(0, oper_cnt,0, 0) , DST(0, oper_cnt,0, 0)  }
,{1,eop_Resize , efwRealloc ,{},{ {0,szDD + 10} },  DST(oper_cnt - szDD, szDD, szDD, 0) , DST(oper_cnt - szDD, 0, 0, 0) }
,{2,eop_Resize , 0			 ,{},{ {0,szDD - 10} },  DST(0, 0, szDD - oper_cnt, 0) , DST(0, 0, szDD - oper_cnt, 0) }
,{3,eop_Reserve, efwRealloc ,{},{ {0,szDD + 120} }, DST(0, szDD, szDD, 0) , DST(0, 0, 0, 0) }
,{4,eop_Insert,	efRESERVE	 , {},{ {2,5}, {2,szDD - 2}, {2,szDD + 10} },
		DST(0, oper_cnt, 0, szDD - oper_pos) , DST(0, oper_cnt, 0, 0) }
,{5,eop_Insert, efRESERVE|efREFS|efwRealloc ,{},{ {2,5}, {2,szDD - 2}, {2,szDD + 10} },
		DST(0, szDD + oper_cnt, 0, 0) , DST(0, szDD + oper_cnt, 0, 0) }
,{6,eop_Insert, efwRealloc	 ,{},{ {2,5}, {2,szDD - 2}, {2,szDD + 10} }, //перемещение в памяти после insert с realloc и refcnt=1
		DST(0, szDD + oper_cnt, szDD, 0 ) , DST(0, oper_cnt, 0, 0) }
,{7,eop_Append, efRESERVE		 ,{},{{0,5}}, 	DST(0, oper_cnt, 0, 0) , DST(0, oper_cnt, 0, 0) }
,{8,eop_Append, efwRealloc		 ,{},{{0,5}}, 	DST(0, szDD + oper_cnt, szDD, 0) , DST(0, oper_cnt, 0, 0) }
,{9,eop_Erase,  0				 ,{}, { {2,5},{2,10} }, DST(0, 0, oper_cnt, szDD - oper_pos - oper_cnt) , DST(0, 0, oper_cnt, 0) }
,{10,eop_Erase, efREFS|efwRealloc,{}, { {2,5},{2,10} }, DST(0, szDD - oper_cnt, 0, 0) , DST(0, szDD - oper_cnt, 0, 0) }
,{11,eop_Erase, efREFS|efwRealloc,{}, { {2,szDD}},DST(0, oper_pos, 0, 0) , DST(0, oper_pos, 0, 0) }
,{12,eop_Erase, 0				 ,{}, { {2,szDD}},DST(0, 0, szDD - oper_pos, 0) , DST(0, 0, szDD - oper_pos, 0) }
,{13,eop_Erase, efREFS|efwRealloc,{}, { {2,10}},DST(0, szDD - oper_cnt, 0, 0) , DST(0, szDD - oper_cnt, 0, 0) }
,{14,eop_Erase, 0				 ,{}, { {2,10}},DST(0, 0, oper_cnt, szDD - oper_pos - oper_cnt) , DST(0, 0, oper_cnt, 0) }
,{15,eop_AssignIntersect,efwRealloc,{}, { {2,10}},DST(0, oper_cnt, szDD , 0) , DST(0, oper_cnt, szDD, 0) }
,{16,eop_Assign,0,{}, { {0,10}},DST(0, 0, szDD- oper_cnt , oper_cnt) , DST(0, 0,szDD - oper_cnt , oper_cnt) }
,{17,eop_Assign,efwRealloc		 ,{}, { {0,szDD+10}},DST(0, oper_cnt, szDD, 0) , DST(0, oper_cnt, szDD, 0) }
,{18,eop_Erase, 0				 ,{}, { {0,100}},DST(0, 0, szDD, 0) , DST(0, 0, szDD, 0) }
,{19,eop_MakeUnique,efREFS | efwRealloc,{}, { {0,0}},DST(0, szDD, 0, 0) , DST(0, szDD, 0, 0) }
,{19,eop_InsertIntersect,efRESERVE | efwRealloc,{}, { {0,2}},DST(0, szDD+oper_cnt, szDD, 0) , DST(0, szDD + oper_cnt, szDD, 0) }
,{20,eop_Assign,efCnstSeg | efwRealloc,{}, { {0,szDD},{0,6}},DST(0, oper_cnt, 0, 0) , DST(0, oper_cnt, 0, 0) }
	};

	//if (ctx.uni_op_test(__LINE__,err, operations[19])) return 1;
	if (ctx.uni_op_test(__LINE__, err , operations)) return 1; // fstNMV , fstMV

	return 0;
}



template<typename ERec> int test_t_move_slice(test_state& err) {
	auto& pstat = ERec::stat;
	err.head = ERec::name();
	//bool ism = MI_ISMOVED(ERec);
	enum { szDD = 12 };
	size_t oper_pos, oper_cnt;
	test_operations_loc_t sl;
	test_ctx_t<ERec, szDD> ctx(&oper_pos, &oper_cnt, &sl);
	std::vector<test_operations_t> operations;

	operations = {
{0,eop_Resize , 0 , {{0,2},{0,11}, {2,1},{2,5},{5,7},{5,6},{10,2},{6,6},{7,5}}, {{0,szDD}} ,
	DST(oper_cnt-sl.size,0,szDD-sl.size, (sl.start ? sl.size : 0)) , DST(oper_cnt - sl.size,0,szDD - sl.size,0) }
,{1,eop_Resize , 0 ,{{0,2},{2,3},{3,4},{4,5}},{{0,8}}, // resize без repack
	DST(oper_cnt - sl.size,0,szDD - (sl.start + sl.size), 0) , DST(oper_cnt - sl.size,0,szDD - (sl.start + sl.size),0) }
,{2,eop_Append , 0 ,{{0,3}, {2,3},{2,5}},{{0,5}}, // append без repack
		DST(0 , oper_cnt ,szDD - (sl.start + sl.size), 0) , DST( 0 , oper_cnt ,szDD - (sl.start + sl.size),0) }
,{3,eop_Append , 0 ,{{4,4}},  {{0,8}}, // append c repack
		DST(0 , oper_cnt , szDD - sl.size, sl.size) , DST(0 , oper_cnt ,szDD - sl.size,0) }
,{4,eop_Append , efREFS | efwRealloc , {{1,1},{4,4},{1,10}},  {{0,8},{0,12},{0,20}}, // append c realloc
		DST(0 , oper_cnt+ sl.size , 0 , 0) , DST(0 , oper_cnt + sl.size ,0,0) }
,{5,eop_Insert , 0 ,{ {2,5}, {4,4} , {5,3} },  {{2,4}}, // Insert без repack
		DST(0 , oper_cnt , szDD-(sl.start + sl.size), sl.size-oper_pos ) , DST(0 , oper_cnt ,szDD - (sl.start + sl.size),0) }
,{6,eop_Insert , 0 ,{ {2,8}},  {{2,4}}, // Insert  с repack сдвиг с пересечением
		DST(0 , oper_cnt , szDD - sl.size , sl.size + sl.size - oper_pos) , DST(0 , oper_cnt ,szDD-sl.size,0 ) }
,{7,eop_Reserve , 0 ,{ {2,6}},  {{0,12}}, // resereve -> repack
		DST(0, 0 , szDD - sl.size , szDD - sl.size) , DST(0, 0 , szDD - sl.size , 0 ) }
	};
	if (ctx.uni_op_test(__LINE__, err, operations[7])) return 1;
	if (ctx.uni_op_test(__LINE__, err, operations)) return 1; // fstNMV , fstMV

	return 0;

}

template<typename ERec> int test_t_const_seg(test_state& err) {
	//return 0;
	auto& pstat = ERec::stat;
	err.head = ERec::name();
	enum { szDD = 12 };
	size_t oper_pos, oper_cnt;
	test_operations_loc_t sl;
	test_ctx_t<ERec, szDD> ctx(&oper_pos, &oper_cnt, &sl);
	std::vector<test_operations_t> operations;
	enum { DF = efCnstSeg | efwRealloc, RF = efREFS , CS= efCnstSeg	};

	operations = {
{0,eop_Resize , DF , {}, {{0,szDD-2}} ,	DST(0,oper_cnt,0,0) , DST(0,oper_cnt,0,0) }
,{1,eop_Resize , DF , {}, {{0,szDD+2}} ,	DST(2,szDD,0,0) , DST(2,szDD,0,0) }
,{2,eop_Reserve , DF , {}, {{0,szDD+10}} ,	DST(0,szDD,0,0) , DST(0,szDD,0,0) }
,{3,eop_Insert  , DF , {}, {{2,10}} ,	DST(0,szDD+10,0,0) , DST(0,szDD+10,0,0) }
,{4,eop_Append  , DF , {}, {{0,10}} ,	DST(0,szDD+10,0,0) , DST(0,szDD + 10,0,0) }
,{5,eop_Erase   , DF , {}, {{0,10}} ,	DST(0,szDD-10,0,0) , DST(0,szDD-10,0,0) }
,{6,eop_Resize , DF|RF , {}, {{0,szDD - 2}} ,	DST(0,oper_cnt,0,0) , DST(0,oper_cnt,0,0) }
	};
	//if (ctx.uni_op_test(__LINE__, err, operations[0])) return 1;
	if (ctx.uni_op_test(__LINE__, err, operations)) return 1; // fstNMV , fstMV
	return 0;
}


//template<typename ERec> int test_t_move_slice(test_state& err) {}
//----------------------------------
//----------------------------------
//----------------------------------
//----------------------------------
template<typename ERec> void initchardata(rc_array<ERec> & vec, const char * iv) {
	auto sz = strlen(iv); 
	vec.clear();
	vec.resize(sz);
	auto d = vec.data();
	for (size_t i = 0; i < sz; i++) d[i].val = iv[i];
}
template<typename ERec> void insert_chardata(rc_array<ERec>& vec, size_t pos, const char* iv) {
	auto sz = strlen(iv);
	vec.insert(pos, (ERec*)iv , sz );
}
template<typename ERec> void append_chardata(rc_array<ERec>& vec,  const char* iv) {
	auto sz = strlen(iv);
	vec.append((ERec*)iv, sz);
}
template<typename ERec> std::string chardata_asstr(rc_array<ERec>& vec) {
	return std::string((char*)vec.data(), vec.size());
}
template<typename ERec> int cmp_chardata(rc_array<ERec>& vec, const char* iv) {
	auto sz = strlen(iv);
	if (sz != vec.size()) return 1;
	return strncmp((char*)vec.data(), iv, sz);
}


//template<typename ERec>
int test_dataim_v(int lineno, size_t index, test_state& err, size_t flags, const test_datachars_t & test ) {
	using ERec = oDataSt;
	rc_array<ERec> vec , vec2 ;
	auto operloc= test.operloc[0];

	if (flags & efRESERVE) vec.reserve(128);
	if (flags & efCnstSeg)
		vec.assign_cnstval((ERec*)test.source, strlen(test.source));
	else initchardata(vec, test.source );
	if (flags & efREFS) vec2 = vec;
	test_operations_loc_t sl = {0,0};
	if (test.preSlice.size()) { sl = test.preSlice[0]; vec = vec.slice(sl.start, sl.size); }
	switch (test.operationcode) {
	case eop_Reserve: vec.reserve(operloc.size); break;
	case eop_Resize: vec.resize(operloc.size); break;
	case eop_Append: append_chardata(vec, test.opdata);  break;
	case eop_Insert: insert_chardata(vec, operloc.start, test.opdata); break;
	case eop_Erase: vec.erase( operloc.start, operloc.size ); break;
	case eop_InsertIntersect: vec.insert(operloc.start, &vec.at(operloc.start), operloc.size);
	}
	std::string resstr = chardata_asstr(vec);
	auto r = cmp_chardata(vec, test.opresult );

//	if (r || errobjcnt()) {
	if (r) {
		std::string opinf = get_operation_presentation(test.operationcode, operloc, test.opdata);
		auto slinf = format_ss(" slice=%d[%d]", sl.start, sl.size);
		return err.pushf("line(%d,%d) %s->%s!=%s ctx: %s%s%s", index , lineno, opinf.c_str() , resstr.c_str(), test.opresult
			,(flags & efREFS?" REFS":""), (flags & efRESERVE ? " RESERVE" : ""), (test.preSlice.size() ? slinf.c_str() : ""));
	}
	return 0;
}

int test_dataim_v(int lineno, test_state& err, bool forAllFlags , const std::vector<test_datachars_t> & ctests) {
	using ERec = oDataSt;
	
	// f = efRESERVE | efREFS;
	std::vector<size_t> flags_list = {0};
	//if (forAllFlags) flags_list = { 0,efRESERVE ,efREFS , efRESERVE | efREFS };
	if (forAllFlags) flags_list = { 0, 1 , 2, 3, 4, 5, 6, 7 };
	for (auto flags : flags_list) 
		for (size_t i = 0; i < ctests.size(); i++) {
			if (test_dataim_v(lineno, i, err, flags, ctests[i])) return 1;
	}
	return 0;
}


static int test_data(test_state& err) {
	using ERec = oDataSt;
	auto defsrc = "0123456789AB";
	std::vector<test_datachars_t> ctests = {
		{ eop_Append , defsrc ,  {} , "xyz" , {{ 0,0 }} , "0123456789ABxyz" }
		,{ eop_Append , defsrc ,  {{1,10}} , "xyz" , {{ 0,0 }} , "123456789Axyz" }
		,{ eop_Append , defsrc ,  {{2,3}} , "xyz" , {{ 0,0 }} , "234xyz" }
		,{ eop_Insert , defsrc ,  {} , "xyz" , {{ 2,0 }} , "01xyz23456789AB" }
		,{ eop_Insert , defsrc ,  {} , "xyz" , {{ 9,0 }} , "012345678xyz9AB" }

		,{ eop_Insert , defsrc ,  {} , "abcdef" , {{ 9,0 }} , "012345678abcdef9AB" }
		,{ eop_Insert , defsrc ,  {{1,10}} , "xyz" , {{ 2,0 }} , "12xyz3456789A" }
		,{ eop_Insert , defsrc ,  {{1,10}} , "xyz" , {{ 7,0 }} , "1234567xyz89A" }
		,{ eop_Insert , defsrc ,  {{1,10}} , "abcdef" , {{ 9,0 }} , "123456789abcdefA" }
		,{ eop_Erase , defsrc ,  {} , "" , {{ 1,2 }} , "03456789AB" }

		,{ eop_Erase , defsrc ,  {} , "" , {{ 1,9 }} , "0AB" }
		,{ eop_Erase , defsrc ,  {} , "" , {{ 0,8 }} , "89AB" }
		,{ eop_Erase , defsrc ,  {} , "" , {{ 8,12 }} , "01234567" }
		,{ eop_Erase , defsrc ,  {{1,10}} , "" , {{ 1,2 }} , "1456789A" }
		,{ eop_Erase , defsrc ,  {{1,10}} , "" , {{ 1,8 }} , "1A" }

		,{ eop_Erase , defsrc ,  {{1,10}} , "" , {{ 0,8 }} , "9A" }
		,{ eop_Erase , defsrc ,  {{1,10}} , "" , {{ 8,12 }} , "12345678" }
		,{ eop_Reserve ,defsrc , {} , "" , {{ 0,2 }} , "0123456789AB" }
		,{ eop_Reserve ,defsrc , {} , "" , {{ 0,32 }} , "0123456789AB" }
		,{ eop_Reserve ,defsrc , {{1,5}} , "" , {{ 0,32 }} , "12345" }

		,{ eop_Reserve ,defsrc , {{1,5}} , "" , {{ 0,2 }} , "12345" }
		,{ eop_Resize ,defsrc , {} , "" , {{ 0,15 }} , "0123456789AB   "}
		,{ eop_Resize ,defsrc , {} , "" , {{ 0,8 }} , "01234567"}
		,{ eop_Resize ,defsrc , {{1,10}} , "" , {{ 0,16 }} , "123456789A      "}
		,{ eop_Resize ,defsrc , {{1,10}} , "" , {{ 0,3 }} , "123"}

		,{ eop_InsertIntersect ,defsrc , {} , "" , {{ 1,3 }} , "0123123456789AB"}
		
	};
//	if (test_dataim_v(__LINE__, -1, err, 0, ctests[21])) return 1;
	if (test_dataim_v(__LINE__, err, true, ctests)) return 1;
	return 0;
}

template <class TElem>  int intersection_tmem_chk(int resv, TElem* lp, size_t l_cnt, const TElem* rp, size_t r_cnt) {
	auto retv = intersection_tmem(lp, l_cnt * sizeof(TElem), rp, r_cnt * sizeof(TElem));
	if (resv != retv) {
		printf("intersection_tmem:error(%p,%d,%p,%d) == %d (need %d)", lp, int( l_cnt), rp, int(r_cnt), retv, resv);
		return -1;
	}
	return 0;
};

static int test_intersect() {
	// intersection_mem result 1)  lp in rp , 2) rp in lp , 3) lp > rp , 4) lp < rp
	using ptr = int*;
	if (intersection_tmem_chk(0, ptr(0x1000), 0x10, ptr(0x2000), 0x10)) return -1;
	if (intersection_tmem_chk(0, ptr(0x2000), 0x10, ptr(0x1000), 0x10)) return -1;
	if (intersection_tmem_chk(2, ptr(0x1000), 0x100, ptr(0x1003), 0x10)) return -1;
	if (intersection_tmem_chk(2, ptr(0x1000), 0x100, ptr(0x1000), 0x10)) return -1;
	if (intersection_tmem_chk(2, ptr(0x1000), 0x100, ptr(0x1000 + 0x100 - 0x10), 0x10)) return -1;
	if (intersection_tmem_chk(1, ptr(0x1003), 0x10, ptr(0x1000), 0x100)) return -1;
	if (intersection_tmem_chk(1, ptr(0x1000), 0x10, ptr(0x1000), 0x100)) return -1;
	if (intersection_tmem_chk(1, ptr(0x1000 + 0x100 - 0x10), 0x10, ptr(0x1000), 0x100)) return -1;
	if (intersection_tmem_chk(3, ptr(0x1010), 0x100, ptr(0x1000), 0x100)) return -1;
	if (intersection_tmem_chk(4, ptr(0x1000), 0x100, ptr(0x1010), 0x100)) return -1;
	//std::cout << "test_intersect:PASS\n";
	return 0;

}


static int test_stdvector(test_state& err) {
	//if (test_t_stdvector<nMVnCPst>(err)) return 1;
	if (test_t_stdvector<nMVst>(err)) 
		return 1;
	
	return 0;
}

//void test111(const std::vector<Bin_st> f) {}

static int test_initialize(test_state& err) {

	if (test_t_initialize<nMVst>(err)) return 1;
	if (test_t_initialize<MVst>(err)) return 1;
	if (test_t_initialize<Bin_st>(err)) return 1;
	return 0;
}

static int test_move(test_state& err) {
	//if (test_t_move<Bin_st>(err)) return 1;
	opstat_l.reset();
	if (test_t_move<nMVst>(err)) return 1;
	if (test_t_move<MVst>(err)) return 1;
	if (test_t_move_slice<nMVst>(err)) return 1;
	if (test_t_move_slice<MVst>(err)) return 1;
	if (opstat_l.checkFull(std::cout)) return 1;

	return 0;
}

static int test_const_seg(test_state& err) {
	if (test_t_const_seg<nMVst>(err)) return 1;
	if (test_t_const_seg<MVst>(err)) return 1;
	return 0;
}

// rc_array
mDeclareTestProc(test_darray,esr_TestIsReady){
//int test_darray() {
	if (call_test(test_stdvector, "test_stdvector")) return 1;
	if (call_test(test_initialize, "test_initialize")) return 1;
	if (call_test(test_move, "test_move")) return 1;
	if (call_test(test_data, "test_data")) return 1;
	if (call_test(test_const_seg, "test_const_seg")) return 1;
	return 0;
}



}} // namespace tbgeneral { namespace test_ns {
