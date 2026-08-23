#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class CaesarBox : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "caesar_box"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encrypt", "decrypt"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Caesar Box Cipher Module\n"
                                     "Usage:\n"
                                     "  encrypt: Encrypts the input using the Caesar Box cipher.\n"
                                     "  decrypt: Decrypts the input using the Caesar Box cipher.\n"
                                     "Parameters:\n"
                                     "  --length=<int>: The length of the box (default: 5).\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}