#include "md.hpp"
#include <string>
#include <format>
#include <openssl/evp.h>

namespace c2p2::modules {

    std::expected<DataBuffer, ModuleError> Md::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        int type = 5;
        std::string check_hash;
        std::string format = "hex";

        if (const auto it = params.find("--variant"); it != params.end()) {
            try {
                if (it->second != "4" && it->second != "5") {
                    return std::unexpected(ModuleError{.message = "'md' parameter is required (must be 4 or 5)"});
                }
                type = std::stoi(it->second);
            } catch (...) {
                return std::unexpected(ModuleError{.message = "'md' parameter is required (must be 4 or 5)"});
            }
        } else {
            return std::unexpected(ModuleError{.message = "'md' parameter is required (must be 4 or 5)"});
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
        switch (type) {
            case 4:
                EVP_DigestInit_ex(ctx, EVP_md4(), nullptr);
                break;
            case 5:
                EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
                break;

            default:
                EVP_MD_CTX_free(ctx);
                return std::unexpected(ModuleError{.message = "'md' parameter is required (must be 4 or 5)"});
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