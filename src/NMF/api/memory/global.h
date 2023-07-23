#pragma once

#include <utility>

namespace NMF
{
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
}
