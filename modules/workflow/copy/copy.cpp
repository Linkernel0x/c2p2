#include "copy.hpp"
#include "helpers/ui.hpp"

namespace c2p2::modules
{
    std::expected<DataBuffer, ModuleError> Copy::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        std::string file_path;

        if (const auto it = params.find("--file"); it != params.end()) {
            file_path = it->second;
            if (file_path.empty()) {
                return std::unexpected(ModuleError{.message = "Parameter '--file' cannot be empty"});
            }
        } else {
            return std::unexpected(ModuleError{.message = "Missing required parameter: --file"});
        }

        helpers::write_file_async(file_path, input);

        return input;
    }
}