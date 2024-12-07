#ifndef LOGGER_H_RPC_LIB
#define LOGGER_H_RPC_LIB

#include "log/level.h"
#include "log/handler.h"
#include "log/handlers/default_handler.h"

#include <tuple>
#include <string>
#include <cstdint>
#include <chrono>
#include <source_location>

#include <format>

namespace logging {

template <Level LoggerLevel, Handler... HandlerTypes>
    // requires也得带上
    requires (sizeof...(HandlerTypes) > 0)
class Logger;

// using起类型别名
using DefaultLogger = Logger<Level::Warning, handlers::DefaultHandler<Level::Warning>>;

template <Level LoggerLevel, Handler... HandlerTypes>
    requires (sizeof...(HandlerTypes) > 0)
class Logger {
public:
    static constexpr int32_t handler_count = sizeof...(HandlerTypes);
    static constexpr Level level = LoggerLevel;

    Logger(const std::string& name, std::tuple<HandlerTypes...>&& handlers):name_(name), handlers_(std::move(handlers)) {}

    // 移动赋值运算符和移动构造函数是分开的
    Logger(Logger&& other) noexcept :name_(std::move(other.name_)), handlers_(std::move(other.handlers_)) {}
    Logger& operator(Logger&& other) noexcept {
        name_ = std::move(other.name_);
        handlers_ = std::move(other.handlers_);
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template <Level LogLevel>
        requires (LogLevel > LoggerLevel)
    Logger& log(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        // 返回当前对象
        return *this;
    }

    template <Level LogLevel>
        requires (LogLevel <= LoggerLevel)
    Logger& log(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        // 指派符必须与声明的顺序相同 -- 聚合初始化的一种
        Record record {
            .name = name_,
            .level = LogLevel,
            .time = std::chrono::system_clock::now(),
            .msg = msg,
            .source_location = source_location
        };

        handle_log<LogLevel, handler_count - 1>(record);
        return *this;
    }

    template <Level LogLevel, int32_t HandlerIdx>
        requires (HandlerIdx > 0)
    void handle_log(const Record& record) {
        // 递归调用handleLog将消息同时提交给前一个日志处理器
        // 最后还是0号handler最先处理
        handle_log<LogLevel, HandlerIdx - 1>(record);

        auto& handler = std::get<HandlerIdx>(handlers_);
        handler.emit<LogLevel>(record);
    }

    // 模板函数没有偏特化
    template <Level LogLevel, int32_t HandlerIdx>
        requires (HandlerIdx == 0)
    void handle_log(const Record& record) {
        auto& handler = std::get<HandlerIdx>(handlers_);
        handler.emit<LogLevel>(record);
    }

    // 以下都是对log的包装
    Logger& critical(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        return log<Level::Critical>(msg, source_location);
    }

    Logger& error(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        return log<Level::Error>(msg, source_location);
    }

    Logger& warning(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        return log<Level::Warning>(msg, source_location);
    }

    Logger& info(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        return log<Level::Info>(msg, source_location);
    }

    Logger& debug(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        return log<Level::Debug>(msg, source_location);
    }

    Logger& trace(const std::string& msg, std::source_location source_location = std::source_location::current()) {
        return log<Level::Trace>(msg, source_location);
    }
private:
    std::string name_;
    std::tuple<HandlerTypes...> handlers_;
};

// 日志记录器生成工厂
template <Level LoggerLevel = Level::Warning>
class LoggerFactory {
public:
    template <Handler... HandlerTypes>
    static Logger<LoggerLevel, HandlerTypes...> create_logger(const std::string& name, std::tuple<HandlerTypes...>&& handlers) {
        return Logger<LoggerLevel, HandlerTypes...>(name, std::forward<std::tuple<HandlerTypes...>>(handlers));
    }

    // 默认handler级别与logger一致
    // 参数必定不一样，是重载
    static Logger<LoggerLevel, handlers::DefaultHandler<LoggerLevel>> create_logger(const std::string& name) {
        return Logger<LoggerLevel, handlers::DefaultHandler<LoggerLevel>>(name, std::make_tuple(handlers::DefaultHandler()));
    }
};
}


#endif