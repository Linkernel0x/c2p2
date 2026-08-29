#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Random : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "random"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"uuid", "bytes", "token"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Random Module:\n"
                               "Actions:\n"
                               "  uuid: Generates a new UUID.\n"
                               "  bytes: Generates a new random byte sequence.\n"
                               "  token: Generates a new random token.\n"
                               "Parameters:\n"
                               "  --seed=<int>: Optional seed for random number generation.\n"
                               "  --size=<int>: Size of the random byte sequence or token (default: 32).\n"
                               "  --charset=<string>: Character set for token generation (default: alphanumeric).\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}