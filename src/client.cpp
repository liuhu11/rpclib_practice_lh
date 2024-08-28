#include <cstdint>

#include "rpc/client.h"
#include "rpc/config.h"

namespace rpc{
static constexpr uint32_t default_buffer_size = rpc::Constants::DEFAULT_BUFFER_SIZE;

struct Client::impl {
public:
    Client* parent;
};
}