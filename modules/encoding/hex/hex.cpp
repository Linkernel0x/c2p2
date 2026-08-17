#include "hex.hpp"
#include <format>
#include <cctype>

namespace c2p2::modules {
    static uint8_t hex_to_nibble(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 255;
    }

    std::expected<DataBuffer, ModuleError> Hex::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;

        if (action == "encode") {
            bool uppercase = false;
            if (const auto it = params.find("--uppercase"); it != params.end()) {
                uppercase = (it->second == "true" || it->second == "1");
            }

            std::string delimiter;
            if (const auto it = params.find("--delimiter"); it != params.end()) {
                delimiter = it->second;
            }

            size_t extra_space = delimiter.empty() ? 0 : (!input.empty() ? (input.size() - 1) * delimiter.size() : 0);
            output.reserve(input.size() * 2 + extra_space);

            for (size_t i = 0; i < input.size(); ++i) {
                auto byte_val = static_cast<uint8_t>(input[i]);

                std::string hex_byte = uppercase
                    ? std::format("{:02X}", byte_val)
                    : std::format("{:02x}", byte_val);

                for (char c : hex_byte) {
                    output.push_back(static_cast<std::byte>(c));
                }

                if (!delimiter.empty() && i + 1 < input.size()) {
                    for (char c : delimiter) {
                        output.push_back(static_cast<std::byte>(c));
                    }
                }
            }
        }
        else if (action == "decode") {
    DataBuffer clean_input;
    clean_input.reserve(input.size());

    bool uppercase = false;
    if (const auto it = params.find("--uppercase"); it != params.end()) {
        uppercase = (it->second == "true" || it->second == "1");
    }

    std::string delimiter;
    if (const auto it = params.find("--delimiter"); it != params.end()) {
        delimiter = it->second;
    }

    size_t i = 0;
    while (i < input.size()) {
        char c = static_cast<char>(input[i]);
        if (!delimiter.empty() && i + delimiter.size() <= input.size()) {
            std::string_view sub(reinterpret_cast<const char*>(input.data() + i), delimiter.size());
            if (sub == delimiter) {
                i += delimiter.size();
                continue;
            }
        }

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        if (uppercase && (c >= 'a' && c <= 'f')) {
            return std::unexpected(ModuleError{
                .message = "Hex decode error: Lowercase character found while --uppercase is active"
            });
        }

        clean_input.push_back(input[i]);
        i++;
    }

    if (clean_input.size() % 2 != 0) {
        return std::unexpected(ModuleError{
            .message = "Hex decode error: Input length must be even (excluding whitespace and delimiters)"
        });
    }

    output.reserve(clean_input.size() / 2);

    for (size_t idx = 0; idx < clean_input.size(); idx += 2) {
        const uint8_t high = hex_to_nibble(static_cast<char>(clean_input[idx]));
        const uint8_t low = hex_to_nibble(static_cast<char>(clean_input[idx + 1]));

        if (high == 255 || low == 255) {
            return std::unexpected(ModuleError{
                .message = "Hex decode error: Invalid hex character found"
            });
        }

        uint8_t byte_val = (high << 4) | low;
        output.push_back(static_cast<std::byte>(byte_val));
    }
}

        return output;
    }

}