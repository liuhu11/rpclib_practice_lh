#ifndef THIS_SESSION_H_RPC
#define THIS_SESSION_H_RPC

#include <atomic>

#include "config.h"

namespace rpc {
namespace detail {
class ServerSession;
}

//! \brief Encapsulates information about the server session/connection
//! this handler is running in. This is the interface through which bound
//! functions may interact with the session.
//! \note Accessing the this_session() object is thread safe, but incurs some
//! syncrhonization cost in the form of atomic flags. (usually not a concern)
class this_session_t {
private:
    std::atomic_bool exit_{false};
    session_id_t id_{0};
public:
    friend detail::ServerSession;
    //! \brief Gracefully exits the session (i.e. ongoing writes and reads are
    //! completed; queued writes and reads are not).
    //! \note Use this function if you need to close the connection from a
    //! handler.
    void post_exit();

    //! \brief Returns an ID that uniquely identifies a session. 
    //! \note This is not an ID for the client. If the client disconnects
    //! and reconnects, this ID may change. That being said, you can
    //! use this ID to store client-specific information *for the duration
    //! of the session.
    session_id_t id() const;
private:
    void clear();
    void id(session_id_t id_arg);

};

//! \brief A thread-local object that can be used to control the currently
//! active server session.
//! \note Accessing this object outside of handlers while a server is
//! running is potentially unsafe.
this_session_t& this_session();
}



#endif