#include "logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace obs {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& logPath, bool console) {
    if (initialized_) {
        return;
    }
    
    try {
        sinks_.clear();
        
        if (console) {
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            consoleSink->set_level(spdlog::level::trace);
            consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            sinks_.push_back(consoleSink);
        }
        
        if (!logPath.empty()) {
            auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath, 1024 * 1024 * 5, 3);
            fileSink->set_level(spdlog::level::trace);
            fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
            sinks_.push_back(fileSink);
        }
        
        logger_ = std::make_shared<spdlog::logger>("obs", sinks_.begin(), sinks_.end());
        logger_->set_level(spdlog::level::debug);
        logger_->flush_on(spdlog::level::warn);
        
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
        
        initialized_ = true;
        
        LOG_INFO("Logger initialized successfully");
    } catch (const std::exception& e) {
        fprintf(stderr, "Failed to initialize logger: %s\n", e.what());
    }
}

void Logger::info(const std::string& message) {
    if (logger_) {
        logger_->info(message);
    }
}

void Logger::warn(const std::string& message) {
    if (logger_) {
        logger_->warn(message);
    }
}

void Logger::error(const std::string& message) {
    if (logger_) {
        logger_->error(message);
    }
}

void Logger::debug(const std::string& message) {
    if (logger_) {
        logger_->debug(message);
    }
}

void Logger::trace(const std::string& message) {
    if (logger_) {
        logger_->trace(message);
    }
}

void Logger::setLevel(spdlog::level::level_enum level) {
    if (logger_) {
        logger_->set_level(level);
    }
}

void Logger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

} // namespace obs