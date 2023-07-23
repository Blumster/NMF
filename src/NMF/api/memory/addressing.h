#pragma once

#include <stdint.h>

#include "../../common.h"

namespace NMF
{
    NMF_EXPORT void* operator"" _addr(uint64_t address);
    NMF_EXPORT void* operator"" _rva(uint64_t address);
}
