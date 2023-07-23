#pragma once

#include "../../export.h"

namespace NMF
{
    class MemoryManager;

    class NMF_EXPORT Patch
    {
        friend class MemoryManager;

        Patch(void* gameAddress, void* data, size_t size);
        ~Patch();

    public:
        Patch(Patch&&) = default;
        Patch& operator=(Patch&&) = default;

        Patch(const Patch&) = delete;
        Patch& operator=(const Patch&) = delete;

        bool Apply();
        bool Remove();

    private:
        void* Address;
        void* Data;
        void* OriginalData;
        size_t Size;
    };
}
