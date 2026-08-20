#include "cli/parser.hpp"
#include "core/pipeline.hpp"
#include "core/registry.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace c2p2::cli {

static std::string invert_action(const std::string& action) {
    if (action == "encode") return "decode";
    if (action == "decode") return "encode";
    if (action == "encrypt") return "decrypt";
    if (action == "decrypt") return "encrypt";
    return action;
}

int run(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  Single Module:   c2p2 <module> <action> [params...] [--input-file <path> | \"text\"] [--output-file <path>]\n"
                  << "  Pipeline File:   c2p2 run --pipeline <json_path> [--reverse] [--input-file <path> | \"text\"] [--output-file <path>]\n";
        return 1;
    }

    std::string first_arg = argv[1];

    // --- MODALITÀ RUN PIPELINE DA FILE JSON ---
    if (first_arg == "run") {
        std::string pipeline_file;
        std::string input_file_path;
        std::string output_file_path;
        std::string raw_input;
        bool reverse = false;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--pipeline" && i + 1 < argc) {
                pipeline_file = argv[++i];
            } else if (arg == "--reverse") {
                reverse = true;
            } else if (arg == "--input-file" && i + 1 < argc) {
                input_file_path = argv[++i];
            } else if (arg == "--output-file" && i + 1 < argc) {
                output_file_path = argv[++i];
            } else {
                if (!raw_input.empty()) raw_input += ' ';
                raw_input += arg;
            }
        }

        if (pipeline_file.empty()) {
            std::cerr << "Error: --pipeline <path.json> is required for 'run' command.\n";
            return 1;
        }

        Pipeline pipeline = Pipeline::import_from_json(pipeline_file);

        if (reverse) {
            auto original_steps = pipeline.get_steps();
            std::reverse(original_steps.begin(), original_steps.end());
            pipeline.clear();

            for (auto& step : original_steps) {
                pipeline.add_step(
                    step.instance_id,
                    step.module,
                    invert_action(step.action),
                    step.params
                );
            }
        }

        DataBuffer in_buffer;
        if (!input_file_path.empty()) {
            if (std::ifstream f(input_file_path, std::ios::binary | std::ios::ate); f) {
                auto size = f.tellg();
                f.seekg(0, std::ios::beg);
                in_buffer.resize(size);
                f.read(reinterpret_cast<char*>(in_buffer.data()), size);
            } else {
                std::cerr << "Error: Could not open input file '" << input_file_path << "'\n";
                return 1;
            }
        } else if (!raw_input.empty()) {
            for (char c : raw_input) in_buffer.push_back(static_cast<std::byte>(c));
        }

        auto result = pipeline.run(in_buffer);
        if (!result) {
            std::cerr << "Error while executing pipeline: " << result.error().message << "\n";
            return 1;
        }

        if (!output_file_path.empty()) {
            std::ofstream out(output_file_path, std::ios::binary);
            if (out) {
                out.write(reinterpret_cast<const char*>(result->data()), result->size());
            } else {
                std::cerr << "Error: Could not write output file '" << output_file_path << "'\n";
                return 1;
            }
        } else {
            for (std::byte b : *result) {
                std::cout << static_cast<char>(b);
            }
            std::cout << "\n";
        }
        return 0;
    }

    // --- MODALITÀ SINGOLO MODULO ---
    if (argc < 3) {
        std::cerr << "Error: Missing action for module '" << first_arg << "'\n";
        return 1;
    }

    std::string module_name = first_arg;
    std::string action = argv[2];

    auto mod = Registry::instance().create(module_name);
    if (!mod) {
        std::cerr << "Error: Module '" << module_name << "' not found.\n";
        return 1;
    }

    ParamsMap params;
    DataBuffer in_buffer;
    std::string output_file_path;
    std::string raw_input;

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--input-file" && i + 1 < argc) {
            if (std::ifstream f(argv[++i], std::ios::binary | std::ios::ate); f) {
                auto size = f.tellg();
                f.seekg(0, std::ios::beg);
                in_buffer.resize(size);
                f.read(reinterpret_cast<char*>(in_buffer.data()), size);
            } else {
                std::cerr << "Error: Could not open input file '" << argv[i] << "'\n";
                return 1;
            }
        }
        else if (arg == "--output-file" && i + 1 < argc) {
            output_file_path = argv[++i];
        }
        else if (arg.rfind("--", 0) == 0 && i + 1 < argc && argv[i + 1][0] != '-') {
            params[arg] = argv[++i];
        }
        else if (auto pos = arg.find('='); pos != std::string::npos) {
            std::string key = arg.substr(0, pos);
            if (key.rfind("--", 0) != 0) key = "--" + key;
            params[key] = arg.substr(pos + 1);
        }
        else {
            if (!raw_input.empty()) raw_input += ' ';
            raw_input += arg;
        }
    }

    if (in_buffer.empty() && !raw_input.empty()) {
        for (char c : raw_input) in_buffer.push_back(static_cast<std::byte>(c));
    }

    Pipeline pipeline;
    pipeline.add_step("cli_step", mod, action, params);

    auto result = pipeline.run(in_buffer);
    if (!result) {
        std::cerr << "Error while executing: " << result.error().message << "\n";
        return 1;
    }

    if (!output_file_path.empty()) {
        std::ofstream out(output_file_path, std::ios::binary);
        if (out) {
            out.write(reinterpret_cast<const char*>(result->data()), result->size());
        } else {
            std::cerr << "Error: Could not write to output file '" << output_file_path << "'\n";
            return 1;
        }
    } else {
        for (std::byte b : *result) {
            std::cout << static_cast<char>(b);
        }
        std::cout << "\n";
    }

    return 0;
}

}