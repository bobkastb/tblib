#pragma once

#include <tb_basetypes.h>

namespace crsys {

int32_t _InterlockedDecrement(int32_t* d);
int32_t _InterlockedIncrement(int32_t* d);
int32_t _InterlockedAdd(int32_t* d, int32_t v);

}