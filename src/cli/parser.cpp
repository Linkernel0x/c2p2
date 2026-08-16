#include "cli/parser.hpp"
#include "core/pipeline.hpp"
#include "core/registry.hpp"
#include <iostream>
#include <fstream>

namespace c2p2::cli {

int run(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: c2p2 <module> <action> [params...] [--input-file <path> | \"text\"] [--output-file <path>]\n";
        return 1;
    }

    std::string module_name = argv[1];
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
        else if (auto pos = arg.find('='); pos != std::string::npos) {
            params[arg.substr(0, pos)] = arg.substr(pos + 1);
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
            std::cerr << "Error: Could not open output file '" << output_file_path << "'\n";
            return 1;
        }
    } else {
        for (std::byte b : *result) std::cout << static_cast<char>(b);
        std::cout << "\n";
    }

    return 0;
}

}