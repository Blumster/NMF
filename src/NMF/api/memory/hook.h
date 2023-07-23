#pragma once

#include <stdint.h>

#include "../../export.h"

namespace NMF
{
    class MemoryManager;

    class NMF_EXPORT Hook
    {
        friend class MemoryManager;

        Hook(void* gameAddress, void* hookFunc);
        ~Hook();

    public:
        Hook(Hook&&) = default;
        Hook& operator=(Hook&&) = default;

        Hook(const Hook&) = delete;
        Hook& operator=(const Hook&) = delete;

        bool Apply();
        bool Remove();

        template <typename F>
        F GetOriginalFunction() const { return reinterpret_cast<F>(OriginalFunc); }

    private:
        void* Address;
        void* OriginalFunc;
        void* HookFunc;
        uint8_t Attached;
    };
}
