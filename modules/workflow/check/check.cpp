#include "check.hpp"
#include <algorithm>
#include <format>
#include "helpers/ui.hpp"

namespace c2p2::modules
{
    std::expected<DataBuffer, ModuleError> Check::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        std::string value_str;
        std::string type_str;

        if (const auto it = params.find("--value"); it != params.end()) {
            value_str = it->second;
        } else {
            return std::unexpected(ModuleError{.message = "Missing required parameter: --value"});
        }

        if (const auto it = params.find("--type"); it != params.end()) {
            type_str = it->second;
        } else {
            return std::unexpected(ModuleError{.message = "Missing required parameter: --type"});
        }

        if (action == "is" || action == "is_not") {
            DataBuffer check_buffer;

            if (type_str == "string") {
                check_buffer = string_to_databuffer(value_str);
            } else if (type_str == "path") {
                if (!helpers::read_file(value_str, check_buffer)) {
                    return std::unexpected(ModuleError{.message = "Failed to read file from path: " + value_str});
                }
            } else {
                return std::unexpected(ModuleError{.message = "Invalid 'type' parameter (must be 'string'  or 'path')"});
            }

            const bool matches = (input == check_buffer);
            if ((action == "is" && matches) || (action == "is_not" && !matches)) {
                return input;
            }
        }

        else if (action == "size" || action == "size_not") {
            size_t target_value;
            try {
                target_value = std::stoull(value_str);
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid '--value' parameter (must be an integer)"});
            }

            if (type_str.size() < 2) {
                return std::unexpected(ModuleError{.message = "Invalid '--type' parameter (must have at least mode and one operator, e.g. '&>')"});
            }

            const char mode = type_str.front();
            if (mode != '&' && mode != '|') {
                return std::unexpected(ModuleError{.message = "Invalid '--type' mode (must start with '&' or '|')"});
            }

            const std::string_view operators = std::string_view(type_str).substr(1);
            if (operators.find_first_not_of("><=*") != std::string_view::npos) {
                return std::unexpected(ModuleError{.message = "Invalid operator in '--type' (only '>', '<', '=', '*' allowed)"});
            }

            const size_t current_size = input.size();
            std::vector<bool> results;
            results.reserve(operators.size());

            for (const char op : operators) {
                switch (op) {
                    case '>': results.push_back(current_size > target_value); break;
                    case '<': results.push_back(current_size < target_value); break;
                    case '=': results.push_back(current_size == target_value); break;
                    case '*': results.push_back(target_value != 0 && (current_size % target_value == 0)); break;
                    default: break;
                }
            }

            bool final_result = false;
            if (mode == '&') {
                final_result = std::ranges::all_of(results, [](bool v) { return v; });
            } else if (mode == '|') {
                final_result = std::ranges::any_of(results, [](bool v) { return v; });
            }

            if ((action == "size" && final_result) || (action == "size_not" && !final_result)) {
                return input;
            }
        } else {
            return std::unexpected(ModuleError{.message = "Unknown action: " + action});
        }

        return std::unexpected(ModuleError{.message = std::format("Check assertion failed. Check: '{}' Value type: '{}'", action, type_str)});
    }
}