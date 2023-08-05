#ifdef NMF_USE_IMGUI
#include "NMF.h"

#include "lib/imgui/imgui.h"
#include "lib/imgui/backends/imgui_impl_win32.h"

#if defined(NMF_IMGUI_DX9)
#include "lib/imgui/backends/imgui_impl_dx9.h"
#include <d3d9.h>
#elif defined(NMF_IMGUI_DX10)
#include "lib/imgui/backends/imgui_impl_dx10.h"
#error "Not implemented yet!"
#elif defined(NMF_IMGUI_DX11)
#include "lib/imgui/backends/imgui_impl_dx11.h"
#error "Not implemented yet!"
#elif defined(NMF_IMGUI_DX12)
#include "lib/imgui/backends/imgui_impl_dx12.h"
#error "Not implemented yet!"
#else
#error "Invalid DX version!"
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace NMF
{
    bool ImGuiManager::IsMenuOpen{ false };
    HWND ImGuiManager::GameHWND{ nullptr };
    HINSTANCE ImGuiManager::GameHINSTANCE{ nullptr };
    int ImGuiManager::GamenShowCmd{ 0 };

#ifdef NMF_USE_LOGGING
    Logger ImGuiManager::Logger{ "ImGuiManager" };
#endif

#ifdef NMF_IMGUI_POP_OUT
    bool ImGuiManager::IsExternalWindow{ false };
    HWND ImGuiManager::ExternalHWND{ nullptr };
#endif

#ifdef NMF_IMGUI_DEMO
    bool ImGuiManager::IsImGUIDemoOpen{ false };
#endif

#if defined(NMF_IMGUI_DX9)
    IDirect3DDevice9* ImGuiManager::GameDevice{ nullptr };
    IDirect3D9* ImGuiManager::GameInstance{ nullptr };

#ifdef NMF_IMGUI_POP_OUT
    IDirect3DDevice9* ImGuiManager::ExternalDevice{ nullptr };
#endif

#ifdef NMF_IMGUI_DX9_HOOK_VIRTUAL
    using D3DeviceEndSceneFunction = HRESULT __stdcall(void*);
    using D3DeviceResetFunction = HRESULT __stdcall(void*, void*);

    Hook* ImGuiManager::DeviceEndSceneHook{ nullptr };
    Hook* ImGuiManager::DeviceResetHook{ nullptr };
#endif
#else
#error "Not implemented yet!"
#endif

    void ImGuiManager::Setup(HINSTANCE hinstance, HWND hwnd, int nShowCmd)
    {
        GameHINSTANCE = hinstance;
        GameHWND = hwnd;
        GamenShowCmd = nShowCmd;

#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "Setting up ImGuiManager...");
#endif

        if (!RegisterExternalWindow())
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Could not register external window!");
#endif

            NMFExit(NMFExitCode::ImGuiManagerRegisterExternalWindow);
            return;
        }

        if (!CreateExternalWindow())
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Could not create external window!");
#endif

            NMFExit(NMFExitCode::ImGuiManagerCreateExternalWindow);
            return;
        }

        if (!IMGUI_CHECKVERSION())
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Invalid ImGUI version!");
#endif

            NMFExit(NMFExitCode::ImGuiManagerInvalidImGUIVersion);
            return;
        }

        return;
    }

#if defined(NMF_IMGUI_DX9)
    void* GetVirtualFunction(void* obj, int index)
    {
        return (*static_cast<void***>(obj))[index];
    }

    void ImGuiManager::SetupD3D(IDirect3D9* instance, IDirect3DDevice9* device)
    {
        GameInstance = instance;
        GameDevice = device;

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(GameHWND);
        ImGui_ImplDX9_Init(GameDevice);

#ifdef NMF_IMGUI_DX9_HOOK_VIRTUAL
        DeviceEndSceneHook = MemoryManager::CreateAndApplyHook(GetVirtualFunction(device, 42), &ImGuiManager::DX9EndScene);
        DeviceResetHook = MemoryManager::CreateAndApplyHook(GetVirtualFunction(device, 16), &ImGuiManager::DX9Reset);

        if (DeviceEndSceneHook == nullptr || DeviceResetHook == nullptr)
        {
            Logger.Log(LogSeverity::Error, "ImGuiManager: unable to hook EndScene/Reset! Exiting...");

            NMFExit(NMFExitCode::ImGuiManagerDeviceHooks);
        }
#endif

#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "ImGuiManager has been set up!");
#endif
    }

    void ImGuiManager::EndScene()
    {
#ifdef NMF_IMGUI_POP_OUT
        if (IsExternalWindow)
        {
            IDirect3DDevice9_Clear(ExternalDevice, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 255), 0, 0);
            IDirect3DDevice9_BeginScene(ExternalDevice);

            Render();

#ifdef NMF_IMGUI_DX9_HOOK_VIRTUAL
            DeviceEndSceneHook->GetOriginalFunction<D3DeviceEndSceneFunction*>()(ExternalDevice);
#else
            IDirect3DDevice9_EndScene(ExternalDevice);
#endif

            IDirect3DDevice9_Present(ExternalDevice, NULL, NULL, NULL, NULL);

            RedrawWindow(ExternalHWND, NULL, NULL, RDW_INTERNALPAINT);

            return;
        }
#endif

        if (IsMenuOpen)
            Render();
    }

#ifdef NMF_IMGUI_DX9_HOOK_VIRTUAL
    HRESULT __stdcall ImGuiManager::DX9EndScene(void* device)
    {
        HRESULT result = DeviceEndSceneHook->GetOriginalFunction<D3DeviceEndSceneFunction*>()(device);

        EndScene();

        return result;
    }

    HRESULT __stdcall ImGuiManager::DX9Reset(void* device, void* pParams)
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();

        HRESULT result = DeviceResetHook->GetOriginalFunction<D3DeviceResetFunction*>()(device, pParams);

        ImGui_ImplDX9_CreateDeviceObjects();

        return result;
    }
#endif
#else
#error "Not implemented yet!"
#endif

    void ImGuiManager::Teardown()
    {
#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "Tearing down ImGuiManager...");
#endif

#if defined(NMF_IMGUI_DX9)
        ImGui_ImplDX9_Shutdown();
#else
#error "Not implemented yet!"
#endif

        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (ExternalDevice != nullptr)
        {
#if defined(NMF_IMGUI_DX9)
            IDirect3DDevice9_Release(ExternalDevice);
#else
#error "Not implemented yet!"
#endif
            ExternalDevice = nullptr;
        }

        if (ExternalHWND != NULL)
        {
            DestroyWindow(ExternalHWND);

            ExternalHWND = NULL;
        }

        UnregisterClass(ExternalWindowClassName, GameHINSTANCE);

#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "ImGuiManager has been torn down!");
#endif
    }

    void ImGuiManager::RecreateContext()
    {
#if defined(NMF_IMGUI_DX9)
        ImGui_ImplDX9_Shutdown();
#else
#error "Not implemented yet!"
#endif
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

#ifdef NMF_IMGUI_POP_OUT
        if (IsExternalWindow)
        {
            ImGui_ImplWin32_Init(ExternalHWND);

#if defined(NMF_IMGUI_DX9)
            ImGui_ImplDX9_Init(ExternalDevice);
#else
#error "Not implemented yet!"
#endif
        }
        else
#endif
        {
            ImGui_ImplWin32_Init(GameHWND);

#if defined(NMF_IMGUI_DX9)
            ImGui_ImplDX9_Init(GameDevice);
#else
#error "Not implemented yet!"
#endif
        }
    }

    void ImGuiManager::Render()
    {
#if defined(NMF_IMGUI_DX9)
        ImGui_ImplDX9_NewFrame();
#else
#error "Not implemented yet!"
#endif
        
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Text("NMF>");

#ifdef NMF_USE_MODDING
            if (ModManagerBase::Instance != nullptr)
                ModManagerBase::Instance->OnDebugImGuiDraw(ImGuiDrawTarget::MainMenu);
#endif

#ifdef NMF_IMGUI_DEMO
            if (ImGui::MenuItem("ImGui demo"))
                IsImGUIDemoOpen = !IsImGUIDemoOpen;
#endif

#ifdef NMF_IMGUI_POP_OUT
            if (IsExternalWindow)
            {
                ImGui::SameLine(ImGui::GetWindowWidth() - 119.0f);

                if (ImGui::MenuItem("Return to game"))
                {
                    if (HideExternalWindow())
                    {
                        IsExternalWindow = false;

                        RecreateContext();
                        return;
                    }
                }
            }
            else
            {
                ImGui::SameLine(ImGui::GetWindowWidth() - 70.0f);

                if (ImGui::MenuItem("Pop out"))
                {
                    if (CreateExternalDevice() && ShowExternalWindow())
                    {
                        IsExternalWindow = true;

                        RecreateContext();
                        return;
                    }
                }
            }
#endif

            ImGui::EndMainMenuBar();
        }

#ifdef NMF_USE_MODDING
        if (ModManagerBase::Instance != nullptr)
            ModManagerBase::Instance->OnDebugImGuiDraw(ImGuiDrawTarget::General);
#endif

#ifdef NMF_IMGUI_DEMO
        if (IsImGUIDemoOpen)
            ImGui::ShowDemoWindow(&IsImGUIDemoOpen);
#endif

        ImGui::EndFrame();
        ImGui::Render();

#if defined(NMF_IMGUI_DX9)
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
#else
#error "Not implemented yet!"
#endif
    }

    WndProcHandleResult ImGuiManager::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
#ifdef NMF_IMGUI_POP_OUT
        if (!IsExternalWindow)
#endif
        {
            if (uMsg == WM_KEYDOWN && wParam == VK_F8)
                ShowMenu(!IsMenuOpen);

            if (!IsMenuOpen)
                return WndProcHandleResult::Continue;

            if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) == TRUE)
                return WndProcHandleResult::Handled;

            return WndProcHandleResult::Default;
        }

        return WndProcHandleResult::Continue;
    }

    void ImGuiManager::ShowMenu(bool isShown)
    {
        IsMenuOpen = isShown;
    }

    bool ImGuiManager::IsMenuShown()
    {
        return IsMenuOpen;
    }

#ifdef NMF_IMGUI_POP_OUT
    LRESULT CALLBACK ImGuiManager::ExternalWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_SIZE:
        {
#if defined(NMF_IMGUI_DX9)
            D3DPRESENT_PARAMETERS D3DPP;
            ZeroMemory(&D3DPP, sizeof(D3DPP));
            D3DPP.Windowed = TRUE;
            D3DPP.SwapEffect = D3DSWAPEFFECT_COPY;

            IDirect3DDevice9_Reset(ExternalDevice, &D3DPP);
#else
#error "Not implemented yet!"
#endif
            break;
        }
        case WM_CLOSE:
        {
            IsExternalWindow = false;

            RecreateContext();
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        }

        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) == TRUE)
            return 0;

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    bool ImGuiManager::RegisterExternalWindow()
    {
        WNDCLASS windowClass;
        ZeroMemory(&windowClass, sizeof(WNDCLASS));
        windowClass.lpfnWndProc = ExternalWindowProc;
        windowClass.hInstance = GameHINSTANCE;
        windowClass.hIcon = NULL;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = ExternalWindowClassName;

        return RegisterClass(&windowClass);
    }

    bool ImGuiManager::CreateExternalWindow()
    {
        if (ExternalHWND == NULL)
        {
            ExternalHWND = CreateWindowEx(
                0,
                ExternalWindowClassName,
                ExternalWindowName,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                1920 / 2,
                1080 / 2,
                NULL,
                NULL,
                GameHINSTANCE,
                NULL
            );
        }

        return ExternalHWND != NULL;
    }

    bool ImGuiManager::ShowExternalWindow()
    {
        ShowWindow(ExternalHWND, GamenShowCmd);

        if (!UpdateWindow(ExternalHWND))
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Could not show external window.");
#endif

            return false;
        }

        return true;
    }

    bool ImGuiManager::HideExternalWindow()
    {
        ShowWindow(ExternalHWND, SW_HIDE);

        if (!UpdateWindow(ExternalHWND))
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Could not hide external window.");
#endif

            return false;
        }

        return true;
    }

    bool ImGuiManager::CreateExternalDevice()
    {
        if (ExternalDevice != nullptr)
            return true;

        if (GameInstance == NULL)
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "D3D instance does not exist yet?");
#endif

            return false;
        }

        HRESULT result = -1;

#if defined(NMF_IMGUI_DX9)
        D3DPRESENT_PARAMETERS D3DPP;
        ZeroMemory(&D3DPP, sizeof(D3DPP));
        D3DPP.Windowed = TRUE;
        D3DPP.SwapEffect = D3DSWAPEFFECT_COPY;

        result = IDirect3D9Ex_CreateDevice(
            GameInstance,
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            ExternalHWND,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &D3DPP,
            &ExternalDevice);
#else
#error "Not implemented yet!"
#endif
        
        if (FAILED(result))
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Could not create D3D device: 0x%X!", result);
#endif

            return false;
        }

        return true;
    }
#endif
}

#endif
