#include <stdint.h>
#include <Windows.h>
#include <Psapi.h>

#include "NMF.h"

#include "lib/detours/include/detours.h"

#pragma comment(lib, "psapi.lib")

#if defined(NMF_X86)
#pragma comment(lib, "lib/detours/x86/detours.lib")
#pragma comment(lib, "lib/detours/x86/syelog.lib")
#elif defined(NMF_X64)
#pragma comment(lib, "lib/detours/x64/detours.lib")
#pragma comment(lib, "lib/detours/x64/syelog.lib")
#else
#error "Invalid Architecture!"
#endif

#if defined(NMF_STATIC_BASE)
constexpr uint64_t StaticImageBase = NMF_STATIC_BASE;
#elif defined(NMF_X86)
constexpr uint64_t StaticImageBase = 0x400000ull;
#elif defined(NMF_X64)
constexpr uint64_t StaticImageBase = 0x140000000ull;
#else
#error "Invalid Architecture!"
#endif

#define TXT_(str) L ## #str
#define TXT(str) TXT_(str)

namespace NMF
{
    void* ImageBase = nullptr;
    uint64_t ImageSize = 0x0ull;

    void* GetImageBase()
    {
        if (ImageBase == nullptr || ImageSize == 0x0ull)
        {
            MODULEINFO moduleInfo = { 0 };

            HMODULE hModule = GetModuleHandleW(NULL);
            if (hModule == 0)
            {
                exit(1);

                return 0x0ull;
            }

            GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO));

            ImageBase = moduleInfo.lpBaseOfDll;
            ImageSize = static_cast<uint64_t>(moduleInfo.SizeOfImage);
        }

        return ImageBase;
    }

    uint64_t GetImageSize()
    {
        // Force calculation of the values
        GetImageBase();

        return ImageSize;
    }

    void* operator"" _addr(uint64_t address)
    {
        if (address < StaticImageBase || address >= StaticImageBase + GetImageSize())
        {
            exit(1);

            return nullptr;
        }

        return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(GetImageBase()) + (address - StaticImageBase));
    }

    void* operator"" _rva(uint64_t address)
    {
        if (address >= GetImageSize())
        {
            exit(1);

            return nullptr;
        }

        return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(GetImageBase()) + address);
    }

#pragma region Hook & Patch
    Hook::Hook(void* gameAddress, void* hookFunc)
        : Address(gameAddress), OriginalFunc(gameAddress), HookFunc(hookFunc), Attached(0)
    {
    }

    Hook::~Hook()
    {
        Remove();
    }

    bool Hook::Apply()
    {
        if (Attached == 1)
            return false;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (DetourAttach(&OriginalFunc, HookFunc) != NO_ERROR || DetourTransactionCommit() != NO_ERROR)
        {
            DetourTransactionAbort();

            return false;
        }

        Attached = 1;

        // TODO: FlushInstructionCache();

        return true;
    }

    bool Hook::Remove()
    {
        if (Attached == 0)
            return false;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (DetourDetach(&OriginalFunc, HookFunc) != NO_ERROR || DetourTransactionCommit() != NO_ERROR)
        {
            DetourTransactionAbort();

            return false;
        }

        Attached = 0;

        // TODO: FlushInstructionCache();

        return true;
    }

    Patch::Patch(void* gameAddress, const void* data, size_t size)
        : Address(gameAddress), OriginalData(nullptr), Size(size)
    {
        Data = new uint8_t[Size];

        memcpy(Data, data, Size);
    }

    Patch::~Patch()
    {
        Remove();

        if (OriginalData != nullptr)
        {
            delete[] OriginalData;

            OriginalData = nullptr;
        }
    }

    bool Patch::Apply()
    {
        // If it has original data, it is already applied
        if (OriginalData != nullptr)
            return false;

        DWORD flOldProtect;
        VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &flOldProtect);

        OriginalData = new uint8_t[Size];

        memcpy(OriginalData, Address, Size);

        memcpy(Address, Data, Size);

        VirtualProtect(Address, Size, flOldProtect, &flOldProtect);

        FlushInstructionCache(GetCurrentProcess(), Address, Size);

        return true;
    }

    bool Patch::Remove()
    {
        // If it has no original data, it is not yet applied
        if (OriginalData == nullptr)
            return false;

        DWORD flOldProtect;
        VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &flOldProtect);

        memcpy(Address, OriginalData, Size);

        delete[] OriginalData;

        OriginalData = nullptr;

        VirtualProtect(Address, Size, flOldProtect, &flOldProtect);

        FlushInstructionCache(GetCurrentProcess(), Address, Size);

        return true;
    }
#pragma endregion

#pragma region Memory Manager
    std::map<uint64_t, Hook*> MemoryManager::Hooks;
    std::vector<Patch*>       MemoryManager::Patches;

#ifdef NMF_USE_LOGGING
    Logger                    MemoryManager::Logger{ "MemoryManager" };
#endif

    void MemoryManager::Setup()
    {
#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "MemoryManager has been set up!");
#endif
    }

    void MemoryManager::Teardown()
    {
#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "Tearing down MemoryManager...");
#endif

        for (const auto& hook : Hooks)
        {
            hook.second->Remove();

            delete hook.second;
        }

        for (const auto& patch : Patches)
        {
            patch->Remove();

            delete patch;
        }

        Hooks.clear();
        Patches.clear();

#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "MemoryManager has been torn down!");
#endif
    }

    Hook* MemoryManager::CreateHook(void* gameAddress, void* hookFunc)
    {
        uint64_t hookId = reinterpret_cast<uint64_t>(gameAddress);

#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "Creating hook of 0x%lX, replacing with 0x%lX", hookId, hookFunc);
#endif

        auto itr = Hooks.find(hookId);
        if (itr != Hooks.end())
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Warning, "Hook of 0x%lX already exists! Skipping...", hookId);
#endif

            return nullptr;
        }

        auto hook = new Hook(gameAddress, hookFunc);

        Hooks[hookId] = hook;

        return hook;
    }

    Hook* MemoryManager::CreateAndApplyHook(void* gameAddress, void* hookFunc)
    {
        auto hook = MemoryManager::CreateHook(gameAddress, hookFunc);
        if (hook)
            hook->Apply();

        return hook;
    }

    Patch* MemoryManager::CreatePatch(void* gameAddress, const void* data, size_t size)
    {
        uint64_t patchAddr = reinterpret_cast<uint64_t>(gameAddress);

#ifdef NMF_USE_LOGGING
        Logger.Log(LogSeverity::Debug, "Creating patch of size %lu at 0x%lX", size, patchAddr);
#endif

        for (const auto& patch : Patches)
        {
            uint64_t currPatchAddr = reinterpret_cast<uint64_t>(patch->Address);

            if (patchAddr + size > currPatchAddr && patchAddr < currPatchAddr + patch->Size)
            {
#ifdef NMF_USE_LOGGING
                Logger.Log(LogSeverity::Warning, "Patch at 0x%lX size %lu overlaps with patch at 0x%lX size %lu! Skipping...", patchAddr, size, currPatchAddr, patch->Size);
#endif

                return nullptr;
            }
        }

        auto patch = new Patch(gameAddress, data, size);

        Patches.push_back(patch);

        return patch;
    }

    Patch* MemoryManager::CreatePatch(void* gameAddress, std::initializer_list<uint8_t> data)
    {
        return CreatePatch(gameAddress, data.begin(), data.size());
    }

    Patch* MemoryManager::CreateAndApplyPatch(void* gameAddress, const void* data, size_t size)
    {
        auto patch = MemoryManager::CreatePatch(gameAddress, data, size);
        if (patch)
            patch->Apply();

        return patch;
    }

    Patch* MemoryManager::CreateAndApplyPatch(void* gameAddress, std::initializer_list<uint8_t> data)
    {
        return CreateAndApplyPatch(gameAddress, data.begin(), data.size());
    }

    Patch* MemoryManager::NopMemory(void* gameAddress, size_t size)
    {
        auto nopArray = new unsigned char[size];

        for (size_t i = 0; i < size; ++i)
            nopArray[i] = 0x90;

        auto patch = MemoryManager::CreatePatch(gameAddress, nopArray, size);
        if (patch)
            patch->Apply();

        return patch;
    }
#pragma endregion
}

#undef TXT_
#undef TXT
#undef NMF_TEMP_MAIN_MODULE_NAME
