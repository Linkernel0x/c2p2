#include "delay.hpp"
#include <thread>
#include "helpers/ui.hpp"

namespace c2p2::modules
{
    std::expected<DataBuffer, ModuleError> Delay::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        int millis;

        if (const auto it = params.find("--ms"); it != params.end()) {
            try {
                millis = std::stoi(it->second);
                if (millis < 0) {
                    return std::unexpected(ModuleError{.message = "Parameter '--ms' must be a non-negative integer"});
                }
            } catch (const std::exception&) {
                return std::unexpected(ModuleError{.message = "Parameter '--ms' is not a valid integer"});
            }
        } else {
            return std::unexpected(ModuleError{.message = "Missing required parameter: --ms"});
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(millis));

        return input;
    }
}