#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Copy : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "copy"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"input"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Copy Module:\n"
                               "Actions:\n"
                               "  input: Log the input\n"
                               "Parameters:\n"
                               "  --file=<string>: File to write logs on\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}