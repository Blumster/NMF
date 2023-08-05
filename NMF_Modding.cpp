#pragma once

#include <stdint.h>

#ifndef NMF_VERSION
#include "NMF.h"
#endif

namespace NMF
{
    ModManagerBase* ModManagerBase::Instance{ nullptr };
}
