#pragma once
#include "w-metadata.h"
#include "tb_exception_base.h"
#include "tsys/t_basedsync.h"
#include "tb_algo.h"

//DEBUG_TECH_ARRAY вставляется в каждую ветку , для того чтобы при тестировании понять что код тестировщика посещает КАЖДУЮ "ветку" 
//то есть тестирование полностью "покрывает" код. Если макрос неопределен - никаких вызовов не будет
#ifndef DEBUG_TECH_ARRAY
#define DEBUG_TECH_ARRAY( eventclass ,  mask )
#endif

namespace crsys { bool testPointerInConstSegment(const void* pdata); }; //TODO:

namespace tbgeneral {


#pragma pack(push , 1)
	template<typename t_TElem> struct rc_string_header {
		typedef t_TElem TElem;
		typedef rc_string_header<TElem> thistype;
		enum { efExternalStorage=1 };
		enum { evNoRefs=0  };
		//enum {e_sdatasize = 1*sizeof(TElem) };
		//void* allocator;
		using size_type = size_t;// uint32_t;
		size_type countchars, freserve;
		int32_t refcnt;
		uint8_t flags;
		TElem fdata[1];

		//static int stat_cntr_Alloc;
		//static int stat_cntr_Free;
		size_t capacity() const { return  this ? freserve : 0; }
		size_t size() const { return  this ? countchars : 0; }
		int refcount() const { return  this ? refcnt : 0; }
		const TElem* getdata() const { return fdata; };
		TElem* getdata() { return fdata; };
		const TElem* data() const { return fdata; };
		TElem* data() { return fdata; };

		//int charsize() const { return sizeof(TElem); }
		void decuse(){
			int r = crsys::_InterlockedDecrement(&refcnt);
			if (r <= 1) {
				destructorarray(fdata, countchars);
				countchars = 0;
				if (!(flags & efExternalStorage)) {
					::free(this);
					DEBUG_TECH_ARRAY(_DBG_SHDRALLOC, 1);
				}
			};
		};
		void incuse() { crsys::_InterlockedIncrement(&refcnt); };
		thistype* incuse_() { incuse(); return this; }
		static thistype* MakeFromBuff( void * buff , size_t capa  ){
			auto res = static_cast<thistype*>(buff);
			auto cntcapa = binsize2count(capa);
			res->initthis( evNoRefs , 0 , cntcapa , efExternalStorage );
			return res;
		};

		public:
		//bool isUnique()const { return this && this->refcnt == 1; }
		bool isOwner()const { return this && this->refcnt == 1; }
		static thistype* makenew( size_t cnt, size_t allocCnt) {
			size_t nsz = sizeof(thistype) - sizeof(fdata) + allocCnt * sizeof(TElem);
			thistype* res = static_cast<thistype*>(malloc(nsz));
			DEBUG_TECH_ARRAY(_DBG_SHDRALLOC, ~1 );
			res->initthis( 1 , cnt , allocCnt);
			return res;
		}

		void set_size(size_t sz) { countchars = (uint32) sz; }
		void set_size(const t_TElem* ep) { countchars = (uint32)(ep-fdata); }
		
		void DestructFrom(t_TElem* from) {
			size_t nsz = from - fdata;
			if (nsz < countchars) {
				destructorarray(from, countchars - nsz);
				countchars = static_cast<uint32_t>(nsz);
			}
			else if (nsz > countchars)
				countchars = static_cast<uint32_t>(nsz);
				//TODO: RAISE("Invalid operation");
		}
		void ConstructFor(t_TElem* st , size_t count) {
			constructorarray(st, count);
			change_size_m(st + count);
		}
		void ConstructFrom(t_TElem* to, const t_TElem* from , size_t count) {
			constructorarray(to , count , from , count );
			change_size_m( to + count);
		}
		size_t count2binsize(size_t capacity) { return sizeof(thistype) - sizeof(fdata) + capacity * sizeof(TElem); }
		static size_type binsize2count(size_t bincapacity) { return (bincapacity - sizeof(thistype) + sizeof(fdata))/sizeof(TElem); }
	private:
		void initthis( int32_t _refcnt, size_type count , size_type capa , uint8_t _flags=0  ){
			refcnt = _refcnt;
			countchars = count;
			freserve = capa;
			flags = _flags;
		}
		void change_size_m( t_TElem* to ){
			auto fin = this->fdata + this->freserve;
			if ( to<fdata || to > fin) 
				RAISE("range error in rc_string_header");
			countchars = std::max<size_type>(countchars, to - fdata);
		}

	};//template<class TChar> struct string;

#pragma pack(pop)

}


//#include "t_array.h"



namespace tbgeneral {

	template<typename TChar, int c_mode> class rc_basic_array {
	public:
		enum {
			eIsStrMode = c_mode & ebao_STRMODE ? 1 : 0
			, eAddZeroE = eIsStrMode
			, eRESETonRESIZE0 = 0 //c_mode & 4 ? 1 : 0 //eIsStrMode
		};
		enum opOptions {
			eoptAuto = 1,
			eoptNoNewConstruct = 2, // resize без вызова конструктора для новых данных
			eoptNoCopyConstruct = 4, // resize не копирует данные в новую область и не инициализирует данные в новой области
			eoptMakeUnique = 8, // resize должен сделать объект уникальным владельцем , имеет смысл если новый размер <= старого размера
			eoptNoDecUse = 0x10, // resize не освобождает старую память
			//eoptNoRealloc = 0x100, // resize не должен делать реаллок если нужно уменьшение размера
			//eoptNoRemoveStorage = 0x20, // resize не должен освобождать fstorage 
			eoptIncreaseCapacity = 0x40, // resize не должен освобождать fstorage 
			eoptUserResize = 0x80,
			eoptAutoMsk = 0xFF
		};
		using thistype = rc_basic_array<TChar, c_mode>;
		using storagetype = rc_string_header<TChar>;

	public:
		storagetype* fstorage;
		TChar* sdata;
	//protected:
		size_t fsize;

	public:

		using value_type = TChar;
		//traits_type	char_traits<char>
		//allocator_type	allocator<char>
		using reference = TChar&;
		using const_reference = const TChar&;
		using pointer = TChar*;
		using const_pointer = const TChar*;
		using iterator = TChar*; //a random access iterator to char(convertible to const_iterator)
		using const_iterator = const TChar*; // a random access iterator to const char
		using reverse_iterator = TChar*; // reverse_iterator<iterator>
		using const_reverse_iterator = const TChar*; //	reverse_iterator<const_iterator>
		//difference_type	ptrdiff_t
		using size_type = size_t;
		static constexpr auto npos{ static_cast<size_type>(-1) };


		rc_basic_array() { fstorage = 0; sdata = 0; fsize = 0; }
		~rc_basic_array() { done(); };
		size_type size() const { return fsize; }
		size_type storage_capacity() const { return fstorage ? fstorage->capacity() - eAddZeroE : 0; }
		size_type capacity() const {
			size_type r = fstorage ? fstorage->capacity() - startshift() : 0;
			return r ? r - eAddZeroE : r;
		}
		bool empty() const { return size() == 0; }
		size_type max_size() const { return -1; }
		iterator begin() { return data();  }
		const_iterator begin() const { return data(); }
		const_iterator cbegin() const { return data(); }
		iterator end() { return data()+size(); }
		const_iterator end() const { return data()+size(); }
		const_iterator cend() const { return data() + size(); }
		//reverse_iterator rbegin() { return data(); }
		//const_reverse_iterator rbegin() const { return data(); }
		//const_reverse_iterator crbegin() const { return data(); }
		//reverse_iterator rend() { return data(); }
		//const_reverse_iterator rend() const { return data(); }
		//const_reverse_iterator crend() const { return data(); }

		//void shrink_to_fit() { reserve() }
		//TODO:resize
		void resize(size_type cnt) { doresize(cnt, eoptUserResize | eoptMakeUnique); };
		//TODO:reserve
		void reserve(size_t cnt) {
			doreserve(cnt);
		};

		pointer data() { if (eIsStrMode) makeunique(); return sdata; }
		const_pointer data() const { return sdata; }
		const_pointer cdata() const { return sdata; }
		const TChar& at(size_t i) const { _rangecheck(i); return data()[i]; }
		TChar& at(size_t i) { _rangecheck(i); if (eIsStrMode) makeunique(); return data()[i]; }
		const TChar& operator [] (size_t i) const { return at(i); }
		TChar& operator [] (size_t i) { return at(i); }
		const TChar& front() const { return at(0); }
		TChar& front() { return at(0); }
		const TChar& back() const { return at(size() - 1); }
		TChar& back() { return at(size() - 1); }
		const storagetype* getstorage() const { return fstorage; }

		//void lappend(const thistype& d) { return append(d.data(), d.size()); };
		void lappend(const TChar* d, size_t count) { linsert(size(), d, count); }; //TODO:
		void lassign(const thistype& d) {
			if (d.fstorage != fstorage) {
				DEBUG_TECH_ARRAY(_DBG_ARR_ASSIGNTHIS, ~0 );
				if (d.fstorage) d.fstorage->incuse();
				done();
				fstorage = d.fstorage;
			}
			this->sdata = d.sdata; this->fsize = d.fsize;
		};
		void lassign(const TChar* d, size_t count) {
			if (!count) { done(); return; }
			// нужно делать проверку на пересечение sdata,fstorage->size()  и  d,count :: eoptMakeUnique|DestructFrom может прибить данные
			storagetype* savest = (fstorage && intersection_tmem(sdata, size(), d, count)) ? fstorage->incuse_() : 0;
			if (fstorage && isUnique()  ) { // для уникального слайса. Слайсу больше не нужна точность, достаточно объема
				sdata = fstorage->fdata; fsize = fstorage->size();
				DEBUG_TECH_ARRAY(_DBG_ARR_ASSIGN_DATA,  2);
			} 
			auto oldst = fstorage;	auto oldsize = size();
			doresize(count, eoptNoCopyConstruct | eoptNoNewConstruct | eoptMakeUnique); // repack-а быть не должно (нет уникального слайса)
			if (oldst == fstorage) { // местоположение не изменилось 
				auto cpysz = std::min(oldsize, fsize);
				copydata(sdata, d, cpysz);
				fstorage->ConstructFrom(sdata+ cpysz, d+cpysz, fsize- cpysz);
				DEBUG_TECH_ARRAY(_DBG_ARR_ASSIGN_DATA, 4);
			} else { // местоположение  изменилось
				fstorage->ConstructFrom(sdata, d, count);
				DEBUG_TECH_ARRAY(_DBG_ARR_ASSIGN_DATA, ~7 );
			}
			if (savest) {
				savest->decuse();
				DEBUG_TECH_ARRAY(_DBG_ARR_ASSIGN_DATA, 1);
			}
		};
		void copy_from( const thistype& d ) { lassign( d.cbegin(), d.size() ); }
		void linsert(size_t pos, const TChar* d, size_t count) {
			if (!count) return;
			// нужно делать проверку на пересечение sdata,fstorage->size()  и  d,count :: eoptMakeUnique может прибить данные
			storagetype* savest = (fstorage && intersection_tmem(sdata, size(), d, count)) ? fstorage->incuse_() : 0;
			auto oldsz = size() , newsz=oldsz+count;
			if (pos > oldsz) { pos = oldsz; }
			auto olddata = sdata; auto oldstorage = fstorage; 
			auto _isConst = isConst(); 
			auto oldisUnique = isUnique();
			// storage_capacity
			//if (isUnique() && storage_capacity() >= newsz) {}
			doresize(newsz, eoptNoDecUse| eoptNoCopyConstruct | eoptNoNewConstruct | eoptMakeUnique | eoptIncreaseCapacity);
			//TODO: repack в doresize тоже не стоит выполнять если добавление не в конец. Можно убрать лишние перемещения здесь
			
			auto cnt_cpy = oldsz - pos; // количество элементов для сдвига вперед , пересечение может быть только если count<cnt_cpy
			if (oldstorage != fstorage ) { // oldstorage!= Это код вставки если произошло выделение в новой области
				if (oldsz>0) {
				if (!_isConst && oldisUnique && MI_ISMOVED(TChar)) { // ( не константа && способность переноса & уникальность )
					DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, 2);
					oldstorage->DestructFrom(olddata + oldsz);
					movedataX<TChar, 0>(sdata, olddata, pos);
					movedataX<TChar, 0>(sdata + pos + count, olddata + pos, oldsz - pos);
					oldstorage->set_size(olddata); // без деструкторов старой области за исключением тех , которые не попали в слайс
				}
				else { // обычная трактовка! Данные константа | нет переноса /
					DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, 4);
					fstorage->ConstructFrom(sdata, olddata, pos);
					fstorage->ConstructFrom(sdata + pos + count, olddata + pos, oldsz - pos);
				}};
				fstorage->ConstructFrom(sdata + pos, d, count);
			} else  if (oldsz == pos) { // Это добавление, вставки нет
				DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, 8);
				fstorage->ConstructFrom(sdata + pos, d, count);
			} else { // Это код вставки. Сдвиг и копирование. если выделение новой области памяти не произошло
				if (MI_ISMOVED(TChar)) { // сдвиг и вставка перемещаемых объектов
					DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, 0x10 );
					movedataX<TChar, 0>(sdata + pos + count, sdata + pos, cnt_cpy);
					fstorage->ConstructFrom(sdata + pos, d, count); // раздвинутый участок считаем неинициализированным
				}else { // сдвиг и вставка НЕ перемещаемых объектов 
					auto retc= constructorOrCopy(sdata + pos + count, sdata + pos, cnt_cpy );
					if (count >= cnt_cpy) { // нет пересечения (count>=cnt_cpy). Частично копирование, частично конструктор копирования
						DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, 0x20);
						copydata(sdata + pos, d, cnt_cpy ); // тут (sdata + pos) инициализированно
						fstorage->ConstructFrom(sdata + oldsz, d+cnt_cpy , count - cnt_cpy ); // тут (sdata + oldsz) нужен конструктор
					} else { // есть пересечение (count<cnt_cpy), только копирование
						DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, ~0x3F);
						copydata(sdata + pos, d, count );
					}

				}
			}
			if (oldstorage && oldstorage != fstorage) oldstorage->decuse();
			if (savest) {
				savest->decuse(); DEBUG_TECH_ARRAY(_DBG_ARR_INSERT, 1);
			}
		};
		void lerase(size_t pos = 0, size_t count = npos) {
			auto oldsize = size();
			if (pos >= oldsize) return;
			auto maxsz = oldsize - pos;
			if (count == npos || count > maxsz) count = maxsz;
			auto newsz = oldsize - count;
			if (!newsz) { 
				DEBUG_TECH_ARRAY(_DBG_ARR_ERASE , 1);
				resize(0); return; }
			// TODO: if fstorage==0 & const seg
			if (isUnique()) {
				DEBUG_TECH_ARRAY(_DBG_ARR_ERASE, 2);
				DestructStorageAtEnd();
				//movedataX<TChar, 1>(sdata + pos, sdata + pos + count, oldsize - pos);
				auto mvsz = oldsize - (pos + count);
				auto finsz = movedataX<TChar, 1>(sdata + pos, sdata + pos + count, oldsize - (pos+count) );
				fstorage->set_size((sdata + oldsize - finsz) - fstorage->fdata); // подсказываем для fstorage что размер инициализированных данных уменьился изза movedataX
				fsize = oldsize - count;
				DestructStorageAtEnd();
				putzero_toend();
			}
			else {
				DEBUG_TECH_ARRAY(_DBG_ARR_ERASE, 4);
				auto oldst = fstorage; auto olddata = sdata;
				fstorage = 0; fsize = 0; // отцепимся чтобы не сделать decuse в  makenew
				auto newsz = oldsize - count;
				//doresize(oldsize - count, eoptNoCopy | eoptMakeUnique  );
				makenew(newsz, newsz, eoptNoNewConstruct | eoptNoCopyConstruct );
				fstorage->ConstructFrom(sdata, olddata, pos);
				fstorage->ConstructFrom(sdata + pos, olddata + pos + count, oldsize - (pos + count));
				if (oldst) oldst->decuse();
			}
			DEBUG_TECH_ARRAY(_DBG_ARR_ERASE, ~7);
		}; //TODO:
		iterator erase(size_t pos = 0, size_t count = npos) { lerase(pos, count); return begin()+pos; };
		iterator erase(iterator p) { lerase(p - cbegin(), 1); return p; };
		iterator erase(iterator p, iterator last) { lerase(p - cbegin(), last - p); return p; };
		
		void swap(thistype& d) { auto t = d; d = *this; *this = t; }
		size_t push_back(const TChar d) { auto r=size(); lappend(&d, 1); return r; }
		bool pop_back() { if (size()) erase(&back()); else return false; return true; } // RAISE("pop_back:invalid size");
		bool pop_back(TChar& rv) { if (!size()) return false;
			auto b = &back(); rv = *b; erase( b ); return true;	}
		thistype& operator = (const thistype& d) { lassign(d); return *this; }


		// new functions
		void lslice(thistype& res, size_type sti, size_type count = npos) const {
			if (count == npos) count = fsize - sti;
			if (count > fsize - sti) rangeerror();
			res = *this;
			res.sdata = sdata + sti; res.fsize = count;
		} //TODO:
		//bool isUnique() { return !fstorage || fstorage->isOwner(); }
		bool isUnique()const { return fstorage && (fstorage->isOwner()); }
		bool isConst()const { return !fstorage && fsize; }
		bool isSlice()const { return fstorage && fstorage->fdata != sdata && fstorage->size() != fsize; }

		void makeunique() {
			if (!isUnique()) {
				auto oldsz = size();
				if (!oldsz) return;
				makenew(oldsz, oldsz, eoptUserResize);
				DEBUG_TECH_ARRAY(_DBG_ARR_MAKEUNIQUE, ~0);
			}
		}
		void clear() { done(); }
		void assign_cnstval(const TChar* d, size_t count) { done(); sdata = const_cast<TChar*>(d); fsize = count;  };
		void assign_storage(storagetype* newst){
			if (newst) newst->incuse();
			done();
			if (newst) {
				fstorage = newst; sdata = newst->getdata(); fsize = newst->size();
				putzero_toend();
			}
		}
		void assign_storage(void* localbuff , size_t capa) { assign_storage(storagetype::MakeFromBuff( localbuff, capa) ); }
		template<size_t N> void assign_storage(TChar(&v)[N]) { assign_storage(v, N*sizeof(TChar)); }

		void free_external_storage(void* localbuff) {
			if (localbuff!=fstorage) return;
			auto s = fstorage; s->incuse();
			makeunique();
			s->decuse();
		}


	protected:
		void rangeerror() const { RAISE("range error in darray"); }
		void _rangecheck(size_t i) const { if (((size_t)i >= size()) && (i != 0)) RAISE("range error in darray"); }
		size_t startshift() const { return fstorage ? (sdata - fstorage->fdata) : 0; }
		void putzero_toend() {
			if (eAddZeroE) sdata[fsize] = MI_GETZERO(TChar);
		}
		void DestructStorageAtEnd() {
			if (fstorage && isUnique()) fstorage->DestructFrom(sdata + fsize);
		}
		void _set_fsize(size_t newsz) {
			fsize = newsz;
			if (isUnique()) { //TODO: Вызов деструктора тут выглядит странно
				DestructStorageAtEnd();
				putzero_toend();
			}
		}
		void makenew(size_t newcnt, size_t capa_cnt, uint opts) {
			auto oldsize = size();
			capa_cnt += eAddZeroE;
			if (opts & eoptIncreaseCapacity) capa_cnt = capacity_newsz(capa_cnt);
			auto newst = storagetype::makenew(newcnt, capa_cnt);
			if (!(opts & eoptNoCopyConstruct) && oldsize) {
				auto cpy_sz = std::min(oldsize, newcnt);
				//TODO: oldsize<= newcnt
				// Moved if possible - MI_ISMOVED && Not need destructor ( is unique && is no slice )
				if (MI_ISMOVED(TChar) && isUnique() && !isConst() && !isSlice()) {
					DEBUG_TECH_ARRAY(_DBG_ARR_MAKENEW, 1);
					movedataX<TChar,0>(newst->fdata, sdata, cpy_sz);
					fsize = 0;
					fstorage->countchars = sdata - fstorage->fdata;
				} else { 
					newst->ConstructFrom(newst->fdata, sdata, cpy_sz);
					DEBUG_TECH_ARRAY(_DBG_ARR_MAKENEW, 2);
			}}
			if (!(opts & eoptNoDecUse)) done(); else reset_nodone();
			fstorage = newst;
			sdata = fstorage->fdata;
			_set_fsize(newcnt); //TODO:??
			if (newcnt > oldsize)
				if (!(opts & eoptNoNewConstruct)) fstorage->ConstructFor(sdata + oldsize, newcnt - oldsize);
			DEBUG_TECH_ARRAY(_DBG_ARR_MAKENEW, (opts & eoptNoDecUse ? 4 : 8));
			DEBUG_TECH_ARRAY(_DBG_ARR_MAKENEW, (newcnt > oldsize ? 0x10 : 0x20));
			DEBUG_TECH_ARRAY(_DBG_ARR_MAKENEW, (newcnt > oldsize && opts & eoptNoNewConstruct ? 0x40 : 0x80));
			DEBUG_TECH_ARRAY(_DBG_ARR_MAKENEW, ~0xFF );
		}
		void repack(uint opts) { // only for isUnique!!!
			if (!fstorage || !size() || !startshift() || !isUnique()) return;
			DEBUG_TECH_ARRAY(_DBG_ARR_REPACK, ~0 );
			DestructStorageAtEnd();
			//copydata(fstorage->fdata, sdata, fsize); // она будет делать копирование даже для MI_ISMOVED. А movedataX - для таких скопирует память. Деструкторов столько же, но операторов копирования - меньше
			movedataX<TChar, 1>(fstorage->fdata, sdata, fsize);
			fstorage->set_size(sdata);
			sdata = fstorage->fdata;
			DestructStorageAtEnd();
		}
		void doresize(size_t newsz, uint opts = 0) {
			auto oldsz = size();
			//TODO: нужно всегда делать MakeUnique ? 
			if (oldsz == newsz) { DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, 1 );
				if (!isUnique() && (opts & eoptMakeUnique)) {
					makenew(newsz, newsz, opts); DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, 0x100 );
				}
				return;
			} else if (!newsz) {  DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, 2 );
				if (eRESETonRESIZE0) done();
				else { _set_fsize(0); DestructStorageAtEnd(); }
				return;
			}
			else if (newsz < oldsz) { 
				if (!isUnique() && (opts & eoptMakeUnique)) { DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, 4 );
					makenew(newsz, newsz, opts);
				} else { DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, 8 );
					_set_fsize(newsz);
				}
				return;
			} // newsz > oldsz 
			if (isUnique() && fstorage->capacity()) {
				DestructStorageAtEnd();
				if (newsz <= capacity()) { 
					if (!(opts & eoptNoNewConstruct)) fstorage->ConstructFor(sdata + fsize, newsz - fsize);
					_set_fsize(newsz);
					DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, (opts & eoptNoNewConstruct ? 0x10 : 0x20 ));
					return;
				} else if (newsz <= startshift() + capacity()) { 
					repack(opts);
					if (!(opts & eoptNoNewConstruct)) fstorage->ConstructFor(sdata + fsize, newsz - fsize);
					_set_fsize(newsz);
					DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, (opts & eoptNoNewConstruct ? 0x40 : 0x80));
					return;
				}
			}
			DEBUG_TECH_ARRAY(_DBG_ARR_RESIZE, ~0x1FF );
			makenew(newsz, newsz, opts);
		}
		void doreserve(size_t new_capa) {
			auto old_capa = capacity();
			if (new_capa < size()) { new_capa = size(); }
			if (new_capa == old_capa) return;
			if (isUnique() && (startshift() + old_capa == new_capa)) {
				DEBUG_TECH_ARRAY(_DBG_ARR_RESERVE, 1 );
				repack(0);
				return;
			}
			DEBUG_TECH_ARRAY(_DBG_ARR_RESERVE, ~1);
			makenew(size(), new_capa, 0);
		}
		void reset_nodone() { sdata = 0; fsize = 0; fstorage = 0; }
		void done() {
			if (fstorage) { fstorage->decuse();  }
			reset_nodone();
		};

		//TODO: Надо как то делать безопасным!
		public:
		void shift_begin(int shift) { sdata += shift; }

	};

	//inline size_t strlen_s(const char* d) { return (d ? ::strlen(d) : 0); }
	//inline size_t strlen_s(const wchar* d) { return d ? wcslen(d) : 0; };




	template<typename TChar> class rc_array : public rc_basic_array<TChar, ebao_ARRAYMODE> {
	public:
		using thistype = rc_array<TChar>;
		//using size_type = typename rc_basic_array<TChar, ebao_ARRAYMODE>::size_type;
		using size_type = typename thistype::size_type;

		size_type length() const { return this->size(); }
		void clear() { this->done(); }
		rc_array() {};
		rc_array(const TChar* d) { assign(d); };
		rc_array(const TChar* d, size_type cnt) { this->lassign(d, cnt); };
		rc_array(const thistype& d) { this->lassign(d); };
		//inline rc_array(std::initializer_list<TChar> ilist) { lassign(ilist.begin(), ilist.size()); };
		rc_array(std::initializer_list<TChar> ilist) { this->lassign(ilist.begin(), ilist.size()); };
		
		thistype& operator += (const thistype& d) { return append(d); };
		thistype& operator = (const thistype& d) { this->lassign(d); return *this; };
		thistype& operator=(std::initializer_list<TChar> ilist) { this->lassign(ilist.begin(), ilist.size());return *this; };
		//thistype& operator = (const TChar* d) { return assign(d); };
		thistype& append(const thistype& d) { this->lappend(d.data(), d.size()); return *this; };
		thistype& append(const TChar* d, size_t count) { this->lappend(d, count); return *this; };
		thistype& append(const TChar* d) { this->lappend(d, 1); return *this; };
		thistype& append(std::initializer_list<TChar> ilist) { this->lappend(ilist.begin(), ilist.size()); return *this; };
		thistype& assign(const TChar* d) { this->lassign(d, 1); return *this; };
		thistype& assign(const TChar* d, size_t count) { this->lassign(d, count); return *this; };
		thistype& assign(const thistype& d) { this->lassign(d); return *this; };
		thistype& assign(std::initializer_list<TChar> ilist) { this->lassign(ilist.begin(), ilist.size()); return *this; };
		thistype& insert(size_t pos, const thistype& d) { this->linsert(pos, d.data(), d.size()); return *this; };
		thistype& insert(size_t pos, const TChar* d, size_t count) { this->linsert(pos, d, count); return *this; };
		thistype& insert(size_t pos, const TChar* d) { this->linsert(pos, d, 1); return *this; };
		thistype& insert(size_t pos, std::initializer_list<TChar> ilist) { this->linsert(pos,ilist.begin(), ilist.size()); return *this; };
		thistype slice(size_t sti, size_t count = -1) const { thistype res; this->lslice(res, sti, count); return res; }
		thistype makecopy() const { thistype res(this->data(), this->size());  return res; }


	}; // rc_array


	template<typename tArr> tArr slice(const tArr& src, size_t sti, size_t count = -1) {
		tArr res;
		src.lslice(res, sti, count);
		return res;
	}

	template<typename TChar, size_t N> rc_array<TChar> array2localstorage(size_t needsz , TChar(&v)[N]   ) {
		rc_array<TChar> r; r.assign_storage(v, N * sizeof(TChar)); r.resize(needsz);
		return r;
	}


}//namespace tbgeneral

namespace tbgeneral {
	template<typename EType> using darray = rc_array<EType>;
	template<typename EType> using parray = rc_array<EType>;
}


