#include "rot8000.hpp"

namespace c2p2::modules {

    constexpr uint32_t ROT8000_START = 0x0021;
    constexpr uint32_t ROT8000_END = 0x7E7D;
    constexpr uint32_t ROT8000_RANGE = 0x8000;
    constexpr uint32_t ROT8000_SHIFT = 0x4000;

    static uint32_t transform_code_point(uint32_t cp) {
        // only characters in the range
        if (cp >= ROT8000_START && cp <= ROT8000_END) {
            return (cp - ROT8000_START + ROT8000_SHIFT) % ROT8000_RANGE + ROT8000_START;
        }
        return cp;
    }

    //from 32bit Unicode to UTF-8 bytes
    static void append_utf8_bytes(DataBuffer& out, uint32_t cp) {
        if (cp <= 0x7F) {
            // 1-byte ASCII
            out.push_back(static_cast<std::byte>(cp));
        } else if (cp <= 0x7FF) {
            // 2-byte (110xxxxx 10xxxxxx)
            out.push_back(static_cast<std::byte>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<std::byte>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            // 3-byte (1110xxxx 10xxxxxx 10xxxxxx)
            out.push_back(static_cast<std::byte>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<std::byte>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<std::byte>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0x10FFFF) {
            // 4-byte (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
            out.push_back(static_cast<std::byte>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<std::byte>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<std::byte>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<std::byte>(0x80 | (cp & 0x3F)));
        }
    }

    static DataBuffer process_rot8000(const DataBuffer& input) {
        DataBuffer output;
        output.reserve(input.size());

        size_t i = 0;
        while (i < input.size()) {
            uint32_t cp = 0;
            size_t bytes_consumed = 0;

            if (const auto b0 = static_cast<unsigned char>(input[i]); b0 <= 0x7F) {
                cp = b0;
                bytes_consumed = 1;
            } else if ((b0 & 0xE0) == 0xC0 && i + 1 < input.size()) {
                const auto b1 = static_cast<unsigned char>(input[i + 1]);
                cp = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
                bytes_consumed = 2;
            } else if ((b0 & 0xF0) == 0xE0 && i + 2 < input.size()) {
                const auto b1 = static_cast<unsigned char>(input[i + 1]);
                const auto b2 = static_cast<unsigned char>(input[i + 2]);
                cp = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
                bytes_consumed = 3;
            } else if ((b0 & 0xF8) == 0xF0 && i + 3 < input.size()) {
                const auto b1 = static_cast<unsigned char>(input[i + 1]);
                const auto b2 = static_cast<unsigned char>(input[i + 2]);
                const auto b3 = static_cast<unsigned char>(input[i + 3]);
                cp = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
                bytes_consumed = 4;
            } else {
                //malformed utf-8, copy the byte as is
                output.push_back(input[i]);
                ++i;
                continue;
            }

            const uint32_t transformed_cp = transform_code_point(cp);
            append_utf8_bytes(output, transformed_cp);

            i += bytes_consumed;
        }

        return output;
    }

    std::expected<DataBuffer, ModuleError> Rot8000::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
        ) const {
        DataBuffer output;

        if (action == "encrypt" || action == "decrypt") {
            output = process_rot8000(input);
        }

        return output;
    }
}