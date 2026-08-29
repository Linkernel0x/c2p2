#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Check : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "check"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"is", "is_not", "size", "size_not"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Check Module:\n"
                               "Actions:\n"
                               "  is: Check if the input data is equal to the specified value.\n"
                               "  is_not: Check if the input data is not equal to the specified value.\n"
                               "  size: Check if the input data size is what specified in params to the value.\n"
                               "  size_not: Check if the input data size is not what specified in params to the value.\n"
                               "Parameters:\n"
                               "  --value=<string|int>: Specify the input data to check.\n"
                               "  --type=<string>: Specify the type of --value.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}