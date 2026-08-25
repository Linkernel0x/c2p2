#include "base64.hpp"
#include <format>

namespace c2p2::modules {

    std::expected<DataBuffer, ModuleError> Base64::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        output.reserve((input.size() + 2) / 3 * 4); // b64 expands bytes in ASCII
        static constexpr std::string_view base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        if (action == "encode") {
            for (size_t i = 0; i + 2 < input.size(); i += 3) {
                // process input
                uint32_t bit24 = (static_cast<uint8_t>(input[i])     << 16) |
                                 (static_cast<uint8_t>(input[i + 1]) << 8)  |
                                  static_cast<uint8_t>(input[i + 2]);

                // split the 24bit int into 6bit chars
                uint8_t idx1 = (bit24 >> 18) & 0x3F;
                uint8_t idx2 = (bit24 >> 12) & 0x3F;
                uint8_t idx3 = (bit24 >> 6)  & 0x3F;
                uint8_t idx4 =  bit24        & 0x3F;

                output.push_back(static_cast<std::byte>(base64_chars[idx1]));
                output.push_back(static_cast<std::byte>(base64_chars[idx2]));
                output.push_back(static_cast<std::byte>(base64_chars[idx3]));
                output.push_back(static_cast<std::byte>(base64_chars[idx4]));
            }
            size_t remaining = input.size() % 3;

            if (remaining == 1) {
                //forms two 6-bit chunks + '==' padding
                uint32_t bit24 = static_cast<uint8_t>(input[input.size() - 1]) << 16;

                output.push_back(static_cast<std::byte>(base64_chars[(bit24 >> 18) & 0x3F]));
                output.push_back(static_cast<std::byte>(base64_chars[(bit24 >> 12) & 0x3F]));
                output.push_back(static_cast<std::byte>('='));
                output.push_back(static_cast<std::byte>('='));
            }
            else if (remaining == 2) {
                //same thing but with three 6-bit chunks + '=' padding
                uint32_t bit24 = (static_cast<uint8_t>(input[input.size() - 2]) << 16) |
                                 (static_cast<uint8_t>(input[input.size() - 1]) << 8);

                output.push_back(static_cast<std::byte>(base64_chars[(bit24 >> 18) & 0x3F]));
                output.push_back(static_cast<std::byte>(base64_chars[(bit24 >> 12) & 0x3F]));
                output.push_back(static_cast<std::byte>(base64_chars[(bit24 >> 6)  & 0x3F]));
                output.push_back(static_cast<std::byte>('='));
            }
            return output;
        }
        else if (action == "decode") {
            // decode a singke char to its 6bit value
            auto decode_char = [&](char c) -> std::expected<uint32_t, ModuleError> {
                if (c == '=') return 0;
                auto pos = base64_chars.find(c);
                if (pos == std::string_view::npos) {
                    return std::unexpected(ModuleError{.message = std::format("Invalid Base64 character: '{}'", c)});
                }
                return static_cast<uint32_t>(pos);
            };

            // process in chunks of 4
            for (size_t i = 0; i + 3 < input.size(); i += 4) {
                char c1 = static_cast<char>(input[i]);
                char c2 = static_cast<char>(input[i + 1]);
                char c3 = static_cast<char>(input[i + 2]);
                char c4 = static_cast<char>(input[i + 3]);

                auto val1 = decode_char(c1);
                auto val2 = decode_char(c2);
                auto val3 = decode_char(c3);
                auto val4 = decode_char(c4);

                if (!val1 || !val2 || !val3 || !val4) {
                    return std::unexpected(ModuleError{.message = "Base64 decode error: invalid payload or character"});
                }

                // reconstruct
                uint32_t bit24 = (*val1 << 18) |
                                 (*val2 << 12) |
                                 (*val3 << 6)  |
                                  *val4;

                //extract and ignores =
                uint8_t byte1 = (bit24 >> 16) & 0xFF;
                output.push_back(static_cast<std::byte>(byte1));

                if (c3 != '=') {
                    uint8_t byte2 = (bit24 >> 8) & 0xFF;
                    output.push_back(static_cast<std::byte>(byte2));
                }

                if (c4 != '=') {
                    uint8_t byte3 = bit24 & 0xFF;
                    output.push_back(static_cast<std::byte>(byte3));
                }
            }
            return output;
        }

        return std::unexpected(ModuleError{
            .message = std::format("Unsupported action '{}' for Base64 module", action)
        });
    }

}