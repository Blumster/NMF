#pragma once

#include <stdint.h>

#include "common.h"

namespace NMF
{
    enum class WndProcHandleResult : uint8_t
    {
        Continue,
        Handled,
        Default
    };

    enum class ImGuiDrawTarget : uint8_t
    {
        MainMenu,
        General
    };

    //NMF_EXPORT SDKError SetupSDK();
    //NMF_EXPORT SDKError TeardownSDK();
    //NMF_EXPORT GameTypes GetGameType();
    NMF_EXPORT bool HostIsGame();
    NMF_EXPORT void RestoreImportTable();
    NMF_EXPORT void SDKExit(int exitCode);
}
