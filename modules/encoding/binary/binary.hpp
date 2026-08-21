#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Binary : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "binary"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Binary Module:\n"
                               "Actions:\n"
                               "  encode: Encode input data to binary representation.\n"
                               "  decode: Decode binary input data.\n"
                               "Parameters:\n"
                               "  --delimiter=<string>: Specify a string to insert between each byte in the output. (Default is space).\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}