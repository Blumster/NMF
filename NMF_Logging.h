#pragma once

#ifdef NMF_USE_LOGGING

#include <iostream>
#include <stdint.h>
#include <vector>

#ifndef NMF_VERSION
#include "NMF.h"
#endif

namespace NMF
{
    enum class LogDestination : uint8_t
    {
        None       = 0,
        File       = 1,
        Console    = 2,
        GUIConsole = 4
    };

    enum class LogSeverity : uint32_t
    {
        Info,
        Warning,
        Debug,
        VerboseDebug,
        Error
    };

    enum class LogFilter : uint32_t
    {
        None         = 0,
        Info         = (1 << static_cast<int>(LogSeverity::Info)),
        Warning      = (1 << static_cast<int>(LogSeverity::Warning)),
        Debug        = (1 << static_cast<int>(LogSeverity::Debug)),
        VerboseDebug = (1 << static_cast<int>(LogSeverity::VerboseDebug)),
        Error        = (1 << static_cast<int>(LogSeverity::Error)),
        All          = Info | Warning | Debug | Error,
        AllVerbose   = All | VerboseDebug
    };

    constexpr LogDestination operator|(LogDestination lhs, LogDestination rhs)
    {
        return static_cast<LogDestination>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    constexpr LogDestination operator&(LogDestination lhs, LogDestination rhs)
    {
        return static_cast<LogDestination>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    constexpr LogFilter operator<<(int lhs, LogSeverity rhs)
    {
        return static_cast<LogFilter>(lhs << static_cast<int>(rhs));
    }

    constexpr LogFilter operator|(LogFilter lhs, LogFilter rhs)
    {
        return static_cast<LogFilter>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    constexpr LogFilter operator&(LogFilter lhs, LogFilter rhs)
    {
        return static_cast<LogFilter>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    class NMF_EXTENDED_EXPORT Logger final
    {
    public:
        Logger(std::initializer_list<const char*> l);

        void AddScope(const char* scope);
        void RemoveLastScope();
        void Log(LogSeverity severity, const char* fmt, ...) const;

    private:
#pragma warning(disable: 4251)
        std::vector<const char*> Scopes;
#pragma warning(default: 4251)
    };

    class NMF_EXPORT LogManager final
    {
        static LogDestination Destination;
        static LogFilter SeverityFilter;
        static std::ofstream* OutFileStream;
        static Logger LogManagerLogger;

    public:
        static void Setup(LogDestination destination, LogFilter filter);
        static void Teardown();
        static void Log(LogSeverity severity, const char* fmt, ...);
    };
}

#endif
