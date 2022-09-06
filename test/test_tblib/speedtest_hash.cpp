#include "test_state.h"
#include "gdata/hash_functions.h"
#include "gdata/t_format.h"


#include <time.h>

namespace tbgeneral {
	namespace test_ns {



		static void _speed_tester(int hashid, const char* m1, int timelen = 15) {
			byte cdata[16] = { 0,1,2,3,4,5,6,7,8,9 };
			auto h = cHash_Base::MakeHash(hashid);
			auto t = clock();
			for (int i = 0; i < 10000000; i++) {
				auto hr = h->CalcHash(cdata, sizeof(cdata));
				if (0 == i % 500000) {
					float tm = float(clock() - t) / CLOCKS_PER_SEC;
					printf("speedtest::%s:: .[%d] time:%f  sp:%f\n", m1, i, tm, i / tm);
					if (tm > timelen) break;
				}
			}
		}

		static void test_hash__speed() {
			printf("test_hash_speed:start\n");
			_speed_tester(cHash_Base::MD5, "MD5", 10);
			_speed_tester(cHash_Base::SHA256, "SHA256", 10);

			printf("end...\n");
		}

mDeclareTestProc(test_hash_speed,esr_TestIsAStub){
	return 0;
}
	}
}; // namespace tbgeneral { namespace test{
