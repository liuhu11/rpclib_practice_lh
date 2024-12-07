#ifndef DEFAULT_HANDLER_H_RPC_LOG
#define DEFAULT_HANDLER_H_RPC_LOG

#include "log/handler.h"

#include <iostream>
#include <syncstream>

namespace logging::handlers {
template <Level HandlerLevel = Level::Warning>
class DefaultHandler : public BaseHandler<HandlerLevel> {
public:
    DefaultHandler(Formatter formatter = logging::formatters::format_record):BaseHandler<HandlerLevel>(formatter) {}
    // 基类删除了拷贝构造 子类其实也不会自动生成
    DefaultHandler(const DefaultHandler&) = delete;
    DefaultHandler& operator=(const DefaultHandler&) = delete;

    // 三/五原则 定义了移动构造并不会自动生成移动赋值运算符
    DefaultHandler(DefaultHandler&& other) noexcept :BaseHandler<HandlerLevel>(other.formatter()) {}

    template <Level EmitLevel>
        requires (EmitLevel > HandlerLevel)
    void emit(const Record&) {
    }

    template <Level EmitLevel>
        requires (EmitLevel <= HandlerLevel)
    void emit(const Record& record) {
        // 父类继承来的
        std::osyncstream(std::cout) << format(record) << std::endl;
    }
};
}



#endif