#ifndef DEV_UTILS_H_RPC
#define DEV_UTILS_H_RPC

#ifdef LINUX_RPC
#include "pthread.h"
#endif
#include <string>

namespace rpc::detail {
inline void name_thread(const std::string& name) {
    (void)name;
    #ifdef LINUX_RPC
    pthread_setname_np(pthread_self(), name.data());
    #endif
}
}

#endif