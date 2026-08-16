#include "rot13.hpp"

namespace c2p2::modules {

    std::expected<DataBuffer, ModuleError> Rot13::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        output.reserve(input.size());

        for (std::byte b : input) {
            char c = static_cast<char>(b);
            if (c >= 'a' && c <= 'z') {
                c = 'a' + (c - 'a' + 13) % 26;
            } else if (c >= 'A' && c <= 'Z') {
                c = 'A' + (c - 'A' + 13) % 26;
            }
            output.push_back(static_cast<std::byte>(c));
        }

        return output;
    }

}