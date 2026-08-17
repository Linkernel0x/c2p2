#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Rot13 : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "rot13"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "ROT13 Module:\n"
                               "Actions:\n"
                               "  encode: Encode input data using ROT13.\n"
                               "  decode: Decode ROT13 input data.\n"
                               "Parameters:\n"
                               "  None for encode/decode.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}