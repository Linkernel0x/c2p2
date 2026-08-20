#include "rot47.hpp"

namespace c2p2::modules
{
    static DataBuffer transform_caesar(const DataBuffer& input, int shift) {
        DataBuffer output;
        output.reserve(input.size());

        shift = (shift % 94 + 94) % 94;

        for (std::byte b : input) {
            char c = static_cast<char>(b);
            if (c >= '!' && c <= '~') {
                c = '!' + (c - '!' + shift) % 94;
            }
            output.push_back(static_cast<std::byte>(c));
        }

        return output;
    }

    std::expected<DataBuffer, ModuleError> Rot47::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
        ) const {
        DataBuffer output;
        int shift = 47;

        if (auto it = params.find("--shift"); it != params.end()) {
            try {
                shift = std::stoi(it->second);
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid 'shift' parameter (must be an integer)"});
            }
        }

        if (action == "encrypt") {
            output = transform_caesar(input, shift);
        }

        if (action == "decrypt") {
            output = transform_caesar(input, -shift);
        }

        return output;
    }
}