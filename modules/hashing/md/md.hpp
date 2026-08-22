#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Md : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "md"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"hash", "check"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "MD Module:\n"
                               "Actions:\n"
                               "  hash: Calculate the MD hash of the input data.\n"
                               "  check: Verify the MD hash of the input data against a provided hash. In case of mismatch returns an error.\n"
                               "Parameters:\n"
                               "  --variant=<string>: Specify the MD variant (must be 4 or 5). Default: 5.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };
}