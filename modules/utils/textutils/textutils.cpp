#include "textutils.hpp"
#include <algorithm>
#include <random>
#include <cctype>

namespace c2p2::modules {

    std::expected<DataBuffer, ModuleError> Textutils::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        if (action != "transform") {
            return std::unexpected(ModuleError{.message = "Unknown action: " + action});
        }

        std::string text = databuffer_to_string(input);

        if (const auto it = params.find("--replace"); it != params.end()) {
            const auto& value = it->second;
            auto pos = value.find(':');
            if (pos == std::string::npos) {
                return std::unexpected(ModuleError{.message = "Invalid '--replace' format. Use 'old:new'"});
            }
            std::string old_str = value.substr(0, pos);
            std::string new_str = value.substr(pos + 1);

            if (!old_str.empty()) {
                size_t start_pos = 0;
                while ((start_pos = text.find(old_str, start_pos)) != std::string::npos) {
                    text.replace(start_pos, old_str.length(), new_str);
                    start_pos += new_str.length();
                }
            }
        }

        if (const auto it = params.find("--remove"); it != params.end()) {
            const auto& value = it->second;
            if (value == "whitespace") {
                std::erase_if(text, [](const unsigned char c) { return std::isspace(c); });
            } else if (value == "punctuation") {
                std::erase_if(text, [](const unsigned char c) { return std::ispunct(c); });
            } else if (value == "digits") {
                std::erase_if(text, [](const unsigned char c) { return std::isdigit(c); });
            } else if (value == "letters") {
                std::erase_if(text, [](const unsigned char c) { return std::isalpha(c); });
            } else {
                return std::unexpected(ModuleError{.message = "Invalid '--remove' parameter"});
            }
        }

        if (const auto it = params.find("--case"); it != params.end()) {
            const auto& value = it->second;
            if (value == "lower") {
                for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            } else if (value == "upper") {
                for (char& c : text) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            } else if (value == "title") {
                bool new_word = true;
                for (char& c : text) {
                    if (std::isspace(static_cast<unsigned char>(c))) {
                        new_word = true;
                    } else {
                        c = new_word ? static_cast<char>(std::toupper(static_cast<unsigned char>(c)))
                                     : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                        new_word = false;
                    }
                }
            } else if (value == "swap") {
                for (char& c : text) {
                    if (std::islower(static_cast<unsigned char>(c))) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                    else if (std::isupper(static_cast<unsigned char>(c))) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            } else if (value == "snake_case") {
                std::string result;
                for (char c : text) {
                    if (std::isspace(static_cast<unsigned char>(c))) result += '_';
                    else result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                text = std::move(result);
            } else if (value == "kebab-case") {
                std::string result;
                for (char c : text) {
                    if (std::isspace(static_cast<unsigned char>(c))) result += '-';
                    else result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                text = std::move(result);
            } else {
                return std::unexpected(ModuleError{.message = "Invalid '--case' parameter"});
            }
        }

        if (const auto it = params.find("--order"); it != params.end()) {
            const auto& value = it->second;
            if (value == "reverse") {
                std::ranges::reverse(text);
            } else if (value == "sort") {
                std::ranges::sort(text);
            } else if (value == "sort_desc") {
                std::ranges::sort(text, std::greater<char>());
            } else if (value == "mirror") {
                std::string rev = text;
                std::ranges::reverse(rev);
                text += rev;
            } else if (value == "shuffle") {
                std::random_device rd;
                std::mt19937 g(rd());
                std::ranges::shuffle(text, g);
            } else {
                return std::unexpected(ModuleError{.message = "Invalid '--order' parameter"});
            }
        }

        if (const auto it = params.find("--trim"); it != params.end()) {
            const auto& value = it->second;
            auto is_not_space = [](unsigned char c) { return !std::isspace(c); };
            if (value == "left" || value == "both") {
                text.erase(text.begin(), std::find_if(text.begin(), text.end(), is_not_space));
            }
            if (value == "right" || value == "both") {
                text.erase(std::find_if(text.rbegin(), text.rend(), is_not_space).base(), text.end());
            }
        }

        if (const auto it = params.find("--truncate"); it != params.end()) {
            const auto& value = it->second;
            try {
                if (value.empty() || value.front() == '-') throw std::invalid_argument("Negative value");
                size_t len = std::stoul(value);
                if (len < text.size()) text.resize(len);
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid '--truncate' parameter"});
            }
        }

        if (const auto it = params.find("--pad"); it != params.end()) {
            const auto& value = it->second;
            try {
                if (value.empty() || value.front() == '-') throw std::invalid_argument("Negative value");
                size_t len = std::stoul(value);
                if (len > text.size()) text.append(len - text.size(), ' ');
            } catch (...) {
                return std::unexpected(ModuleError{.message = "Invalid '--pad' parameter"});
            }
        }

        return string_to_databuffer(text);
    }

}