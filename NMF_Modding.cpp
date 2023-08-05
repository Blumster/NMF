#pragma once

#ifndef NMF_VERSION
#include "NMF.h"
#endif

#ifdef NMF_USE_MODDING

#include <stdint.h>

namespace NMF
{
    ModManagerBase* ModManagerBase::Instance{ nullptr };
}

#endif
