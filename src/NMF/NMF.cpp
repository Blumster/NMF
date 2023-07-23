#include "NMF.h"

namespace NMF
{
    bool HostIsGame()
    {
        return false; // TODO
    }

#define HANDLE_MODULE_WNDPROC(WNDPROC_FUNC) \
    result = WNDPROC_FUNC(hWnd, uMsg, wParam, lParam); \
    if (result != WndProcHandleResult::Continue) \
        return result == WndProcHandleResult::Default ? DefWindowProcA(hWnd, uMsg, wParam, lParam) : 0

    Hook* WndProcHook = nullptr;

    LRESULT WINAPI SDKWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_SYSKEYDOWN:
        case WM_SYSCOMMAND:
        case WM_CLOSE:
        case WM_DESTROY:
        case WM_QUIT:
            return WndProcHook->GetOriginalFunction<WNDPROC>()(hWnd, uMsg, wParam, lParam);
        }

        WndProcHandleResult result;

#ifdef _DEBUG
        HANDLE_MODULE_WNDPROC(GUIManager::WndProc);
#endif
        HANDLE_MODULE_WNDPROC(ModManager::WndProc);

        return WndProcHook->GetOriginalFunction<WNDPROC>()(hWnd, uMsg, wParam, lParam);
    }
}
