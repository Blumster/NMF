#include <windows.h>
#include <Psapi.h>

#include "addressing.h"
#include "../../NMF.h"

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

#if defined(NMF_MAIN_MODULE_NAME)
#define NMF_TEMP_MAIN_MODULE_NAME TXT(NMF_MAIN_MODULE_NAME)
#else
#define NMF_TEMP_MAIN_MODULE_NAME NULL
#endif

void* ImageBase = nullptr;
uint64_t ImageSize = 0x0ull;

namespace NMF
{
    void* GetImageBase()
    {
        if (ImageBase == nullptr || ImageSize == 0x0ull)
        {
            MODULEINFO moduleInfo = { 0 };

            HMODULE hModule = GetModuleHandleW(NMF_TEMP_MAIN_MODULE_NAME);
            if (hModule == 0)
            {
                if (!NMF::HostIsGame())
                {
                    // Fake these values to be able to create the addresses, but those will be invalid and if they are called, the app will crash
                    ImageBase = nullptr;
                    ImageSize = 0xFFFFFFFFFFFFFFFFull;

                    return 0x0ull;
                }

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
}

#undef TXT_
#undef TXT
#undef NMF_TEMP_MAIN_MODULE_NAME
