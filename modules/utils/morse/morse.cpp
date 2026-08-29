#include "morse.hpp"
#include "helpers/ui.hpp"
#include <cctype>
#include <sstream>
#include <cmath>
#include <fstream>
#include <vector>

namespace c2p2::modules {

    static const std::unordered_map<char, std::string_view> MORSE_MAP = {
        {'A', ".- "},   {'B', "-... "}, {'C', "-.-. "}, {'D', "-.. "},  {'E', ". "},
        {'F', "..-. "}, {'G', "--. "},  {'H', ".... "}, {'I', ".. "},   {'J', ".--- "},
        {'K', "-.- "},  {'L', ".-.. "}, {'M', "-- "},   {'N', "-. "},   {'O', "--- "},
        {'P', ".--. "}, {'Q', "--.- "}, {'R', ".-. "},  {'S', "... "},  {'T', "- "},
        {'U', "..- "},  {'V', "...- "}, {'W', ".-- "},  {'X', "-..- "}, {'Y', "-.-- "},
        {'Z', "--.. "}, {'0', "----- "},{'1', ".---- "},{'2', "..--- "},{'3', "...-- "},
        {'4', "....- "},{'5', "..... "},{'6', "-.... "},{'7', "--... "},{'8', "---.. "},
        {'9', "----. "}, {'.', ".-.-.- "}, {',', "--..-- "}, {'?', "..--.. "}
    };

    static const std::unordered_map<std::string, char>& get_decode_map() {
        static std::unordered_map<std::string, char> decode_map;
        if (decode_map.empty()) {
            for (const auto& [ch, code] : MORSE_MAP) {
                std::string clean_code(code);
                if (clean_code.ends_with(' ')) {
                    clean_code.pop_back();
                }
                decode_map[clean_code] = ch;
            }
        }
        return decode_map;
    }

    enum class MorseStyle { STANDARD, PRETTY, BITFIELD, RAW };
    struct AudioParams { std::string path; int wpm = 20; int frequency = 700; };

    static std::string format_symbol(char symbol, const MorseStyle style) {
        switch (style) {
        case MorseStyle::PRETTY:
            if (symbol == '.') return "•";
            if (symbol == '-') return "–";
            if (symbol == '/') return "/";
            if (symbol == ' ') return " ";
            break;
        case MorseStyle::BITFIELD:
            if (symbol == '.') return "01";
            if (symbol == '-') return "11";
            if (symbol == '/') return "10";
            if (symbol == ' ') return "00";
            break;
        case MorseStyle::RAW:
            if (symbol == '.') return "10";
            if (symbol == '-') return "1110";
            if (symbol == ' ') return "00";
            if (symbol == '/') return "00";
            break;
        default:
            break;
        }
        return {1, symbol};
    }

    static std::string encode_morse(const std::string& input, const MorseStyle style) {
        std::string standard;
        for (const char i : input) {
            const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(i)));
            if (upper == ' ') {
                standard += "/ ";
            } else if (MORSE_MAP.contains(upper)) {
                standard += MORSE_MAP.at(upper);
            }
        }

        if (!standard.empty() && standard.back() == ' ') {
            standard.pop_back();
        }

        if (style == MorseStyle::STANDARD) return standard;

        std::string output;
        for (const char c : standard) {
            output += format_symbol(c, style);
        }

        return output;
    }

    static std::string decode_morse(const std::string& input) {
        const auto& decode_map = get_decode_map();
        std::string output;
        std::istringstream stream(input);
        std::string token;

        while (stream >> token) {
            if (token == "/") {
                output += ' ';
            } else if (decode_map.contains(token)) {
                output += decode_map.at(token);
            }
        }
        return output;
    }

    static DataBuffer create_wav_header(const std::vector<int16_t>& samples, const int sample_rate = 44100) {
        const auto data_size = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
        const uint32_t chunk_size = 36 + data_size;
        const uint32_t byte_rate = sample_rate * 1 * sizeof(int16_t);

        DataBuffer wav;
        wav.reserve(44 + data_size);

        auto append_bytes = [&wav](auto value, const size_t size) {
            for (size_t i = 0; i < size; ++i) {
                wav.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
            }
        };

        //header riff
        wav.push_back(std::byte{'R'});
        wav.push_back(std::byte{'I'});
        wav.push_back(std::byte{'F'});
        wav.push_back(std::byte{'F'});
        append_bytes(chunk_size, 4);
        wav.push_back(std::byte{'W'});
        wav.push_back(std::byte{'A'});
        wav.push_back(std::byte{'V'});
        wav.push_back(std::byte{'E'});
        // Subchunk "fmt "
        wav.push_back(std::byte{'f'});
        wav.push_back(std::byte{'m'});
        wav.push_back(std::byte{'t'});
        wav.push_back(std::byte{' '});
        append_bytes(16, 4);
        append_bytes(1, 2);
        append_bytes(1, 2);
        append_bytes(sample_rate, 4);
        append_bytes(byte_rate, 4);
        append_bytes(2, 2);
        append_bytes(16, 2);
        // Subchunk "data"
        wav.push_back(std::byte{'d'});
        wav.push_back(std::byte{'a'});
        wav.push_back(std::byte{'t'});
        wav.push_back(std::byte{'a'});
        append_bytes(data_size, 4);

        for (int16_t sample : samples) {
            wav.push_back(static_cast<std::byte>(sample & 0xFF));
            wav.push_back(static_cast<std::byte>((sample >> 8) & 0xFF));
        }
        return wav;
    }

    static DataBuffer generate_morse_audio(const std::string& morse_code, const AudioParams& audio_params) {
        const int dot_duration_ms = 1200 / audio_params.wpm;
        constexpr int sample_rate = 44100;

        std::vector<int16_t> audio_samples;

        auto append_tone = [&](const int duration_ms) {
            const int total_samples = (sample_rate * duration_ms) / 1000;
            for (int i = 0; i < total_samples; ++i) {
                const double t = static_cast<double>(i) / sample_rate;
                const double sample = std::sin(2.0 * M_PI * audio_params.frequency * t);
                audio_samples.push_back(static_cast<int16_t>(sample * 32767.0 * 0.5));
            }
        };

        auto append_silence = [&](const int duration_ms) {
            const int total_samples = (sample_rate * duration_ms) / 1000;
            audio_samples.insert(audio_samples.end(), total_samples, 0);
        };

        for (const char c : morse_code) {
            if (c == '.') {
                append_tone(dot_duration_ms);
                append_silence(dot_duration_ms);
            } else if (c == '-') {
                append_tone(dot_duration_ms * 3);
                append_silence(dot_duration_ms);
            } else if (c == ' ' || c == '/') {
                append_silence(dot_duration_ms * 2);
            }
        }

        return create_wav_header(audio_samples, sample_rate);
    }

    std::expected<DataBuffer, ModuleError> Morse::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        const std::string input_str = databuffer_to_string(input);
        AudioParams audio_params;
        auto style = MorseStyle::STANDARD;
        DataBuffer output;

        if (const auto it = params.find("--audio-output"); it != params.end()) {
            audio_params.path = it->second;
            if (const auto it2 = params.find("--wpm"); it2 != params.end()) {
                try {
                    const int wpm = std::stoi(it2->second);
                    audio_params.wpm = (wpm > 0) ? wpm : 20;
                } catch (...) {
                    return std::unexpected(ModuleError{.message = "Invalid '--wpm' parameter"});
                }
            }
            if (const auto it2 = params.find("--frequency"); it2 != params.end()) {
                try {
                    const int frequency = std::stoi(it2->second);
                    if (frequency <= 0) throw std::invalid_argument("Invalid frequency");
                    audio_params.frequency = frequency;
                } catch (...) {
                    return std::unexpected(ModuleError{.message = "Invalid '--frequency' parameter"});
                }
            }
            if (action != "encode") {
                return std::unexpected(ModuleError{.message = "At the moment audio is only supported for 'encode' action"});
            }
        }
        if (const auto it = params.find("--format"); it != params.end()) {
            if (it->second == "pretty") {
                style = MorseStyle::PRETTY;
            } else if (it->second == "bitfield") {
                style = MorseStyle::BITFIELD;
            } else if (it->second == "raw") {
                style = MorseStyle::RAW;
            }
            if (action != "encode") {
                return std::unexpected(ModuleError{.message = "At the moment Styling is only supported for 'encode' action"});
            }
        }

        if (action == "encode") {
            const std::string standard_morse = encode_morse(input_str, MorseStyle::STANDARD);
            const std::string formatted_output = (style == MorseStyle::STANDARD)
                ? standard_morse
                : encode_morse(input_str, style);

            output = string_to_databuffer(formatted_output);

            if (!audio_params.path.empty()) {
                std::thread([standard_morse, audio_params]() {
                    DataBuffer wav_data = generate_morse_audio(standard_morse, audio_params);
                    helpers::write_file(audio_params.path, wav_data);
                }).detach();
            }
        }

        if (action == "decode") {
            output = string_to_databuffer(decode_morse(input_str));
        }

        return output;
    }

}