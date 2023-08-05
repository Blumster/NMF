#include "NMF.h"

#ifdef NMF_USE_LOGGING

#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <Windows.h>

namespace NMF
{
    Logger::Logger(std::initializer_list<const char*> l)
        : Scopes(l)
    {
    }

    void Logger::AddScope(const char* scope)
    {
        Scopes.push_back(scope);
    }

    void Logger::RemoveLastScope()
    {
        Scopes.pop_back();
    }

    void Logger::Log(LogSeverity severity, const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);

        char buffer[1025] = { 0 };
        vsnprintf_s(buffer, 1025, fmt, args);

        va_end(args);

        std::stringstream ss;

        for (auto scope : Scopes)
            ss << "[" << scope << "]";

        LogManager::Log(severity, "%s: %s", ss.str().c_str(), buffer);
    }

    std::ofstream* LogManager::OutFileStream = nullptr;
    LogDestination LogManager::Destination = LogDestination::None;
    LogFilter      LogManager::SeverityFilter = LogFilter::None;
    Logger         LogManager::LogManagerLogger{ "LogManager" };

    const char* SeverityToString(LogSeverity severity)
    {
        switch (severity)
        {
        case LogSeverity::Info:
            return "Info";

        case LogSeverity::Warning:
            return "Warning";

        case LogSeverity::Debug:
            return "Debug";

        case LogSeverity::VerboseDebug:
            return "VerboseDebug";

        case LogSeverity::Error:
            return "Error";
        }

        return "Unknown";
    }

    void LogManager::Setup(LogDestination destination, LogFilter filter)
    {
        Destination = destination;
        SeverityFilter = filter;

        if ((Destination & LogDestination::File) == LogDestination::File && OutFileStream == nullptr)
        {
            auto path = std::filesystem::current_path() / "logs";

            std::filesystem::create_directory(path);

            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            std::tm tm;
            localtime_s(&tm, &time);

            std::stringstream fileName;
            fileName << std::put_time(&tm, "%Y_%m_%d_%H_%M_%S_log.txt");

            path /= fileName.str();

            OutFileStream = new std::ofstream(path);
        }

        if ((Destination & LogDestination::Console) == LogDestination::Console)
        {
            AllocConsole();

            FILE* file = nullptr;

            freopen_s(&file, "CONIN$", "r", stdin);
            freopen_s(&file, "CONOUT$", "w", stdout);
        }

        LogManagerLogger.Log(LogSeverity::Debug, "LogManager has been set up!");
        LogManagerLogger.Log(LogSeverity::Debug, "Logging to File: %s", ((Destination & LogDestination::File) == LogDestination::File) ? "true" : "false");
        LogManagerLogger.Log(LogSeverity::Debug, "Logging to Console: %s", ((Destination & LogDestination::Console) == LogDestination::Console) ? "true" : "false");
        LogManagerLogger.Log(LogSeverity::Debug, "Logging to GUIConsole: %s", ((Destination & LogDestination::GUIConsole) == LogDestination::GUIConsole) ? "true" : "false");
    }

    void LogManager::Teardown()
    {
        LogManagerLogger.Log(LogSeverity::Debug, "Tearing down LogManager...");

        if (OutFileStream != nullptr)
        {
            OutFileStream->flush();
            OutFileStream->close();

            delete OutFileStream;

            OutFileStream = nullptr;
        }

        if ((Destination & LogDestination::Console) == LogDestination::Console)
        {
            FreeConsole();
        }
    }

    void LogManager::Log(LogSeverity severity, const char* fmt, ...)
    {
        LogFilter filter = 1 << severity;

        if ((SeverityFilter & filter) != filter)
            return;

        va_list args;
        va_start(args, fmt);

        char buffer[1025];
        vsnprintf_s(buffer, 1025, fmt, args);

        va_end(args);

        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        tm tm;
        localtime_s(&tm, &time);

        auto timeStr = std::put_time(&tm, "%F %T");

        if ((Destination & LogDestination::File) == LogDestination::File && OutFileStream != nullptr)
        {
            *OutFileStream << "[" << timeStr << "][" << SeverityToString(severity) << "]";

            if (buffer[0] != '[')
            {
                *OutFileStream << ": ";
            }

            *OutFileStream << buffer << std::endl;
        }

        if ((Destination & LogDestination::Console) == LogDestination::Console)
        {
            std::cout << "[" << timeStr << "][" << SeverityToString(severity) << "]";

            if (buffer[0] != '[')
            {
                std::cout << ": ";
            }

            std::cout << buffer << std::endl;
        }

        if ((Destination & LogDestination::GUIConsole) == LogDestination::GUIConsole)
        {
            // TODO
        }
    }
}

#endif
