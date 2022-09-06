#pragma once

#include "sys_mutex.h"

namespace crsys{

	template <typename Resource_t> class cResourceDistrib{
		using thistype = cResourceDistrib<Resource_t>;
		typedef Resource_t * Resource_p;
		//static thistype * ResourceDistrib;
		crsys::sys_mutex mutex;
		tbgeneral::darray<Resource_p> storage;
		public:
		Resource_p AllocResource(){ AUTOENTER(mutex);
			Resource_p res;
			if (storage.size()) {
				storage.pop_back( res ); 
			} else {
				res= new Resource_t();
			};
			return res;
		 };
		void FreeResource(Resource_p fr){ AUTOENTER(mutex); 
				if (fr) storage.push_back( fr );
		};
		static thistype & getmain() {
			static thistype* ResourceDistrib=0;
			if (!ResourceDistrib)
			{   AUTOENTERG();
				if (!ResourceDistrib) ResourceDistrib = new thistype();
			}
			return *ResourceDistrib;
		};
	};

	template <typename Resource_t> Resource_t* AllocateGlobalCashedResource(){ 
			return cResourceDistrib<Resource_t>::getmain().AllocResource(); 
	};
	template <typename Resource_t> void FreeGlobalCashedResource(Resource_t* res){ 
			cResourceDistrib<Resource_t>::getmain().FreeResource(res); 
	};

	template <typename Resource_t> struct cResourceCashProxy{
		Resource_t * res;
		cResourceCashProxy() { res = AllocateGlobalCashedResource<Resource_t>(); }
		~cResourceCashProxy() { FreeGlobalCashedResource<Resource_t>( res ); }
		Resource_t *  operator ->() const { return res;};
		Resource_t &  operator *() const { return *res;};
		private:
		cResourceCashProxy(const cResourceCashProxy & s) {}; // копирование запрещено
		cResourceCashProxy & operator = (const cResourceCashProxy & s) {}; // копирование запрещено
	};

};
