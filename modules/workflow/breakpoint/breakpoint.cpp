#include "breakpoint.hpp"

namespace c2p2::modules
{
    std::expected<DataBuffer, ModuleError> Breakpoint::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
        ) const {
        if (action == "gentle") {
            return input;
        }
        return std::unexpected(ModuleError{.message = "You reached a breakpoint."});
    }
}