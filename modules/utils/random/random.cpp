#include "random.hpp"

#include <format>
#include <random>
#include <optional>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include <openssl/rand.h>

namespace c2p2::modules {

    static bool fill_random_bytes(std::span<std::byte> buffer, std::optional<uint64_t> seed) {
        if (seed.has_value()) {
            std::mt19937_64 rng(seed.value());
            for (auto& byte : buffer) {
                byte = static_cast<std::byte>(rng() & 0xFF);
            }
            return true;
        } else {
            return RAND_bytes(reinterpret_cast<unsigned char*>(buffer.data()), static_cast<int>(buffer.size())) == 1;
        }
    }

    std::expected<DataBuffer, ModuleError> Random::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        std::optional<uint64_t> seed;
        size_t size = 32;

        try {
            if (const auto it = params.find("--seed"); it != params.end()) {
                seed = std::stoull(it->second);
            }
            if (const auto it = params.find("--size"); it != params.end()) {
                size = std::stoull(it->second);
            }
        } catch (const std::exception& e) {
            return std::unexpected(ModuleError{std::format("Error while parsing parameters: {}", e.what())});
        }

        if (action == "bytes") {
            output.resize(size);
            if (!fill_random_bytes(output, seed)) {
                return std::unexpected(ModuleError{"Failed to generate secure random bytes"});
            }
        }

        else if (action == "uuid") {
            std::array<uint8_t, 16> bytes{};
            if (!fill_random_bytes(std::as_writable_bytes(std::span(bytes)), seed)) {
                return std::unexpected(ModuleError{"Failed to generate random bytes for UUID"});
            }

            bytes[6] = (bytes[6] & 0x0F) | 0x40;
            bytes[8] = (bytes[8] & 0x3F) | 0x80;

            std::string uuid_str = std::format(
                "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                bytes[0], bytes[1], bytes[2], bytes[3],
                bytes[4], bytes[5], bytes[6], bytes[7],
                bytes[8], bytes[9], bytes[10], bytes[11],
                bytes[12], bytes[13], bytes[14], bytes[15]
            );

            output = string_to_databuffer(uuid_str);
        }

        else if (action == "token") {
            std::string charset_arg = "alphanumeric";
            if (const auto it = params.find("--charset"); it != params.end()) {
                charset_arg = it->second;
            }

            std::string charset;
            if (charset_arg == "alphabetic") {
                charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            } else if (charset_arg == "numeric") {
                charset = "0123456789";
            } else if (charset_arg == "hex") {
                charset = "0123456789abcdef";
            } else if (charset_arg == "alphanumeric") {
                charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            } else if (charset_arg == "ASCII") {
                charset = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
            } else {
                charset = charset_arg;
            }

            if (charset.empty()) {
                return std::unexpected(ModuleError{"Charset cannot be empty"});
            }

            std::string token_str;
            token_str.reserve(size);

            if (seed.has_value()) {
                std::mt19937_64 rng(seed.value());
                std::uniform_int_distribution<size_t> dist(0, charset.size() - 1);
                for (size_t i = 0; i < size; ++i) {
                    token_str += charset[dist(rng)];
                }
            } else {
                const size_t max_valid = (256 / charset.size()) * charset.size();
                while (token_str.size() < size) {
                    uint8_t byte_val;
                    if (RAND_bytes(&byte_val, 1) != 1) {
                        return std::unexpected(ModuleError{"Failed to generate random bytes for token"});
                    }
                    if (byte_val < max_valid) {
                        token_str += charset[byte_val % charset.size()];
                    }
                }
            }

            output = string_to_databuffer(token_str);
        } else {
            return std::unexpected(ModuleError{std::format("Unknown action: {}", action)});
        }

        return output;
    }
}