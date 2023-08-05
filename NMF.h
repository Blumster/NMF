#pragma once

#include <stdint.h>
#include <Windows.h>

#define NMF_VERSION "0.1"

#ifndef NMF_EXPORT
#define NMF_EXPORT
#endif

#ifndef NMF_EXTENDED_EXPORT
#define NMF_EXTENDED_EXPORT
#endif

#define NMF_DELETE_ALL_INIT(type) \
    type() = delete; \
    type(const type&) = delete; \
    type(type&&) = delete; \
    type& operator=(const type&) = delete; \
    type& operator=(type&&) = delete;

#define NMF_DELETE_INIT(type) \
    type(const type&) = delete; \
    type(type&&) = delete; \
    type& operator=(const type&) = delete; \
    type& operator=(type&&) = delete;


#include "NMF_Memory.h"

#ifdef NMF_USE_IMGUI
#include "NMF_UI.h"
#endif

#ifdef NMF_USE_RTTI
#include "NMF_RTTI.h"
#endif

#undef NMF_DELETE_ALL_INIT
#undef NMF_DELETE_INIT

namespace NMF
{
    enum class WndProcHandleResult : uint8_t
    {
        Continue,
        Handled,
        Default
    };

    //NMF_EXPORT SDKError SetupSDK();
    //NMF_EXPORT SDKError TeardownSDK();
    //NMF_EXPORT GameTypes GetGameType();
    //NMF_EXPORT bool HostIsGame();
    //NMF_EXPORT void RestoreImportTable();
    //NMF_EXPORT void SDKExit(int exitCode);
}
