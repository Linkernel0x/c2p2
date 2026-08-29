#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Delay : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "delay"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"process"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Delay Module:\n"
                               "Actions:\n"
                               "  process: Delay the execution\n"
                               "Parameters:\n"
                               "  --ms=<int>: Milliseconds to delay\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}