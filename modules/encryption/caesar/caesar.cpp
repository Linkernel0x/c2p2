#include "caesar.hpp"
#include <string>
#include <format>

namespace c2p2::modules {

    static DataBuffer transform_caesar(const DataBuffer& input, int shift) {
        DataBuffer output;
        output.reserve(input.size());

        shift = (shift % 26 + 26) % 26;

        for (std::byte b : input) {
            char c = static_cast<char>(b);

            if (c >= 'a' && c <= 'z') {
                c = static_cast<char>('a' + (c - 'a' + shift) % 26);
            } else if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>('A' + (c - 'A' + shift) % 26);
            }

            output.push_back(static_cast<std::byte>(c));
        }

        return output;
    }

    std::expected<DataBuffer, ModuleError> Caesar::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        int shift = 3;

        if (auto it = params.find("shift"); it != params.end()) {
            try {
                shift = std::stoi(it->second);
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid 'shift' parameter (must be an integer)"});
            }
        }

        if (action == "encrypt") {
            return transform_caesar(input, shift);
        }

        if (action == "decrypt") {
            return transform_caesar(input, -shift);
        }

        return std::unexpected(ModuleError{
            .message = std::format("Unsupported action '{}' for Caesar module", action)
        });
    }

}