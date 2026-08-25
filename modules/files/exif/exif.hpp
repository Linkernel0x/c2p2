#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Exif : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "exif"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"get", "remove", "set"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "EXIF Module:\n"
                               "Actions:\n"
                               "  get: Extract metadata from input data.\n"
                               "  remove: Delete metadata from input data.\n"
                               "  set: Modify metadata in input data.\n"
                               "Parameters:\n"
                               "  --metadata <string>: Specify the metadata to extract/delete/modify. (Default: everything)\n"
                               "  --key <string>: Specify the metadata key to modify. (Required for set action)\n";
            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}