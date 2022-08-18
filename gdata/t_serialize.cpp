#include "tb_serialize.h"
#include "hash_functions.h"
#include "tb_algo.h"
namespace tbgeneral {

	bool t_serializer::checkread(size_t readsz, size_t* maxsz) {
		auto msz = size() - getpos();
		if (maxsz) *maxsz = msz;
		return readsz <= msz;
	};
	bool t_serializer::calc_crc_cmp(t_label start, t_label end, byte* ebuff, size_t buffsz) {
		if (end < start || buffsz == 0) return false;
		int32 sz = static_cast<int32>( end - start), csz = 0;
		auto savepos = getpos();
		seek(start);
		if (!checkread(sz, 0)) return false;
		auto h = cHash_Base::MakeHash(cHash_Base::MD5);
		byte buff[10 * 1024];
		h->start();
		for (; sz > 0; sz -= csz) {
			csz = min<int>(sz, sizeof(buff));
			read(buff, csz);
			h->calc(buff, csz);
		};
		h->end();

		seek(savepos);
		csz = static_cast<int32>(min(buffsz,   h->hResult.size() ));
		bool res = 0 == memcmp(ebuff, h->hResult.data(), csz);
		memcpy(ebuff, h->hResult.data(), csz);
		return res;
	};

	#pragma pack(push , 1)
	struct crc_header_t{
		uint32 sz; 
		byte b[cHash_Base::MD5_SIZE];
	};
	#pragma pack(pop )

	t_serializer::t_label t_serializer::make_start_crc() {
		t_label res = getpos();
		//byte b[cHash_Base::MD5_SIZE + sizeof(uint32)];
		crc_header_t head;
		ZEROMEM(head);
		write(&head, sizeof(head));
		return res;
	};

	bool t_serializer::fix_end_crc(t_label l) {
		auto savepos = getpos();
		//int32 sz; byte b[cHash_Base::MD5_SIZE];
		crc_header_t head;
		//read(&sz,sizeof(sz)); read(b,sizeof(b));
		//auto start = l + sizeof(sz) + sizeof(b);
		auto start = l + sizeof( head );
		calc_crc_cmp(start, savepos, head.b, sizeof(head.b));
		seek(l);
		head.sz = static_cast<int32>(savepos - start);
		//write(&sz, sizeof(sz)); write(b, sizeof(b));
		write(&head ,  sizeof( head ) );
		seek(savepos);
		return true;
	};
	bool t_serializer::check_crc() {
		//int32 sz; byte b[cHash_Base::MD5_SIZE];
		//read(&sz, sizeof(sz)); read(b, sizeof(b));
		crc_header_t head;
		read(&head ,  sizeof( head ) );
		if (head.sz == 0) return true;
		auto savepos = getpos();
		bool res = calc_crc_cmp(savepos, savepos + head.sz, head.b, sizeof(head.b));
		return res;
	};

	size_t t_mem_serializer::write(const void* data, size_t sz) {
		if (r_position + sz > buff.size())
			buff.resize(r_position + sz);
		memcpy(&buff[r_position], (const byte*)data, sz);
		r_position += sz;
		return sz;
	};
	size_t t_mem_serializer::read(void* data, size_t sz) {
		if (r_position + sz > buff.size())
			RAISE_I(eSerializer_ReadEOF);
		memcpy(data, &buff[r_position], sz);
		r_position += sz;
		return sz;
	};

};


