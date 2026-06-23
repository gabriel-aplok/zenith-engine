#pragma once
#include <string>
#include <source_location>
#include <chrono>
#include <format>
#include <iostream>

namespace Zenith {

    enum class LogLevel {
        Info,
        Debug,
        Warning,
        Error,
        Fatal
    };

    class Log {
    public:
        static void Init();

        static void Print(LogLevel level,
            const std::string& message,
            const std::source_location& location = std::source_location::current());

        // Formatted logging
        template <typename... Args>
        static void Info(const std::string& fmt, Args&&... args) {
            Print(LogLevel::Info, std::vformat(fmt, std::make_format_args(args...)));
        }

        template <typename... Args>
        static void Debug(const std::string& fmt, Args&&... args) {
            Print(LogLevel::Debug, std::vformat(fmt, std::make_format_args(args...)));
        }

        template <typename... Args>
        static void Warn(const std::string& fmt, Args&&... args) {
            Print(LogLevel::Warning, std::vformat(fmt, std::make_format_args(args...)));
        }

        template <typename... Args>
        static void Error(const std::string& fmt, Args&&... args) {
            Print(LogLevel::Error, std::vformat(fmt, std::make_format_args(args...)));
        }

        template <typename... Args>
        static void Fatal(const std::string& fmt, Args&&... args) {
            Print(LogLevel::Fatal, std::vformat(fmt, std::make_format_args(args...)));
        }
    };

} // namespace Zenith