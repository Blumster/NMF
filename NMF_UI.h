#pragma once

#include <stdint.h>

#ifndef NMF_VERSION
#include "NMF.h"
#endif

namespace NMF
{
    enum class ImGuiDrawTarget : uint8_t
    {
        MainMenu,
        General
    };
}
