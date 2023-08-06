#pragma once

#ifndef NMF_VERSION
#include "NMF.h"
#endif

#ifdef NMF_USE_MODDING

#include <stdint.h>

namespace NMF
{
    ModManagerBase* ModManagerBase::Instance{ nullptr };

#ifdef NMF_USE_LOGGING
    Logger ModManagerBase::BaseLogger{ "ModManagerBase" };

    Logger ModBase::BaseLogger{ "ModBase" };
#endif
}

#endif
