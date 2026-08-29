#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Textutils : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "textutils"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"process"};
        }

        [[nodiscard]] DataBuffer help_text() const override {
            const std::string help = "Textutils Module:\n"
                               "Actions:\n"
                               "  transform: Applies chainable text transformations in a fixed, safe order.\n"
                               "Parameters:\n"
                               "  --replace <old:new>   Replace occurrences of 'old' with 'new'\n"
                               "  --remove <type>      Remove characters: whitespace, punctuation, digits, letters\n"
                               "  --case <style>       Change casing: lower, upper, title, swap, snake_case, kebab-case\n"
                               "  --order <mode>       Reorder text: reverse, sort, sort_desc, mirror, shuffle\n"
                               "  --trim <mode>        Trim outer whitespace: left, right, both\n"
                               "  --truncate <length>  Limit text to maximum length\n"
                               "  --pad <length>       Pad text with spaces to reach minimum length\n";

            return string_to_databuffer(help);
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}