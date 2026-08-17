#include "vigenere.hpp"
#include <string>
#include <format>
#include <cctype>
#include <algorithm>

namespace c2p2::modules {

    static DataBuffer transform_vigenere(const DataBuffer& input, const std::vector<int>& key) {
        DataBuffer output;
        output.reserve(input.size());

        std::vector<int> shifts;
        shifts.reserve(key.size());
        for (int shift : key) {
            shifts.push_back((shift % 26 + 26) % 26);
        }

        size_t key_idx = 0;
        for (size_t i = 0; i < input.size(); ++i) {
            char c = static_cast<char>(input[i]);

            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                int current_shift = shifts[key_idx % shifts.size()];

                if (c >= 'a' && c <= 'z') {
                    c = static_cast<char>('a' + (c - 'a' + current_shift) % 26);
                } else if (c >= 'A' && c <= 'Z') {
                    c = static_cast<char>('A' + (c - 'A' + current_shift) % 26);
                }

                key_idx++;
            }

            output.push_back(static_cast<std::byte>(c));
        }

        return output;
    }

    std::expected<DataBuffer, ModuleError> Vigenere::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;
        std::string input_key;

        if (auto it = params.find("--key"); it != params.end()) {
            input_key = it->second;
        }

        if (input_key.empty()) {
            return std::unexpected(ModuleError{.message = "Vigenere error: 'key' parameter is required"});
        }

        std::vector<int> key;
        key.reserve(input_key.size());

        for (char c : input_key) {
            char lower_c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (lower_c >= 'a' && lower_c <= 'z') {
                int index = lower_c - 'a';
                key.push_back(action == "decrypt" ? -index : index);
            }
        }

        if (key.empty()) {
            return std::unexpected(ModuleError{.message = "Vigenere error: 'key' must contain at least one alphabetic character"});
        }

        if (action == "encrypt" || action == "decrypt") {
            return transform_vigenere(input, key);
        }

        return output;
    }

}