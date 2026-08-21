#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Vigenere : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "vigenere"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encrypt", "decrypt"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Vigenère Cipher Module:\n"
                               "Actions:\n"
                               "  encrypt: Encrypt input data using Vigenère cipher.\n"
                               "  decrypt: Decrypt Vigenère cipher input data.\n"
                               "Parameters:\n"
                               "  --key=<string>: Specify the key value for the cipher.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}