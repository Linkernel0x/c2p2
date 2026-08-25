#include "url.hpp"
#include <format>
#include <cstddef>
#include <ranges>
#include <string_view>

namespace c2p2::modules {

    const std::unordered_map<std::string, std::string> url_encoding_map = {
        {"!",  "%21"},
        {"#",  "%23"},
        {"$",  "%24"},
        {"&",  "%26"},
        {"\'", "%27"},
        {"(",  "%28"},
        {")",  "%29"},
        {"*",  "%2A"},
        {"+",  "%2B"},
        {",",  "%2C"},
        {"/",  "%2F"},
        {":",  "%3A"},
        {";",  "%3B"},
        {"=",  "%3D"},
        {"?",  "%3F"},
        {"@",  "%40"},
        {"[",  "%5B"},
        {"]",  "%5D"},
        {" ",  "%20"},
        {"\"", "%22"},
        {"<",  "%3C"},
        {">",  "%3E"},
        {"\\", "%5C"},
        {"^",  "%5E"},
        {"`",  "%60"},
        {"{",  "%7B"},
        {"|",  "%7C"},
        {"}",  "%7D"},
        {"~",  "%7E"}
    };

    //flip key-value pairs for encoding
    static std::unordered_map<std::string, std::string> invert_map(
    const std::unordered_map<std::string, std::string>& original_map) {
        std::unordered_map<std::string, std::string> inverted;
        inverted.reserve(original_map.size());

        for (const auto& [key, value] : original_map) {
            inverted[value] = key;
        }
        return inverted;
    }

    static DataBuffer replace_buffer(
    const DataBuffer& input,
    const std::unordered_map<std::string, std::string>& map)
    {
        // find the longest key
        size_t max_search_len = 0;
        for (const auto& val : map | std::views::keys) {
            if (val.length() > max_search_len) {
                max_search_len = val.length();
            }
        }

        DataBuffer output;
        output.reserve(input.size());

        size_t i = 0;
        while (i < input.size()) {
            bool matched = false;
            const size_t current_search_len = std::min(max_search_len, input.size() - i);

            //try matching longest potential substrings first
            for (size_t len = current_search_len; len > 0; --len) {
                std::string_view sub(reinterpret_cast<const char*>(input.data() + i), len);

                auto it = map.find(std::string(sub));
                if (it != map.end()) {
                    for (char c : it->second) {
                        output.push_back(static_cast<std::byte>(c));
                    }
                    i += len;
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                output.push_back(input[i]);
                i++;
            }
        }

        return output;
    }

    std::expected<DataBuffer, ModuleError> Url::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        const auto inverted_map = invert_map(url_encoding_map);

        DataBuffer output;
        output.reserve(input.size());

        if (action == "encode") {
            output = replace_buffer(input, url_encoding_map);
        }
        else if (action == "decode") {
            output = replace_buffer(input, inverted_map);
        }

        return output;
    }

}