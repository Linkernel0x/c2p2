#include "cli/parser.hpp"
#include "core/pipeline.hpp"
#include "core/registry.hpp"
#include "helpers/ui.hpp"

namespace c2p2::cli {

    struct ParsedCommand {
        std::string module_name;
        std::string action;
        ParamsMap params;
        std::string input_file_path;
        std::string output_file_path;
        bool pipeline = false;
        std::string pipeline_path;
        bool reverse = false;
        std::string inline_input;
        bool malformed = false;
    };

    static int execute_module(const std::string& module_name, const std::string& action, const DataBuffer& input, const ParamsMap& params, DataBuffer& output) {
        const auto module = Registry::instance().create(module_name);
        if (!module) {
            std::cerr << "Error: Could not create module: " << module_name << std::endl;
            return 1;
        }

        auto pipeline = Pipeline();
        pipeline.add_step("temp", module, action, params);
        auto result = pipeline.run(input);

        if (!result) {
            std::cerr << "Error: " << result.error().message << std::endl;
            return 1;
        }
        output = result.value();

        return 0;
    }

    static int execute_pipeline(const std::string& path, const bool reverse, const DataBuffer& input, DataBuffer& output) {
        auto pipeline = Pipeline::import_from_json(path);

        if (reverse) {
            auto steps = pipeline.get_steps();
            std::reverse(steps.begin(), steps.end());
            pipeline.clear();
            for (auto& s : steps) {
                pipeline.add_step(s.instance_id, s.module, helpers::invert_action(s.action), s.params);
            }
        }

        auto result = pipeline.run(input);
        if (!result) {
            std::cerr << "Error: " << result.error().message << std::endl;
            return 1;
        }
        output = result.value();

        return 0;
    }

    static ParsedCommand parse_command(char* argv[], int argc) {
        ParsedCommand cmd;
        cmd.malformed = false;

        if (argc < 2) {
            cmd.malformed = true;
            return cmd;
        }

        std::string first_arg = argv[1];
        if (first_arg == "run") {
            cmd.pipeline = true;
            if (argc < 3) {
                cmd.malformed = true;
                return cmd;
            }

            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--input-file" && i + 1 < argc) {
                    cmd.input_file_path = argv[i + 1];
                    i += 1;
                } else if (arg == "--output-file" && i + 1 < argc) {
                    cmd.output_file_path = argv[i + 1];
                    i += 1;
                } else if (arg == "--pipeline" && i + 1 < argc) {
                    cmd.pipeline_path = argv[i + 1];
                    i += 1;
                } else if (arg == "--reverse") {
                    cmd.reverse = true;
                } else {
                    if (!cmd.inline_input.empty()) cmd.inline_input += ' ';
                    cmd.inline_input += arg;
                }
            }

            if (cmd.pipeline_path.empty()) {
                cmd.malformed = true;
                return cmd;
            }
        } else {
            cmd.module_name = argv[1];

            if (argc < 3) {
                cmd.malformed = true;
                return cmd;
            }

            cmd.action = argv[2];
            for (int i = 3; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--input-file" && i + 1 < argc) {
                    cmd.input_file_path = argv[i + 1];
                    i += 1;
                } else if (arg == "--output-file" && i + 1 < argc) {
                    cmd.output_file_path = argv[i + 1];
                    i += 1;
                } else if (arg.starts_with("--")) {
                    const std::string& key = arg;
                    std::string value;
                    if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("--")) {
                        value = argv[i + 1];
                        i += 1;
                    }
                    cmd.params[key] = value;
                } else {
                    if (!cmd.inline_input.empty()) cmd.inline_input += ' ';
                    cmd.inline_input += arg;
                }
            }
        }

        return cmd;
    }

    int run(const int argc, char* argv[]) {
        const ParsedCommand cmd = parse_command(argv, argc);
        DataBuffer input;

        if (cmd.malformed) {
            std::cerr << "Usage:" << std::endl
                    << "c2p2  -> open TUI" << std::endl
                    << "c2p2 <module> <action> [params...] [--input-file | 'input text'] [--output-file] [--reverse]  -> execute a module" << std::endl
                    << "c2p2 run <pipeline_path>  -> execute a pipeline." << std::endl;
            return 1;
        }

        if (!cmd.input_file_path.empty()) {
            helpers::read_file(cmd.input_file_path, input);
        } else if (!cmd.inline_input.empty()) {
            input = Module::string_to_databuffer(cmd.inline_input);
        }

        DataBuffer output;
        int code;
        std::string first_arg = argv[1];
        if (first_arg == "run") {
            code = execute_pipeline(cmd.pipeline_path, cmd.reverse, input, output);
        } else {
            code = execute_module(cmd.module_name, cmd.action, input, cmd.params, output);
        }

        if (code == 0) {
            if (!cmd.output_file_path.empty()) {
                helpers::write_file(cmd.output_file_path, output);
            } else {
                std::cout << Module::databuffer_to_string(output) << std::endl;
            }
        }

        return 0;
    }

}