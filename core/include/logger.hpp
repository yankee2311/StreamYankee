#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <vector>

namespace obs {

class Logger {
public:
    static Logger& instance();
    
    void initialize(const std::string& logPath = "", bool console = true);
    
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
    void trace(const std::string& message);
    
    void setLevel(spdlog::level::level_enum level);
    
    void flush();
    
    std::shared_ptr<spdlog::logger> getLogger() { return logger_; }
    
private:
    Logger() = default;
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;
    std::vector<spdlog::sink_ptr> sinks_;
    bool initialized_ = false;
};

#define LOG_INFO(msg) obs::Logger::instance().info(msg)
#define LOG_WARN(msg) obs::Logger::instance().warn(msg)
#define LOG_ERROR(msg) obs::Logger::instance().error(msg)
#define LOG_DEBUG(msg) obs::Logger::instance().debug(msg)
#define LOG_TRACE(msg) obs::Logger::instance().trace(msg)

} // namespace obs