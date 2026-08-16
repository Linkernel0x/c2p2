#pragma once
#include "core/module.hpp"

namespace c2p2::modules {

    class Rot13 : public Module {
    public:
        [[nodiscard]] std::string get_id() const override { return "rot13"; }
        [[nodiscard]] std::vector<std::string> get_supported_actions() const override {
            return {"encode", "decode"};
        }

        [[nodiscard]] std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const override;
    };

}