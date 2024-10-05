#ifndef CLIENT_ERROR_H_CPP
#define CLIENT_ERROR_H_CPP

#include <stdexcept>
#include <string>

namespace rpc::detail {
//! \brief Describes an error that is the result of a connected client
//! doing something unexpected (e.g. calling a function that does not exist,
//! wrong number of arguments, etc.)
class ClientError : public std::exception {
private:
    std::string what_;
public:
    enum class code {
        no_such_function = 1,
        wrong_arity = 2,
        protocol_error = 4
    };
    ClientError(code c, const std::string& msg);
    const char* what() const noexcept;

};
}

#endif