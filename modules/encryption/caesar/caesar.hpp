#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Caesar : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "caesar"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encrypt", "decrypt"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Caesar Cipher Module:\n"
                               "Actions:\n"
                               "  encrypt: Encrypt input data using Caesar cipher.\n"
                               "  decrypt: Decrypt Caesar cipher input data.\n"
                               "Parameters:\n"
                               "  --shift: (optional) Specify the shift value for the cipher. Default is 3.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}