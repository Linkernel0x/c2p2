#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Enigma : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "enigma"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"process"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Enigma M3 Cipher Module:\n"
                               "Actions:\n"
                               "  process: Encrypt or Decrypt data (Enigma is symmetric).\n"
                               "Parameters:\n"
                               "  --config=<path>: path to the machine's configuration.\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}