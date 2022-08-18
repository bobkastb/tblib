#include <vector>
#include <string.h>

struct tstatistics {
	int cnt_Construct_0;
	int cnt_Construct_D;
	int cnt_Destruct;
	int cnt_Assign;
	void reset() { memset(this,0,sizeof(*this)); }
};

struct nMVnCPst1 {
	static tstatistics stat;
	int d;
	char val;
	int* ptr;
	nMVnCPst1() { this->d = 0; this->ptr = &d; this->stat.cnt_Construct_0++; }
	nMVnCPst1(const nMVnCPst1& v) { this->ptr = &d; this->d = v.d; val = v.val; this->stat.cnt_Construct_D++; }
	~nMVnCPst1() { this->stat.cnt_Destruct++; }
	//nMVst& operator = (const nMVst& v) { d = v.d; stat.cnt_Assign++; return *this; }
	bool test() const { return this->ptr == &this->d; }
	static const char* name() { return "nMVst"; }
};

tstatistics nMVnCPst1::stat = { 0,0,0,0 };

template<typename ERec> void init_rec_array_c(ERec* arr, size_t cnt, const char* vals = "") {
	static int start = 0;
	for (size_t i = 0; i < cnt; i++) {
		arr[i].d = start; start++;
		arr[i].val = *vals; if (*vals) vals++;
	};
}


static void test_op_copy() {
	using ERec = nMVnCPst1;
	ERec dd[24]; auto szDD = std::size(dd); init_rec_array_c(dd, szDD);
	std::vector<ERec> d1;
	ERec::stat.reset();
	d1.assign(dd, dd + szDD);
	ERec::stat.reset();
	d1.assign(dd, dd + szDD);

}

void test_stdvector_OC() {
	test_op_copy();
}