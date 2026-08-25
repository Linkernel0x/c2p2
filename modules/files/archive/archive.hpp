#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Archive : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "archive"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"compress", "decompress"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Archive Module:\n"
                               "Actions:\n"
                               "  compress: Compress input data.\n"
                               "  decompress: Decompress input data.\n"
                               "Parameters:\n"
                               "  --format=<string> The format type (Default: zip)\n"
                               "  --filename=<string>: Specify the filename for the compressed file (default: 'data.bin').\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}