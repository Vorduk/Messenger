#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& log_file_name) {
    try {  
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();    // Create console sink with colors.  
        console_sink->set_level(spdlog::level::trace);  // Level of output, above selected.

        // Colors for different levels.
        console_sink->set_color(spdlog::level::info, FOREGROUND_GREEN);                             // Green.
        console_sink->set_color(spdlog::level::warn, FOREGROUND_RED | FOREGROUND_GREEN);            // Yellow.
        console_sink->set_color(spdlog::level::err, FOREGROUND_RED);                                // Red.
        console_sink->set_color(spdlog::level::critical, FOREGROUND_RED | FOREGROUND_INTENSITY);    // Bright red.

        std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name, true);    // Create file sink.
        file_sink->set_level(spdlog::level::trace);

        m_logger = std::make_shared<spdlog::logger>("messenger", spdlog::sinks_init_list{ console_sink, file_sink });   // Create logger with both sinks.
        m_logger->set_level(spdlog::level::trace);

        // Default pattern (if prefixes wasn't set).
        //m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        m_logger->set_pattern("[%^%l%$] %v");

        // Register logger as default.
        spdlog::set_default_logger(m_logger);

        Logger::getInstance().setShowFileInfo(false);
        info(__FILE__, __LINE__, __FUNCTION__, "Logger initialized successfully");
        Logger::getInstance().setShowFileInfo(true);

    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    }
}

void Logger::setMinimalLevel(Level level) {
    if (!m_logger) { 
        return; 
    }

    spdlog::level::level_enum spd_level;
    switch (level) {
    case Level::Trace:    spd_level = spdlog::level::trace; break;
    case Level::Debug:    spd_level = spdlog::level::debug; break;
    case Level::Info:     spd_level = spdlog::level::info; break;
    case Level::Warn:     spd_level = spdlog::level::warn; break;
    case Level::Error:    spd_level = spdlog::level::err; break;
    case Level::Critical: spd_level = spdlog::level::critical; break;
    default:              spd_level = spdlog::level::info; break;
    }

    m_logger->set_level(spd_level);
}

void Logger::setPattern(const std::string& pattern) {
    if (m_logger) {
        m_logger->set_pattern(pattern);
    }
}

void Logger::setShowFileInfo(bool show) {
    m_show_file_info = show;
}

void Logger::setShowLevelPrefix(bool show) {
    m_show_level_prefix = show;
}

