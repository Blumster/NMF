#include <utility>
#include <Windows.h>

#include <detours/detours.h>

#include "hook.h"

namespace NMF
{
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
}
