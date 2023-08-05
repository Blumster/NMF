#pragma once

#include <stdint.h>
#include <Windows.h>

#define NMF_VERSION "0.1"

#ifdef NMF_USER_CONFIG
#include NMF_USER_CONFIG
#endif

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

namespace NMF
{
    enum class NMFExitCode : int32_t
    {
        ImGuiManagerRegisterExternalWindow = 10,
        ImGuiManagerCreateExternalWindow   = 11,
        ImGuiManagerInvalidImGUIVersion    = 12,
        ImGuiManagerDeviceHooks            = 13,
    };

    enum class WndProcHandleResult : uint8_t
    {
        Continue,
        Handled,
        Default
    };

    WndProcHandleResult NMFWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    //NMF_EXPORT SDKError SetupSDK();
    //NMF_EXPORT SDKError TeardownSDK();
    //NMF_EXPORT GameTypes GetGameType();
    //NMF_EXPORT bool HostIsGame();

    NMF_EXPORT void NMFRestoreImportTable();
    NMF_EXPORT void NMFExit(NMFExitCode exitCode);
}

#ifdef NMF_USE_LOGGING
#include "NMF_Logging.h"
#endif

#include "NMF_Memory.h"

#ifdef NMF_USE_IMGUI
#include "NMF_ImGui.h"
#endif

#ifdef NMF_USE_EXT_LUA
#include "NMF_Lua.h"
#endif

#ifdef NMF_USE_MODDING
#include "NMF_Modding.h"
#endif

#ifdef NMF_USE_RTTI
#include "NMF_RTTI.h"
#endif

#undef NMF_DELETE_ALL_INIT
#undef NMF_DELETE_INIT
