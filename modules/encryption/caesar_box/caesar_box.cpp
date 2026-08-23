#include "caesar_box.hpp"
#include <string>
#include <format>

namespace c2p2::modules {
    static DataBuffer resolve_box(const DataBuffer& input, const size_t length_cols, const size_t length_rows) {
        DataBuffer output;
        output.reserve(input.size());

        for (size_t col = 0; col < length_cols; ++col) {
            for (size_t row = 0; row < length_rows; ++row) {
                if (const size_t index = row * length_cols + col; index < input.size()) {
                    output.push_back(input[index]);
                }
            }
        }

        return output;
    }

    std::expected<DataBuffer, ModuleError> CaesarBox::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        int length = 5;
        const auto orig_cols = static_cast<size_t>(length);
        const size_t orig_rows = (input.size() + orig_cols - 1) / orig_cols;

        if (auto it = params.find("--length"); it != params.end()) {
            try {
                length = std::stoi(it->second);
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid 'length' parameter (must be an integer)"});
            }
        }
        if (length <= 0) {
            return std::unexpected(ModuleError{.message = "Parameter 'length' must be greater than 0"});
        }

        if (action == "encrypt") {
            output = resolve_box(input, orig_cols, orig_rows);
        }

        if (action == "decrypt") {
            output = resolve_box(input, orig_rows, orig_cols);
        }

        return output;
    }

}