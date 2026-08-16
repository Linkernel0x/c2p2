#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <expected>
#include <unordered_map>

namespace c2p2 {

    using DataBuffer = std::vector<std::byte>;
    using ParamsMap = std::unordered_map<std::string, std::string>;

    struct ModuleError {
        std::string message;
    };

    class Module {
    public:
        virtual ~Module() = default;

        [[nodiscard]] virtual std::string get_id() const = 0;
        [[nodiscard]] virtual std::vector<std::string> get_supported_actions() const = 0;

        [[nodiscard]] virtual std::expected<DataBuffer, ModuleError> execute(
            const std::string& action,
            const DataBuffer& input,
            const ParamsMap& params
        ) const = 0;
    };

}