#include "enigma.hpp"
#include <fstream>
#include <cctype>
#include <nlohmann/json.hpp>

namespace c2p2::modules {

    struct EnigmaRotor {
        std::string name;
        std::string wiring;
        std::string notches;
        int position = 0;
        int ring = 0;
    };

    struct EnigmaConfig {
        std::vector<EnigmaRotor> rotors;
        std::string reflector_wiring;
        std::vector<std::string> plugboard;
    };

    const std::map<std::string, std::pair<std::string, std::string>> ROTORS_DB = {
        {"I",     {"EKMFLGDQVZNTOWYHXUSPAIBRCJ", "Q"}},
        {"II",    {"AJDKSIRUXBLHWTMCQGZNPYFVOE", "E"}},
        {"III",   {"BDFHJLCPRTXVZNYEIWGAKMUSQO", "V"}},
        {"IV",    {"ESOVPZJAYQUIRHXLNFTGKDCMWB", "J"}},
        {"V",     {"VZBRGITYUPSDNHLXAWMJQOFECK", "Z"}},
        {"VI",    {"JPGVOUMFYQBENHZRDKASXLICTW", "MZ"}},
        {"VII",   {"NZJHGRCXMYSWBOUFAIVDKPLTQE", "MZ"}},
        {"VIII",  {"FKQHTLXOCBJSPDZRAMEWNIUYGV", "MZ"}},
        {"Beta",  {"LEYJVCNIXWPBQMDRTAKZGFUHOS", ""}},
        {"Gamma", {"FSOKANUERHMBTIYCWLQPZXVGJD", ""}}
    };

    const std::map<std::string, std::string> REFLECTORS_DB = {
        {"UKW-A",   "AEJMZALYXVBWFCRQUONTSPIKHGD"},
        {"UKW-B",   "YRUHQSLDPXNGOKMIEBFZCWVJAT"},
        {"UKW-C",   "FVPJIAOYEDRZXWGCTKUQSBNMHL"},
        {"UKW-B-d", "ENKQAUYWJICOPBLMDXZVFTHRGS"},
        {"UKW-C-d", "RDOBJNTKVEHMLFCWZAXGYUPFSI"}
    };

    static bool is_at_notch(const EnigmaRotor& r) {
        char current_char = static_cast<char>('A' + r.position);
        return r.notches.find(current_char) != std::string::npos;
    }

    static std::expected<EnigmaConfig, ModuleError> loadConfig(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::unexpected(ModuleError{.message = "Failed to open config file: " + path});
        }

        nlohmann::json j;
        try {
            file >> j;
            EnigmaConfig config;

            auto rotors_names = j["rotors"].get<std::vector<std::string>>();
            auto positions = j["positions"].get<std::string>();
            auto rings = j["rings"].get<std::vector<int>>();

            for (size_t i = 0; i < rotors_names.size(); ++i) {
                const std::string& name = rotors_names[i];
                if (!ROTORS_DB.contains(name)) {
                    return std::unexpected(ModuleError{.message = "Unknown rotor: " + name});
                }
                auto [wiring, notches] = ROTORS_DB.at(name);
                int pos = std::toupper(positions[i]) - 'A';
                int ring_val = (i < rings.size()) ? (rings[i] - 1) : 0;

                config.rotors.push_back(EnigmaRotor{
                    .name = name,
                    .wiring = wiring,
                    .notches = notches,
                    .position = pos,
                    .ring = ring_val
                });
            }

            std::string ref_name = j["reflector"].get<std::string>();
            if (!REFLECTORS_DB.contains(ref_name)) {
                return std::unexpected(ModuleError{.message = "Unknown reflector: " + ref_name});
            }
            config.reflector_wiring = REFLECTORS_DB.at(ref_name);
            config.plugboard = j["plugboard"].get<std::vector<std::string>>();

            return config;
        } catch (const std::exception& e) {
            return std::unexpected(ModuleError{.message = std::string("JSON parsing error: ") + e.what()});
        }
    }

    static void rotate_rotors(EnigmaConfig& config) {
        size_t n = config.rotors.size();
        if (n < 3) return;

        auto& r_left  = config.rotors[n - 3];
        auto& r_mid   = config.rotors[n - 2];
        auto& r_right = config.rotors[n - 1];

        bool mid_at_notch   = is_at_notch(r_mid);
        bool right_at_notch = is_at_notch(r_right);

        if (mid_at_notch) {
            r_left.position = (r_left.position + 1) % 26;
            r_mid.position  = (r_mid.position + 1) % 26; // Double step
        } else if (right_at_notch) {
            r_mid.position = (r_mid.position + 1) % 26;
        }

        r_right.position = (r_right.position + 1) % 26;
    }

    static char apply_plugboard(const EnigmaConfig& config, char c) {
        for (const auto& pair : config.plugboard) {
            if (pair.length() == 2) {
                if (pair[0] == c) return pair[1];
                if (pair[1] == c) return pair[0];
            }
        }
        return c;
    }

    static int forward_rotor(const EnigmaRotor& r, int input_idx) {
        int shift = r.position - r.ring;
        int in = (input_idx + shift + 26) % 26;
        char out_char = r.wiring[in];
        int out_idx = (out_char - 'A' - shift + 26) % 26;
        return out_idx;
    }

    static int backward_rotor(const EnigmaRotor& r, int input_idx) {
        int shift = r.position - r.ring;
        int in = (input_idx + shift + 26) % 26;
        char target = static_cast<char>('A' + in);
        int pos_in_wiring = static_cast<int>(r.wiring.find(target));
        int out_idx = (pos_in_wiring - shift + 26) % 26;
        return out_idx;
    }

    static char process_char(EnigmaConfig& config, char c) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        rotate_rotors(config);

        c = apply_plugboard(config, c);
        int signal = c - 'A';

        for (int i = static_cast<int>(config.rotors.size()) - 1; i >= 0; --i) {
            signal = forward_rotor(config.rotors[i], signal);
        }

        char ref_char = config.reflector_wiring[signal];
        signal = ref_char - 'A';

        for (size_t i = 0; i < config.rotors.size(); ++i) {
            signal = backward_rotor(config.rotors[i], signal);
        }

        c = static_cast<char>('A' + signal);
        c = apply_plugboard(config, c);

        return c;
    }

    std::expected<DataBuffer, ModuleError> Enigma::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        std::string config_path;

        if (const auto it = params.find("--config"); it != params.end()) {
            config_path = it->second;
        } else {
            return std::unexpected(ModuleError{.message = "'--config' parameter (file path) is required"});
        }

        auto config_res = loadConfig(config_path);
        if (!config_res) {
            return std::unexpected(config_res.error());
        }

        EnigmaConfig config = config_res.value();
        DataBuffer output;

        for (std::byte b : input) {
            char c = static_cast<char>(b);

            if (std::isalpha(static_cast<unsigned char>(c))) {
                c = process_char(config, c);
            }
            output.push_back(static_cast<std::byte>(c));
        }

        return output;
    }

}