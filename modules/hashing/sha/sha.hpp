#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Sha : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "sha"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"hash", "check"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "SHA Module:\n"
                               "Actions:\n"
                               "  hash: Calculate the SHA hash of input data.\n"
                               "  check: Verify if input matches a given hash.\n"
                               "Parameters:\n"
                               "  --variant=<string>: sha1, sha224, sha256, sha384, sha512, sha3-256, sha3-512 (Default: sha256).\n"
                               "  --hash=<string>: Hash value to verify against (Required for 'check').\n"
                               "  --format=<string>: 'hex' or 'binary' (Default: hex).\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}