#include "htmlEntities.hpp"
#include <format>
#include <cstddef> // Per std::byte
#include <string_view>
#include <unordered_map>

namespace c2p2::modules {

    const std::unordered_map<std::string, std::array<std::string, 3>> html_entities = {
        {"&",  {"&amp;", "&#38;", "&#x26;"}},
        {"<",  {"&lt;", "&#60;", "&#x3C;"}},
        {">",  {"&gt;", "&#62;", "&#x3E;"}},
        {"\"", {"&quot;", "&#34;", "&#x22;"}},
        {"'",  {"&#39;", "&#39;", "&#x27;"}},
        {"/",  {"&#x2F;", "&#x2F;", "&#x2F;"}},
        {"`",  {"&#x60;", "&#x60;", "&#x60;"}},
        {"=",  {"&#x3D;", "&#x3D;", "&#x3D;"}},
        {"©",  {"&copy;", "&#169;", "&#169;"}},
        {"®",  {"&reg;", "&#174;", "&#174;"}},
        {"™",  {"&trade;", "&#8482;", "&#8482;"}},
        {"€",  {"&euro;", "&#8364;", "&#8364;"}},
        {"£",  {"&pound;", "&#163;", "&#163;"}},
        {"¥",  {"&yen;", "&#165;", "&#165;"}},
        {"¢",  {"&cent;", "&#162;", "&#162;"}},
        {"§",  {"&sect;", "&#167;", "&#167;"}},
        {"¶",  {"&para;", "&#182;", "&#182;"}},
        {"•",  {"&bull;", "&#8226;", "&#8226;"}},
        {"…",  {"&hellip;", "&#8230;", "&#8230;"}},
        {"–",  {"&ndash;", "&#8211;", "&#8211;"}},
        {"—",  {"&mdash;", "&#8212;", "&#8212;"}},
        {"‘",  {"&lsquo;", "&#8220;", "&#8220;"}},
        {"’",  {"&rsquo;", "&#8221;", "&#8221;"}},
        {"“",  {"&ldquo;", "&#8222;", "&#8222;"}},
        {"”",  {"&rdquo;", "&#8223;", "&#8223;"}},
    };

    static const std::unordered_map<std::string, std::string>& get_decode_map() {
        static std::unordered_map<std::string, std::string> decode_map;
        if (decode_map.empty()) {
            for (const auto& [utf8_str, entities] : html_entities) {
                for (const auto& entity : entities) {
                    decode_map[entity] = utf8_str;
                }
            }
        }
        return decode_map;
    }

    std::expected<DataBuffer, ModuleError> HtmlEntities::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        DataBuffer output;

        if (action == "encode") {
            size_t fmt = 0;
            if (auto it = params.find("format"); it != params.end()) {
                try { fmt = std::stoull(it->second); } catch (...) { fmt = 0; }
                if (fmt > 2) fmt = 0;
            }

            output.reserve(input.size());

            size_t i = 0;
            while (i < input.size()) {
                bool matched = false;

                auto b = std::to_integer<uint8_t>(input[i]);

                size_t char_len = 1;
                if ((b & 0xE0) == 0xC0) char_len = 2;
                else if ((b & 0xF0) == 0xE0) char_len = 3;
                else if ((b & 0xF8) == 0xF0) char_len = 4;

                if (i + char_len <= input.size()) {
                    std::string sequence(reinterpret_cast<const char*>(input.data() + i), char_len);

                    auto it = html_entities.find(sequence);
                    if (it != html_entities.end()) {
                        const std::string& replacement = it->second[fmt];
                        for (char c : replacement) {
                            output.push_back(std::byte(c));
                        }
                        i += char_len;
                        matched = true;
                    }
                }

                if (!matched) {
                    output.push_back(input[i]);
                    i++;
                }
            }
        }
        else if (action == "decode") {
            const auto& decode_map = get_decode_map();
            output.reserve(input.size());

            size_t i = 0;
            while (i < input.size()) {
                bool matched = false;

                if (static_cast<char>(input[i]) == '&') {
                    size_t max_lookahead = std::min(input.size(), i + 10);
                    size_t semi_pos = i;

                    for (size_t j = i + 1; j < max_lookahead; ++j) {
                        if (static_cast<char>(input[j]) == ';') {
                            semi_pos = j;
                            break;
                        }
                    }

                    if (semi_pos > i) {
                        size_t entity_len = semi_pos - i + 1;
                        std::string entity(reinterpret_cast<const char*>(input.data() + i), entity_len);

                        auto it = decode_map.find(entity);
                        if (it != decode_map.end()) {
                            for (char c : it->second) {
                                output.push_back(std::byte(c));
                            }
                            i += entity_len;
                            matched = true;
                        }
                    }
                }

                if (!matched) {
                    output.push_back(input[i]);
                    i++;
                }
            }
        }

        return output;
    }

}