#include "this_server.h"

namespace rpc {
this_server_t& this_server() {
    static thread_local this_server_t inst;
    return  inst;
}

void this_server_t::stop() {
    stopping_ = true;
}

void this_server_t::cancel_stop() {
    stopping_ = false;
}

bool this_server_t::stopping() const {
    return stopping_;
}
}