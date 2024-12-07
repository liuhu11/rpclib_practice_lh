#ifndef HANDLER_H_RPC_LOG
#define HANDLER_H_RPC_LOG

#include "log/formatter.h"
#include "log/record.h"
#include "log/level.h"
#include <concepts>
#include <string>

namespace logging {
// 不强制所有Handler都继承BaseHandler，只需要满足特定的接口，因此定义Concept
template <typename HandlerType>
concept Handler = requires(HandlerType handler, const Record& record, Level level) {
    // 模板参数相关的模板用template消除歧义
    // 要求有emit成员模板函数
    handler.template emit<Level::Debug>(record);
    // same_as是个concept
    {handler.format(record)} -> std::same_as<std::string>;
    // is_move_constructible是type_traits 有_v版本
    // move_constructible是concept
} && std::move_constructible<HandlerType> && !std::copy_constructible<HandlerType>;

// HandlerLevel是日志处理器的日志等级 -- 低于这个级别就不输出了
// 自己实现Handler时可以继承BaseHandler然后实现emit -- 提交到哪里
template <Level HandlerLevel = Level::Warning>
class BaseHandler {
public:
    BaseHandler(Formatter formatter):formatter_(formatter) {}
    BaseHandler(const BaseHandler&) = delete;
    BaseHandler& operator=(const BaseHandler&) = delete;

    // 移动构造的noexcept很重要 ！ 影响选择移动还是复制 -- 确保强异常安全
    BaseHandler(BaseHandler&& other) noexcept :formatter_(std::move(other.fomatter_)) {}

    // 被继承的类需要虚析构函数 避免析构时发生资源泄露
    virtual ~BaseHandler() {}

    // 获取
    Formatter formatter() const {
        return formatter_;
    }
    // 修改
    Formatter formatter(Formatter formatter) {
        formatter_ = std::move(formatter);
    }

    std::string format(const Record& record) {
        return formatter_(record);
    }

private:
    Formatter formatter_;
};
}


#endif