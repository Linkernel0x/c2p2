#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Url : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "url"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "URL Encoding Module:\n"
                               "Actions:\n"
                               "  encode: Encode input data to URL encoding.\n"
                               "  decode: Decode URL encoded input data.\n"
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