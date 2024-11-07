#ifndef LOG_H_RPC
#define LOG_H_RPC

#ifdef RPC_ENABLE_LOGGING
// tmp
#else

#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_DEBUG(...)
#define LOG_TRACE(...)
#define LOG_EXPR(...)
#define RPC_CREATE_LOG_CHANNEL(...)

#endif

#endif