#include "aes.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <algorithm>
#include <memory>

//TODO: test commands
namespace c2p2::modules
{
    // Custom deleters
    struct EvpCipherDeleter { void operator()(EVP_CIPHER* ptr) const { EVP_CIPHER_free(ptr); } };
    struct EvpCtxDeleter { void operator()(EVP_CIPHER_CTX* ptr) const { EVP_CIPHER_CTX_free(ptr); } };

    using UniqueCipher = std::unique_ptr<EVP_CIPHER, EvpCipherDeleter>;
    using UniqueCtx = std::unique_ptr<EVP_CIPHER_CTX, EvpCtxDeleter>;

    static std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
        std::vector<unsigned char> bytes;
        bytes.reserve(hex.size() / 2);
        for (size_t i = 0; i + 1 < hex.size(); i += 2) {
            const auto byte = static_cast<unsigned char>(std::strtol(hex.substr(i, 2).c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    std::expected<DataBuffer, ModuleError> Aes::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        std::string key_str, cipher_p = "aes-256-cbc";

        if (auto it = params.find("--key"); it != params.end()) key_str = it->second;
        else return std::unexpected(ModuleError{.message = "Missing required parameter '--key'"});

        if (auto it = params.find("--cipher"); it != params.end()) cipher_p = it->second;

        UniqueCipher cipher(EVP_CIPHER_fetch(nullptr, cipher_p.c_str(), nullptr));
        if (!cipher) {
            return std::unexpected(ModuleError{.message = "Unsupported cipher: " + cipher_p});
        }

        const int key_len = EVP_CIPHER_get_key_length(cipher.get());
        const int iv_len = EVP_CIPHER_get_iv_length(cipher.get());
        const bool is_gcm = (EVP_CIPHER_get_mode(cipher.get()) == EVP_CIPH_GCM_MODE);

        std::vector<unsigned char> key(key_len, 0);
        std::copy_n(key_str.begin(), std::min(key_str.size(), key.size()), key.begin());

        std::vector<unsigned char> iv(iv_len, 0);
        bool iv_was_generated = false;

        if (action == "encrypt") {
            if (auto it = params.find("--iv"); it != params.end()) {
                std::vector<unsigned char> parsed_iv = hex_to_bytes(it->second);
                std::copy_n(parsed_iv.begin(), std::min(parsed_iv.size(), iv.size()), iv.begin());
            } else if (iv_len > 0) {
                if (RAND_bytes(iv.data(), iv_len) != 1) {
                    return std::unexpected(ModuleError{.message = "Failed to generate random IV"});
                }
                iv_was_generated = true;
            }
        }

        const auto* in_ptr = reinterpret_cast<const unsigned char*>(input.data());
        size_t cipher_data_offset = 0;
        size_t cipher_data_len = input.size();

        if (action == "decrypt") {
            if (auto it = params.find("--iv"); it != params.end()) {
                std::vector<unsigned char> parsed_iv = hex_to_bytes(it->second);
                std::copy_n(parsed_iv.begin(), std::min(parsed_iv.size(), iv.size()), iv.begin());
            } else if (iv_len > 0) {
                if (input.size() < static_cast<size_t>(iv_len)) {
                    return std::unexpected(ModuleError{.message = "Input too short to extract prepended IV"});
                }
                std::copy_n(in_ptr, iv_len, iv.begin());
                cipher_data_offset += iv_len;
                cipher_data_len -= iv_len;
            }
        }

        UniqueCtx ctx(EVP_CIPHER_CTX_new());
        if (!ctx) {
            return std::unexpected(ModuleError{.message = "Failed to create OpenSSL CTX"});
        }

        std::vector<unsigned char> output;
        output.reserve(cipher_data_len + EVP_MAX_BLOCK_LENGTH + (is_gcm ? 16 : 0) + (iv_was_generated ? iv_len : 0));

        if (iv_was_generated) {
            output.insert(output.end(), iv.begin(), iv.end());
        }

        int temp_outl = 0;
        size_t total_outl = output.size();
        output.resize(output.capacity());

        if (action == "encrypt") {
            if (EVP_EncryptInit_ex(ctx.get(), cipher.get(), nullptr, key.data(), iv.data()) != 1) {
                return std::unexpected(ModuleError{.message = "EVP_EncryptInit_ex failed"});
            }

            if (EVP_EncryptUpdate(ctx.get(), output.data() + total_outl, &temp_outl, in_ptr, static_cast<int>(input.size())) != 1) {
                return std::unexpected(ModuleError{.message = "EVP_EncryptUpdate failed"});
            }
            total_outl += temp_outl;

            if (EVP_EncryptFinal_ex(ctx.get(), output.data() + total_outl, &temp_outl) != 1) {
                return std::unexpected(ModuleError{.message = "EVP_EncryptFinal_ex failed"});
            }
            total_outl += temp_outl;

            if (is_gcm) {
                std::vector<unsigned char> tag(16);
                if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
                    return std::unexpected(ModuleError{.message = "Failed to get GCM tag"});
                }
                std::ranges::copy(tag, output.begin() + total_outl);
                total_outl += 16;
            }

        } else if (action == "decrypt") {
            std::vector<unsigned char> tag(16);

            if (is_gcm) {
                if (cipher_data_len < 16) {
                    return std::unexpected(ModuleError{.message = "Input too short for GCM tag"});
                }
                cipher_data_len -= 16;
                std::copy_n(in_ptr + cipher_data_offset + cipher_data_len, 16, tag.begin());
            }

            if (EVP_DecryptInit_ex(ctx.get(), cipher.get(), nullptr, key.data(), iv.data()) != 1) {
                return std::unexpected(ModuleError{.message = "EVP_DecryptInit_ex failed"});
            }

            if (EVP_DecryptUpdate(ctx.get(), output.data() + total_outl, &temp_outl, in_ptr + cipher_data_offset, static_cast<int>(cipher_data_len)) != 1) {
                return std::unexpected(ModuleError{.message = "EVP_DecryptUpdate failed"});
            }
            total_outl += temp_outl;

            if (is_gcm) {
                if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, 16, tag.data()) != 1) {
                    return std::unexpected(ModuleError{.message = "Failed to set GCM tag"});
                }
            }

            if (EVP_DecryptFinal_ex(ctx.get(), output.data() + total_outl, &temp_outl) <= 0) {
                return std::unexpected(ModuleError{.message = "Decryption failed (bad key/IV or invalid tag)"});
            }
            total_outl += temp_outl;
        }

        output.resize(total_outl);

        DataBuffer result;
        result.reserve(total_outl);
        for (const auto b : output) {
            result.push_back(static_cast<std::byte>(b));
        }

        return result;
    }
}