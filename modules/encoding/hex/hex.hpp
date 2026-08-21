#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Hex : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "hex"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Hex Module:\n"
                               "Actions:\n"
                               "  encode: Encode input data to hexadecimal representation.\n"
                               "  decode: Decode hexadecimal input data.\n"
                               "Parameters:\n"
                               "  --uppercase=<bool>: (optional) Set to 'true' or '1' to use uppercase letters in the output. (Default is lowercase).\n"
                               "  --delimiter=<string>: (optional) Specify a string to insert between each byte in the output. (Default is no delimiter).\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}