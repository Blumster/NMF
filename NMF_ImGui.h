#pragma once

#ifdef NMF_USE_IMGUI

#include <stdint.h>

#ifndef NMF_VERSION
#include "NMF.h"
#endif

#if defined(NMF_IMGUI_DX9)
struct IDirect3D9;
struct IDirect3DDevice9;
#elif defined(NMF_IMGUI_DX10)
#error "Not implemented yet!"
#elif defined(NMF_IMGUI_DX11)
#error "Not implemented yet!"
#elif defined(NMF_IMGUI_DX12)
#error "Not implemented yet!"
#else
#error "Invalid DX version!"
#endif

namespace NMF
{
    enum class ImGuiDrawTarget : uint8_t
    {
        MainMenu,
        General
    };

    class NMF_EXPORT ImGuiManager final
    {
    public:
        static void Setup(HINSTANCE hinstance, HWND hwnd, int nShowCmd);
        static void Teardown();
        static void Render();
        static WndProcHandleResult WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void ShowMenu(bool isShown);
        static bool IsMenuShown();
        static void EndScene();

    private:
        static void RecreateContext();

        static bool IsMenuOpen;
        static HWND GameHWND;
        static HINSTANCE GameHINSTANCE;
        static int GamenShowCmd;

#ifdef NMF_IMGUI_POP_OUT
        static LRESULT CALLBACK ExternalWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        static bool RegisterExternalWindow();
        static bool CreateExternalWindow();
        static bool ShowExternalWindow();
        static bool HideExternalWindow();
        static bool CreateExternalDevice();

        static bool IsExternalWindow;
        static HWND ExternalHWND;
        static constexpr const TCHAR* ExternalWindowClassName = TEXT("NMF_class");
        static constexpr const TCHAR* ExternalWindowName = TEXT("NMF");
#endif

#ifdef NMF_USE_LOGGING
        static Logger Logger;
#endif

#ifdef NMF_IMGUI_DEMO
        static bool IsImGUIDemoOpen;
#endif

#if defined(NMF_IMGUI_DX9)
        static IDirect3DDevice9* GameDevice;
        static IDirect3D9* GameInstance;

        void SetupD3D(IDirect3D9* instance, IDirect3DDevice9* device);

#ifdef NMF_IMGUI_POP_OUT
        static IDirect3DDevice9* ExternalDevice;
#endif

#ifdef NMF_IMGUI_DX9_HOOK_VIRTUAL
        static Hook* DeviceEndSceneHook;
        static Hook* DeviceResetHook;

        static HRESULT __stdcall DX9EndScene(void* device);
        static HRESULT __stdcall DX9Reset(void* device, void* pParam);
#endif
#else
        static void* ExternalDevice;
#endif
};

#ifdef NMF_USE_CONSOLE

#ifdef NMF_USE_EXT_LUA_CONSOLE
    // wire the ext lua into the console
#else
    // wire extern functions into the console
#endif
#endif
}

#endif
