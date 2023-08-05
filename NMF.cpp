#include "NMF.h"

#include "lib/detours/include/detours.h"

#define HANDLE_MODULE_WNDPROC(WNDPROC_FUNC) \
    result = WNDPROC_FUNC(hWnd, uMsg, wParam, lParam); \
    if (result != WndProcHandleResult::Continue) \
        return result;

namespace NMF
{
    WndProcHandleResult NMFWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WndProcHandleResult result = WndProcHandleResult::Continue;

        HANDLE_MODULE_WNDPROC(ImGuiManager::WndProc);
        HANDLE_MODULE_WNDPROC(ModManagerBase::Instance->WndProc);

        return result;
    }

    void NMFRestoreImportTable()
    {
        DetourRestoreAfterWith();
    }

    void NMFExit(NMFExitCode exitCode)
    {
#ifdef NMF_USE_LOGGING
        LogManager::Log(LogSeverity::Error, "Requested NMF exit with code: %d! Exiting...", exitCode);
#endif

        exit(static_cast<int>(exitCode));
    }
}

extern "C" __declspec(dllexport) void NMF_dummy_detours_export()
{
}

#undef HANDLE_MODULE_WNDPROC
