#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Breakpoint : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "breakpoint"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"process"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Breakpoint Module:\n"
                               "Actions:\n"
                               "  process: Stops the pipeline and returns an error message.\n"
                               "  gentle: Stops the pipeline and returns the current result.\n"
                               "Parameters:\n"
                               "  No parameters required\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}