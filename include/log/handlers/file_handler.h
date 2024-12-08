#ifndef FILE_HANDLER_H_RPC_LOG
#define FILE_HANDLER_H_RPC_LOG

#include "log/handler.h"

#include <string>
#include <fstream>
#include <filesystem>

namespace logging::handlers {
template <Level HandlerLevel = Level::Warning>
class FileHandler : public BaseHandler<HandlerLevel> {
public:
    static void ensure_direcory(const std::string& file_path) {
        namespace fs = std::filesystem;

        auto parent_directory = fs::absolute(file_path).parent_path();
        fs::create_directories(parent_directory);
    }

    static FileHandler create(const std::string& file_path, Formatter formatter = logging::formatters::format_record) {
        ensure_direcory(file_path);
        return FileHandler(file_path, formatter);
    }

    static FileHandler create(const std::string& file_path, std::ios_base::openmode mode, Formatter formatter = logging::formatters::format_record) {
        ensure_direcory(file_path);
        return FileHandler(file_path, mode, formatter);
    }

    // 最好显示调用父类的构造函数
    // 否则会隐式调用父类的默认构造函数
    FileHandler(const std::string& file_path, Formatter formatter = logging::formatters::format_record):BaseHandler<HandlerLevel>(formatter), 
        stream_(file_path), syncstream_(stream_) {}
    FileHandler(const std::string& file_path, std::ios_base::openmode mode, Formatter formatter = logging::formatters::format_record)
        :BaseHandler<HandlerLevel>(formatter), stream_(file_path, mode), syncstream_(stream_) {}

    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    // ofstream支持移动构造
    FileHandler(FileHandler&& other) noexcept :BaseHandler<HandlerLevel>(other.formatter()), stream_(std::move(other.stream_)), 
        syncstream_(std::move(other.syncstream_)) {}
    FileHandler& operator=(FileHandler&& other) noexcept {
        (void)BaseHandler<HandlerLevel>::operator=(std::move(other));
        stream_ = std::move(other.stream_);
        syncstream_ = std::move(syncstream_);
        return *this;
    }

    template <Level EmitLevel>
        requires (EmitLevel > HandlerLevel)
    void emit(const Record&) {
    }

    template <Level EmitLevel>
        requires (EmitLevel <= HandlerLevel) 
    void emit(const Record& record) {
        syncstream_ << this->format(record) << std::endl;
        (void)syncstream_.emit();
    }
private:
    std::ofstream stream_;
    std::osyncstream syncstream_;
};
}



#endif