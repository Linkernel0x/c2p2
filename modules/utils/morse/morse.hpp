#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Morse : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "morse"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Morse Code Module:\n"
                               "Actions:\n"
                               "  encode: Converts the input in Morse Code.\n"
                               "  decode: Converts the input from Morse Code.\n"
                               "Parameters:\n"
                               "  --format <string>: Specify the format to encode/decode. (Default: standard)\n"
                               "  --wpm <integer>: Specify the words per minute for audio generation. (Default: 20)\n"
                               "  --frequency <integer>: Specify the frequency in Hz for audio generation. (Default: 700)\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}