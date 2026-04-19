#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>

/**
 * @class Logger
 * @brief Thread-safe singleton wrapper around spdlog for application-wide logging.
 *
 * Supports colored console output (green for info, yellow for warnings, red for errors),
 * simultaneous file logging, and configurable prefixes (file/line/function and level tags).
 */
class Logger {
public:

    /// Log severity levels matching spdlog's levels.
    enum class Level {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };
    
    /**
     * @brief Returns the singleton instance of the logger.
     * @return Reference to the Logger instance.
     */
    static Logger& getInstance();

    /**
     * @brief Initializes the logger with console and file sinks.
     * @param logFileName Name of the log file (default: "messenger.log").
     */
    void initialize(const std::string& log_file_name = "messenger.log");

    /**
     * @brief Sets the minimum severity level to be logged.
     * @param level Minimum level (messages below this level are ignored).
     */
    void setMinimalLevel(Level level);

    /**
     * @brief Sets a custom format pattern for log messages.
     * @param pattern spdlog pattern string (e.g., "[%Y-%m-%d %H:%M:%S.%e] [%l] %v").
     */
    void setPattern(const std::string& pattern);

    void setShowFileInfo(bool show);
    void setShowLevelPrefix(bool show);

    // Logging methods with automatic source location capture (via macros).
    template<typename... Args>
    void trace(const char* file, int line, const char* func, fmt::format_string<Args...> fmt, Args&&... args) {
        log(spdlog::level::trace, file, line, func, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(const char* file, int line, const char* func, fmt::format_string<Args...> fmt, Args&&... args) {
        log(spdlog::level::debug, file, line, func, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const char* file, int line, const char* func, fmt::format_string<Args...> fmt, Args&&... args) {
        log(spdlog::level::info, file, line, func, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const char* file, int line, const char* func, fmt::format_string<Args...> fmt, Args&&... args) {
        log(spdlog::level::warn, file, line, func, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const char* file, int line, const char* func, fmt::format_string<Args...> fmt, Args&&... args) {
        log(spdlog::level::err, file, line, func, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(const char* file, int line, const char* func, fmt::format_string<Args...> fmt, Args&&... args) {
        log(spdlog::level::critical, file, line, func, fmt, std::forward<Args>(args)...);
    }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template<typename... Args>
    void log(spdlog::level::level_enum lvl, const char* file, int line, const char* func,
        fmt::format_string<Args...> fmt, Args&&... args) {
        if (!m_logger) return;

        std::string message = fmt::format(fmt, std::forward<Args>(args)...);

        if (m_show_file_info && m_show_level_prefix) {
            // Format: [INFO] file.cpp:123 function() Message.
            m_logger->log(lvl, "[{}:{} {}] {}",
                extractFileName(file), line, func, message);
        }
        else if (m_show_file_info) {
            // Only file info.
            m_logger->log(lvl, "[{}:{} {}] {}",
                extractFileName(file), line, func, message);
        }
        else if (m_show_level_prefix) {
            // Only level prefix.
            m_logger->log(lvl, message);
        }
        else {
            // Without prefix.
            m_logger->log(lvl, message);
        }
    }

    /// Extracts the base filename from a full path.
    static const char* extractFileName(const char* path) {
        const char* lastSlash = strrchr(path, '/');
        const char* lastBackslash = strrchr(path, '\\');
        const char* filename = path;

        if (lastSlash) filename = lastSlash + 1;
        if (lastBackslash && lastBackslash > filename) filename = lastBackslash + 1;

        return filename;
    }

    std::shared_ptr<spdlog::logger> m_logger;
    bool m_show_file_info = true;
    bool m_show_level_prefix = true;
};