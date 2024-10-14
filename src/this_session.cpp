#include "this_session.h"

namespace rpc {
this_session_t& this_session() {
    static thread_local this_session_t inst;
    return inst;
}

void this_session_t::post_exit() {
    exit_.store(true);
}

session_id_t this_session_t::id() const {
    return id_;
}

void this_session_t::clear() {
    exit_.store(false);
}

void this_session_t::id(session_id_t id_arg) {
    id_ = id_arg;
}
}