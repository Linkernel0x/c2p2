#include "binary.hpp"

#include <bitset>
#include <format>

namespace c2p2::modules {

    std::expected<DataBuffer, ModuleError> Binary::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        std::string delimiter = " ";
        if (const auto it = params.find("--delimiter"); it != params.end()) {
            delimiter = it->second;
        }

        if (action == "encode") {
            output.reserve(input.size() * (8 + delimiter.size()));
            for (size_t i = 0; i < input.size(); i++) {
                auto val = std::to_integer<uint8_t>(input[i]);

                std::string bit_str = std::bitset<8>(val).to_string();

                for (char c : bit_str) {
                    output.push_back(static_cast<std::byte>(c));
                }
                if (i + 1 < input.size()) {
                    for (char c : delimiter) {
                        output.push_back(static_cast<std::byte>(c));
                    }
                }
            }
        }
        else if (action == "decode") {
            DataBuffer clean_bits;
            clean_bits.reserve(input.size());

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

                if (c != '0' && c != '1') {
                    return std::unexpected(ModuleError{
                        .message = "Binary decode error: Invalid character found (must be only '0' or '1')"
                    });
                }

                clean_bits.push_back(input[i]);
                i++;
            }

            if (clean_bits.size() % 8 != 0) {
                return std::unexpected(ModuleError{
                    .message = "Binary decode error: Total number of bits must be a multiple of 8"
                });
            }

            output.reserve(clean_bits.size() / 8);

            for (size_t bit_idx = 0; bit_idx < clean_bits.size(); bit_idx += 8) {
                uint8_t byte_val = 0;

                for (size_t b = 0; b < 8; ++b) {
                    char bit_char = static_cast<char>(clean_bits[bit_idx + b]);
                    if (bit_char == '1') {
                        byte_val |= (1 << (7 - b));
                    }
                }

                output.push_back(static_cast<std::byte>(byte_val));
            }
        }

        return output;
    }

}