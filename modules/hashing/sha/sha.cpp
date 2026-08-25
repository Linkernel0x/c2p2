#include "sha.hpp"
#include "helpers/openssl.hpp"
#include <string>
#include <format>
#include <openssl/evp.h>

namespace c2p2::modules
{
    std::expected<DataBuffer, ModuleError> Sha::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        std::string variant;
        std::string check_hash;
        std::string format = "hex";

        if (const auto it = params.find("--variant"); it != params.end()) {
            variant = it->second;
        } else {
            return std::unexpected(ModuleError{.message = "'variant' parameter is required"});
        }
        if (const auto it = params.find("--hash"); it != params.end()) {
            try {
                check_hash = it->second;
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid 'hash' parameter (must be a valid hash string)"});
            }
        } else if (action == "check") {
            return std::unexpected(ModuleError{.message = "'hash' parameter is required for 'check' action"});
        }
        if (const auto it = params.find("--format"); it != params.end()) {
            try {
                if (it->second != "hex" && it->second != "binary") {
                    return std::unexpected(ModuleError{.message = "Invalid 'format' parameter (must be 'hex' or 'binary')"});
                }
                format = it->second;
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid 'format' parameter (must be 'hex' or 'binary')"});
            }
        }

        UniqueMdCtx ctx(EVP_MD_CTX_new());
        if (!ctx) {
            return std::unexpected(ModuleError{.message = "Failed to create OpenSSL CTX"});
        }

        if (!variant.contains("sha")) {
            return std::unexpected(ModuleError{.message = "Unsupported SHA algorithm: " + variant});
        }
        UniqueMd md(EVP_MD_fetch(nullptr, variant.c_str(), nullptr));
        if (!md) {
            return std::unexpected(ModuleError{.message = "Unsupported SHA algorithm: " + variant});
        }
        EVP_DigestInit_ex(ctx.get(), md.get(), nullptr);

        EVP_DigestUpdate(ctx.get(), input.data(), input.size());
        unsigned char result[EVP_MAX_MD_SIZE];
        unsigned int result_length = 0;
        EVP_DigestFinal_ex(ctx.get(), result, &result_length);

        std::string hash_hex;
        for (unsigned int i = 0; i < result_length; ++i) {
            hash_hex += std::format("{:02x}", result[i]);
        }

        if (action == "check") {
            if (hash_hex == check_hash) {
                return input;
            }
            return std::unexpected(ModuleError{.message = std::format("Hash check failed. Computed: {}, Provided: {}", hash_hex, check_hash)});
        }

        if (format == "binary") {
            output.reserve(result_length);
            for (unsigned int i = 0; i < result_length; ++i) {
                output.push_back(static_cast<std::byte>(result[i]));
            }
            return output;
        }

        return string_to_databuffer(hash_hex);
    }

}