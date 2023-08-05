#include "NMF.h"

namespace NMF
{
#define HANDLE_MODULE_WNDPROC(WNDPROC_FUNC) \
    result = WNDPROC_FUNC(hWnd, uMsg, wParam, lParam); \
    if (result != WndProcHandleResult::Continue) \
        return result == WndProcHandleResult::Default ? DefWindowProcA(hWnd, uMsg, wParam, lParam) : 0

    WndProcHandleResult WINAPI NMFWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WndProcHandleResult result = WndProcHandleResult::Continue;

        //HANDLE_MODULE_WNDPROC(GUIManager::WndProc);
        //HANDLE_MODULE_WNDPROC(ModManager::WndProc);

        return result;
    }
}
