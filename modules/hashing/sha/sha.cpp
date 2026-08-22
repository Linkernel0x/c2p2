#include "sha.hpp"
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
        std::string variant = "sha256";
        std::string check_hash;
        std::string format = "hex";

        if (const auto it = params.find("--variant"); it != params.end()) {
            try {
                if (it->second != "sha1" && it->second != "sha224" && it->second != "sha256" && it->second != "sha384" && it->second != "sha512" && it->second != "sha3-256" && it->second != "sha3-512") {
                    return std::unexpected(ModuleError{.message = "'variant' parameter is required (must be 'sha1', 'sha224', 'sha256', 'sha384', 'sha512', 'sha3-256', or 'sha3-512')"});
                }
                variant = it->second;
            } catch (...) {
                return std::unexpected(ModuleError{.message = "'variant' parameter is required (must be 'sha1', 'sha224', 'sha256', 'sha384', 'sha512', 'sha3-256', or 'sha3-512')"});
            }
        } else {
            return std::unexpected(ModuleError{.message = "'variant' parameter is required (must be 'sha1', 'sha224', 'sha256', 'sha384', 'sha512', 'sha3-256', or 'sha3-512')"});
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

        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (variant == "sha1") {
            EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
        } else if (variant == "sha224") {
            EVP_DigestInit_ex(ctx, EVP_sha224(), nullptr);
        } else if (variant == "sha256") {
            EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
        } else if (variant == "sha384") {
            EVP_DigestInit_ex(ctx, EVP_sha384(), nullptr);
        } else if (variant == "sha512") {
            EVP_DigestInit_ex(ctx, EVP_sha512(), nullptr);
        } else if (variant == "sha3-256") {
            EVP_DigestInit_ex(ctx, EVP_sha3_256(), nullptr);
        } else if (variant == "sha3-512") {
            EVP_DigestInit_ex(ctx, EVP_sha3_512(), nullptr);
        } else {
            EVP_MD_CTX_free(ctx);
            return std::unexpected(ModuleError{.message = "'variant' parameter is required (must be 'sha1', 'sha224', 'sha256', 'sha384', 'sha512', 'sha3-256', or 'sha3-512')"});
        }

        EVP_DigestUpdate(ctx, input.data(), input.size());
        unsigned char result[EVP_MAX_MD_SIZE];
        unsigned int result_length = 0;
        EVP_DigestFinal_ex(ctx, result, &result_length);

        EVP_MD_CTX_free(ctx);

        for(unsigned int i = 0; i < result_length; i++) {
            output.push_back(static_cast<std::vector<std::byte>::value_type>(result[i]));
        }

        std::string hash_hex;
        for (unsigned int i = 0; i < result_length; ++i) {
            hash_hex += std::format("{:02x}", result[i]);
        }

        if (action == "check") {
            if (hash_hex == check_hash) {
                return string_to_databuffer(hash_hex);
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