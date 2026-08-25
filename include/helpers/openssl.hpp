#pragma once
#include <openssl/evp.h>
#include <memory>

namespace c2p2::modules {
    //to avoid duplication in modules
    struct EvpCipherDeleter { void operator()(EVP_CIPHER* ptr) const { EVP_CIPHER_free(ptr); } };
    struct EvpMdDeleter { void operator()(EVP_MD* ptr) const { EVP_MD_free(ptr); } };
    struct EvpMdCtxDeleter { void operator()(EVP_MD_CTX* ptr) const { EVP_MD_CTX_free(ptr); } };
    struct EvpCipherCtxDeleter { void operator()(EVP_CIPHER_CTX* ptr) const { EVP_CIPHER_CTX_free(ptr); } };

    using UniqueCipher = std::unique_ptr<EVP_CIPHER, EvpCipherDeleter>;
    using UniqueMd = std::unique_ptr<EVP_MD, EvpMdDeleter>;
    using UniqueMdCtx = std::unique_ptr<EVP_MD_CTX, EvpMdCtxDeleter>;
    using UniqueCipherCtx = std::unique_ptr<EVP_CIPHER_CTX, EvpCipherCtxDeleter>;
}
