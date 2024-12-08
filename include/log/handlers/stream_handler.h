#ifndef STREAM_HANDLER_H_RPC_LOG
#define STREAM_HANDLER_H_RPC_LOG

#include "log/handler.h"

#include <iostream>
#include <memory>
#include <type_traits>
#include <syncstream>

// ostream的移动构造是protected的
namespace logging::handlers {
template <Level HandlerLevel>
class StreamHandler : public BaseHandler<HandlerLevel> {
public:
    StreamHandler(std::ostream& os = std::cout, Formatter formatter = logging::formatters::format_record): BaseHandler<HandlerLevel>(formatter), stream_(os) {}

    // 基类的移动构造函数被声明为protected 子类仍可以有public的移动构造函数 -- 子类调用基类移动构造函数即可
    template <typename OutputStreamType>
        requires std::is_base_of_v<std::ostream, OutputStreamType>
    // 注意 模板里是万能引用 别搞混了
    StreamHandler(OutputStreamType&& owned_os, Formatter formatter = logging::formatters::format_record): BaseHandler<HandlerLevel>(formatter), 
        owned_stream_(std::make_unique<std::ostream>(std::forward<OutputStreamType>(owned_os))), stream_(*owned_stream_) {}
    StreamHandler(Formatter formatter): BaseHandler<HandlerLevel>(formatter), stream_(std::cout) {}

    StreamHandler(const StreamHandler&) = delete;
    StreamHandler& operator=(const StreamHandler&) = delete;

    // 移动构造能声明为noexcept就声明为noexcept
    // 在模板类中，模板类本身不需要带参数
    StreamHandler(StreamHandler&& other) noexcept : BaseHandler<HandlerLevel>(other.formatter()), owned_stream_(std::move(other.owned_stream_)), 
        stream(*owned_stream_) {}
    StreamHandler& operator=(StreamHandler&& other) noexcept {
        (void)BaseHandler<HandlerLevel>::operator=(std::move(other));
        owned_stream_ = std::move(other.owned_stream_);
        stream_ = stream_;
        return *this;
    }
    
    // 子类的析构函数不用显式调用基类的析构函数
    ~StreamHandler() override {}

    template <Level EmitLevel>
        requires (EmitLevel > HandlerLevel)
    void emit(const Record&) {
    }

    template <Level EmitLevel>
        requires (EmitLevel <= HandlerLevel)
    void emit(const Record& record) {
        std::osyncstream(stream_) << this->format(record) << std::endl;
    }
private:
    std::unique_ptr<std::ostream> owned_stream_;
    std::ostream& stream_;
};
}

#endif