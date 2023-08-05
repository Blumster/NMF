#pragma once

#include <typeinfo>
#include <rttidata.h>

#ifndef NMF_VERSION
#include "NMF.h"
#endif

namespace NMF
{
    class RTTI
    {
    public:
        static const std::type_info* TypeId(void* inptr)
        {
            return reinterpret_cast<const std::type_info*>(__RTtypeid(inptr));
        }

        template<typename TO, typename FROM>
        static TO DynamicCast(FROM inptr)
        {
            static_assert(std::is_pointer_v<FROM>, "Only pointers are supported!");
            static_assert(std::is_pointer_v<TO>, "Only pointers are supported!");

            return static_cast<TO>(__RTDynamicCast(inptr, 0, std::remove_pointer_t<FROM>::TypeDescriptor, std::remove_pointer_t<TO>::TypeDescriptor, 0));
        }
    };

    /*inline const std::type_info* RTTI::TypeId(void* inptr)
    {
        return reinterpret_cast<const std::type_info*>(__RTtypeid(inptr));
    }

    template<typename TO, typename FROM>
    inline TO RTTI::DynamicCast(FROM inptr)
    {
        static_assert(std::is_pointer_v<FROM>, "Only pointers are supported!");
        static_assert(std::is_pointer_v<TO>, "Only pointers are supported!");

        return static_cast<TO>(__RTDynamicCast(inptr, 0, std::remove_pointer_t<FROM>::TypeDescriptor, std::remove_pointer_t<TO>::TypeDescriptor, 0));
    }*/
}
