#ifndef FORMATTER_H_RPC_LOG
#define FORMATTER_H_RPC_LOG

#include <string>
#include <functional>

namespace logging {
class Record;

// Formatter类型，所有将日志记录转换成字符串的函数/仿函数和Lambda都可以作为Formatter
using Formatter = std::function<std::string(const Record& record)>;
namespace formatters {
// formatters用于格式化日志记录对象
// 这个命名空间用来提醒这是一个formatter
std::string format_record(const logging::Record& record);
}
}


#endif