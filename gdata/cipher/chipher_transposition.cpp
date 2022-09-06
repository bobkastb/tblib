
#include "blockCipher.h"
#include "gdata/tb_algo.h"


namespace tbgeneral{ 

cChipherKey cChipherKey::random(int sz){
	cChipherKey res;
	res.resA.reserve(sz + sizeof(int) );
	res.resA.resize( sz );

	auto s=res.resA.data(); auto e=s+res.resA.size();
	for( ; s<e; s+=4 )
		*(int*)s= std::rand();
	return res;
};

cChipherKey c_transposition::preparekey(const cChipherKey & _key){
	auto key = _key;
	if (key.getkeysize()<16) { key.init(32); } 
	//auto szk= key.getkeysize();
	return key;
};

darray<int> c_transposition::generate( const cChipherKey & _key , size_t count){
	darray<int> data; data.resize(count); auto fdata= data.data();
	for(size_t i=0;i<count;i++) fdata[i]= static_cast<int>(i);
	c_transposition::make( _key , fdata , count);
	return data;
};


void c_Transposition_Chifer::start(){}; 
void c_Transposition_Chifer::encode(void * data, size_t sz){
	c_transposition::make( fKey , (byte*)data , sz);
};
void c_Transposition_Chifer::decode(void * data, size_t sz){
	c_transposition::make( fKey , (byte*)data , sz);
};
void c_Transposition_Chifer::init( const cChipherKey & key ){ 
	fKey = c_transposition::preparekey(key);
}

}; // tbgeneral{ 


namespace tbgeneral{ namespace test{

void test_c_transposition(){
	enum { size=1024 };
	//int i,j;swap(i,j);
	byte map1[size/8+1] ; ZEROMEM(map1);
	auto da=c_transposition::generate("testkey1",1024);
	FOREACH( d , da ) {
		if (map1[*d/8] & (1<<(*d & 7))) 
			throw "test_c_transposition - ERROR!";
		map1[*d/8] |= (1<<(*d & 7));
	};
	for (int i=0;i<size/8;i++) {
		if (!map1[i])
			throw "test_c_transposition - ERROR!";
	}
	printf("test_c_transposition - OK!");
}


}}; //namespace tbgeneral::test
