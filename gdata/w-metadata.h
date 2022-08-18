#pragma once

#include "tb_basetypes.h"
#include "tb_basedtools.h"
#include <algorithm>
#include "tb_algo.h"



namespace tbgeneral {
	enum { ff_UNICODECHAR = 0xFEFF };

	// ISBIN 
	// ISMOVED - флаг переносимости данных. ƒанные могут буть перемещены в памяти без вызова конструкторов и деструкторов...
	// ѕример непереносимых данных: когда данные содержать указатель на память внутри себя
	template <class C> struct smpl_metainfo { enum { ISBIN = 0, ISMOVED = 0 };  static C zerov() { return C(); }; };

#define DEF_ZBINMETAINFO( tpnm ) template<> struct smpl_metainfo<tpnm> { enum{ISBIN=1,ISMOVED=1}; static tpnm zerov(){ return 0; }; }
#define DEF_BINMETAINFO( tpnm ) template<> struct smpl_metainfo<tpnm> { enum{ISBIN=1,ISMOVED=1}; static tpnm zerov(){ tpnm zv; RESETMEM(zv); return zv; }; }
#define DEF_BINMETAINFO_CH( tpnm , isuni ) template<> struct smpl_metainfo<tpnm> { enum{ISBIN=1,ISMOVED=1,ISUNICODECHAR=isuni}; static tpnm zerov(){ return 0; }; }
#define DEF_MOVE_METAINFO( tpnm ) template<> struct smpl_metainfo<tpnm> { enum{ISBIN=0,ISMOVED=1}; static tpnm zerov(){ tpnm zv; RESETMEM(zv); return zv; }; }

	DEF_BINMETAINFO_CH(char, 0);
	DEF_BINMETAINFO_CH(wchar_t, 1);

	DEF_ZBINMETAINFO(int8_t);
	DEF_ZBINMETAINFO(int16_t);
	DEF_ZBINMETAINFO(int32_t);
	DEF_ZBINMETAINFO(int64_t);
	DEF_ZBINMETAINFO(uint8_t);
	DEF_ZBINMETAINFO(uint16_t);
	DEF_ZBINMETAINFO(uint32_t);
	DEF_ZBINMETAINFO(uint64_t);
	DEF_ZBINMETAINFO(float);
	DEF_ZBINMETAINFO(double);

#define MI_ISBINARY(tpnm) (smpl_metainfo<tpnm>::ISBIN)
#define MI_ISMOVED(tpnm) (smpl_metainfo<tpnm>::ISBIN | smpl_metainfo<tpnm>::ISMOVED)
#define MI_GETZERO(tpnm) (smpl_metainfo<tpnm>::zerov())
#define MI_ISUNICODECHAR(tpnm) (smpl_metainfo<tpnm>::ISUNICODECHAR)


	// при копировании copydata , область данных dst[..cnt] считается инициализированной!
	// копирование в не инициализированную область данных выполняется операцией "constructorarray"
	template <class TElem> void copydata(TElem* dst, const TElem* src, size_t cnt) {
		if (!cnt) return;
		if (MI_ISBINARY(TElem)) { memmove_nwarn(dst, src, sizeof(*src) * cnt); return; }
		if ((dst >= src && dst < src + cnt))
		{
			for (src += cnt - 1, dst += cnt - 1; cnt; cnt--, dst--, src--)
				*dst = *src;
		}
		else { for (; cnt; cnt--, dst++, src++) *dst = *src; }
	};


	template <class TElem> void destructorarray(TElem* dst, size_t cnt) {
		if (!cnt) return;
		if (MI_ISBINARY(TElem)) { return; }
		for (; cnt; cnt--, dst++)
			dst->~TElem();
	};
	template <class TElem> void constructorarray(TElem* dst, size_t cnt) {
		if (!cnt) return;
		memreset(dst, sizeof(*dst) * cnt);
		if (MI_ISBINARY(TElem)) return;
		for (; cnt; cnt--, dst++)
			new (dst) TElem;
	};
	template <class TElem> void constructorarray(TElem* dst, size_t cnt, const TElem* src, size_t scnt) {
		if (!cnt) return;
		if (MI_ISBINARY(TElem)) {
			memreset(dst, sizeof(*dst) * cnt);
			memcopy_nwarn(dst, src, sizeof(*dst) * std::min<size_t>(cnt, scnt));
			return;
		}
		for (; cnt && scnt; cnt--, scnt--, dst++, src++)
			new (dst) TElem(*src);
		for (; cnt; cnt--, dst++)
			new (dst) TElem;
	};

	template <class TElem> void constructorarray(TElem* dst, size_t cnt, const TElem* src) {
		constructorarray(dst, cnt, src, cnt);
	};

	// return 0 - nothing todo 1-constructorarray 2-intersect 
	template <class TElem> int constructorOrCopy(TElem* dst,  const TElem* src , size_t cnt ) {
		auto icode = intersection_tmem(dst, cnt, src, cnt);
		if (icode == 0) { constructorarray(dst, cnt, src, cnt); return 1; }
		if (icode == 1 || icode == 2) return 0; // dst==src 
		auto c = dst > src ? dst - src : src - dst;
		if (dst < src) {
			constructorarray(dst, c, src);
			copydata(dst + c, src + c, cnt - c);
		} else { // dst > src
			auto ofs = cnt - c;
			constructorarray(dst+ ofs, c, src+ofs);
			copydata(dst , src , cnt - c);
		}
		return 2;
	};

	// intersection_mem result 1)  lp in rp , 2) rp in lp , 3) lp > rp , 4) lp < rp




	// перемещение данных (movedataO) , отличается от copydata тем что 
	//		1) Деструкторы будут вызваны (если тип неперемещаем)
	//		2) начальная область данных может быть не инициализированна (destIsInit=0)
	// если TElem это перемещаемый тип, то вызовы конструкторов и деструкторов неделаются и перемещение выполняется в бинарном виде
	//		деструктор вызывается только для той области памяти dst которая перезаписывается
	// если TElem это неперемещаемый тип, то перемещение идет по правилам копирования,
	//		1) в области dest будет вызван конструктор, если destIsInit=0
	//		2) в области src будет вызван деструктор
	// destIsInit : bit 0 - dest уже инициализирован 
	// возвращает размер финализированной области 
	template <class TElem, int destIsInit > size_t movedataX(TElem* dst, TElem* src, size_t cnt  ) {
		if ((!cnt) || (dst == src)) return 0;
		enum { isMoved = MI_ISMOVED(TElem) * 2 }; 
		TElem* s, * d; size_t c;
		if (src<dst && src + cnt>dst) { s = src; d = src + cnt; c = dst - src; }
		else if (src > dst && src < dst + cnt) { s = dst + cnt; d = dst; c = src - dst; }
		else { s = src; d = dst; c = cnt; };
		// d[0..c-1] - инициализируемая область
		// s[0..c-1] - финализируемая область
		if (isMoved) { // бинарный перенос
			if (destIsInit) destructorarray(d, c);
			//memreset(d, c * sizeof(TElem));
			memmove_nwarn(dst, src, sizeof(*src) * cnt);
			memreset(s, c * sizeof(TElem));
		} else { // копирование
			if (!destIsInit) constructorarray(d, c);
			if ((dst >= src && dst < src + cnt)){
				for (src += cnt - 1, dst += cnt - 1; cnt; cnt--, dst--, src--)
					*dst = *src;
			}
			else { for (; cnt; cnt--, dst++, src++) *dst = *src; }
			destructorarray(s, c);
		};
		return c;
	}

	template <class TElem, int flags> void movedataXold(TElem* dst, TElem* src, size_t cnt) {
		if ((!cnt) || (dst == src)) return;
		enum { F = MI_ISMOVED(TElem) * 2, B = MI_ISBINARY(TElem) }; TElem* s, * d; size_t c;
		if (src<dst && src + cnt>dst) { s = src; d = src + cnt; c = dst - src; }
		else if (src > dst && src < dst + cnt) { s = dst + cnt; d = dst; c = src - dst; }
		else { s = src; d = dst; c = cnt; };
		if (F) {
			if (flags & 1) destructorarray(d, c);
			memreset(d, c * sizeof(TElem));
			memmove_nwarn(dst, src, sizeof(*src) * cnt);
			memreset(s, c * sizeof(TElem));
			if (flags & 1) constructorarray(s, c);
		}
		else {
			if (!(flags & 1)) constructorarray(d, c);
			if ((dst >= src && dst < src + cnt))
			{
				for (src += cnt - 1, dst += cnt - 1; cnt; cnt--, dst--, src--)
					*dst = *src;
			}
			else { for (; cnt; cnt--, dst++, src++) *dst = *src; }
			if (!(flags & 1)) destructorarray(s, c);
		};
	};
	template <class TElem> void movedata(TElem* dst, TElem* src, size_t cnt) {
		movedataX<TElem, 1>(dst, src, cnt);
	}

	template <class Tv > void reset_vecb(Tv& dv)
	{
		memreset(dv.data(), sizeof(dv[0]) * dv.size());
	}
	template <class Tv > void reset_vecb(Tv& dv, uint sz)
	{
		memreset(dv.data(), sizeof(dv[0]) * min(dv.size(), sz));
	}
	template <class Tv > void resize_zero(Tv& dv, uint nsz)
	{
		dv.resize(nsz);
		reset_vecb(dv);
	};

	template <class I> I capacity_newsz(I sz, I percent = 25) { I nsz = sz + (sz * percent) / 100; return (nsz < 16 ? 16 : nsz); };


	enum {
		esopt_IgnoreCase = 1, esopt_TrimOn = 8, esopt_IncZeroStr = 0x10
		, esopt_DefOnSplit = esopt_IgnoreCase | esopt_TrimOn
	}; // flags

	enum e_basic_array_options {
		ebao_STRMODE = 1,
		ebao_ARRAYMODE = 0x10
	};


}; //namespace tbgeneral


