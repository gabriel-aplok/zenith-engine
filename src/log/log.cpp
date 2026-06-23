#include <log/log.hpp>
#include <iomanip>
#include <cstdlib>

namespace Zenith {

    void Log::Init() {
        // Future: file logging, async, etc.
    }

    void Log::Print(LogLevel level, const std::string& message, const std::source_location& location) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);

        const char* levelStr = "";
        const char* color = "";

        switch (level) {
        case LogLevel::Info:     levelStr = "INFO";     color = "\033[32m"; break;
        case LogLevel::Debug:    levelStr = "DEBUG";    color = "\033[34m"; break;
        case LogLevel::Warning:  levelStr = "WARNING";  color = "\033[33m"; break;
        case LogLevel::Error:    levelStr = "ERROR";    color = "\033[31m"; break;
        case LogLevel::Fatal:    levelStr = "FATAL";    color = "\033[31;1m"; break;
        }

        std::cout << '['
            << std::setfill('0') << std::setw(2) << tm.tm_hour << ':'
            << std::setw(2) << tm.tm_min << ':'
            << std::setw(2) << tm.tm_sec << "] "
            << color << '[' << levelStr << "] "
            << location.file_name() << ':' << location.line() << ':' << location.column()
            << " " << message << "\033[0m\n";

        if (level == LogLevel::Fatal) {
            std::exit(1);
        }
    }

} // namespace Zenith