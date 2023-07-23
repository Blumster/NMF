#pragma once

#include "../../export.h"

namespace NMF
{
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
}
