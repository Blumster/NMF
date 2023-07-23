#include <cstring>
#include <stdint.h>
#include <utility>
#include <WinSock2.h>
#include <Windows.h>

#include "patch.h"

namespace NMF
{
    Patch::Patch(void* gameAddress, void* data, size_t size)
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
}
