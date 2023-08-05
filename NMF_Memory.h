#pragma once

#include <map>
#include <stdint.h>
#include <vector>

#ifndef NMF_VERSION
#include "NMF.h"
#endif

namespace NMF
{
    class MemoryManager;

    NMF_EXTENDED_EXPORT void* operator"" _addr(uint64_t address);
    NMF_EXTENDED_EXPORT void* operator"" _rva(uint64_t address);

#pragma region Function Wrappers
    template<typename RetType, typename... Args>
    class StdGameFunction
    {
    public:
        using FuncType = RetType(__stdcall*)(Args...);

        NMF_DELETE_ALL_INIT(StdGameFunction);

        StdGameFunction(void* gameAddress)
            : Address(static_cast<FuncType>(gameAddress))
        {
        }

        FuncType get() const
        {
            return Address;
        }

        RetType operator()(Args... args) const
        {
            return Address(args...);
        }

    private:
        FuncType Address;
    };

    template<typename RetType, typename... Args>
    class ThisGameFunction 
    {
    public:
        using FuncType = RetType(__thiscall*)(Args...);

        NMF_DELETE_ALL_INIT(ThisGameFunction);

        ThisGameFunction(void* gameAddress)
            : Address(static_cast<FuncType>(gameAddress))
        {
        }

        FuncType get() const
        {
            return Address;
        }

        RetType operator()(Args... args) const
        {
            return Address(args...);
        }

    private:
        FuncType Address;
    };

    template<typename RetType, typename... Args>
    class CdeclGameFunction
    {
    public:
        using FuncType = RetType(__cdecl*)(Args...);

        NMF_DELETE_ALL_INIT(CdeclGameFunction);

        CdeclGameFunction(void* gameAddress)
            : Address(static_cast<FuncType>(gameAddress))
        {
        }

        FuncType get() const
        {
            return Address;
        }

        RetType operator()(Args... args) const
        {
            return Address(args...);
        }

    private:
        FuncType Address;
    };

    template<typename RetType, typename... Args>
    class FastGameFunction
    {
    public:
        using FuncType = RetType(__fastcall*)(Args...);

        NMF_DELETE_ALL_INIT(FastGameFunction);

        FastGameFunction(void* gameAddress)
            : Address(static_cast<FuncType>(gameAddress))
        {
        }

        FuncType get() const
        {
            return Address;
        }

        RetType operator()(Args... args) const
        {
            return Address(args...);
        }

    private:
        FuncType Address;
    };
#pragma endregion

#pragma region Global Data Wrappers
    template <typename T>
    class GlobalPtr
    {
        T** Address;

    public:
        inline GlobalPtr(void* gameAddress)
            : Address(static_cast<T**>(gameAddress))
        {
        }

        inline T* operator->() const
        {
            return *Address;
        }

        inline T* operator*() const
        {
            return *Address;
        }

        inline T** GetGlobalPtr() const
        {
            return Address;
        }

        inline T* get() const
        {
            return *Address;
        }

        inline operator T* () const
        {
            return *Address;
        }

        inline operator bool() const
        {
            return *Address != nullptr;
        }
    };

    template <typename T, int Size>
    class GlobalPtrArray
    {
        T** Address;

    public:
        inline GlobalPtrArray(void* gameAddress)
            : Address(static_cast<T**>(gameAddress))
        {
        }

        inline T** GetGlobalPtr() const
        {
            return Address;
        }

        inline T*& operator[](int index) const
        {
            return *(Address + index);
        }
    };

    template <typename T>
    class GlobalValue
    {
        T* Address;

    public:
        inline GlobalValue(void* gameAddress)
            : Address(static_cast<T*>(gameAddress))
        {
        }

        inline T* GetGlobalPtr() const
        {
            return Address;
        }

        inline T& get() const
        {
            return *Address;
        }

        inline T* operator->() const
        {
            return Address;
        }

        inline T& operator*() const
        {
            return *Address;
        }

        inline operator T& () const
        {
            return *Address;
        }
    };

    template <typename T, int Size>
    class GlobalValueArray
    {
        T* Address;

    public:
        inline GlobalValueArray(void* gameAddress)
            : Address(static_cast<T*>(gameAddress))
        {
        }

        inline T* GetGlobalPtr() const
        {
            return Address;
        }

        inline T& operator[](int index) const
        {
            return *(Address + index);
        }
    };
#pragma endregion

#pragma region Hook & Patch
    class NMF_EXTENDED_EXPORT Hook
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

    class NMF_EXTENDED_EXPORT Patch
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
#pragma endregion

#pragma region MemoryManager
    class NMF_EXPORT MemoryManager final
    {
    public:
        static void Setup();
        static void Teardown();

        static Hook* CreateHook(void* gameAddress, void* hookFunc);
        static Hook* CreateAndApplyHook(void* gameAddress, void* hookFunc);
        static Patch* CreatePatch(void* gameAddress, void* data, size_t size);
        static Patch* CreateAndApplyPatch(void* gameAddress, void* data, size_t size);
        static Patch* NopMemory(void* gameAddress, size_t size);

    private:
#ifdef NMF_USE_LOGGING
        static Logger Logger;
#endif

#pragma warning(disable: 4251)
        static std::map<uint32_t, Hook*> Hooks;
        static std::vector<Patch*> Patches;
#pragma warning(default: 4251)
    };
#pragma endregion
}
