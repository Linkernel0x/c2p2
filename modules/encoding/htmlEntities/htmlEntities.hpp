#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class HtmlEntities : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "html-entities"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "HTML Entities Module:\n"
                               "Actions:\n"
                               "  encode: Encode input data to HTML entities.\n"
                               "  decode: Decode HTML entities input data.\n"
                               "Parameters:\n"
                               "  --format: (optional) Specify the format of the output. Options are 0 (named), 1 (decimal), or 2 (hexadecimal). Default is 0.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}