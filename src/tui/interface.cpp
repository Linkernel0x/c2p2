#include "tui/interface.hpp"
#include "core/pipeline.hpp"
#include "core/registry.hpp"
#include  "helpers/ui.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <format>
#include <fstream>
#include <ranges>
#include <algorithm>
#include <cctype>

namespace c2p2::tui
{
    using namespace ftxui;

    enum CommandType {
        ADD,
        REMOVE,
        CLEAR,
        LIST,
        HELP,
        EXPORT,
        IMPORT,
        REVERSE,
        RUN
    };

    struct ParsedCommand {
        CommandType type;
        std::string id;
        std::string module_name;
        std::string action;
        ParamsMap params;
        std::string input_file_path;
        std::string output_file_path;
        std::string pipeline_path;
        bool reverse = false;
        std::string inline_input;
        bool malformed = false;
        std::string error_message;
    };

    static std::string buffer_to_safe_string(const DataBuffer& buffer) {
        std::string result;
        for (std::byte b : buffer ) {
            if (char c = static_cast<char>(b); std::isprint (static_cast<unsigned char>(c))) {
                result += c;
            } else {
                result += '.';
            }
        }
        return result;
    }

    const std::string HELP_MESSAGE = "Available commands:\n"
                                  "- add <id> <module> <action> [params...] -> Add a new module to the pipeline\n"
                                  "- remove <id> -> Remove a module from the pipeline\n"
                                  "- list -> List all modules in the pipeline\n"
                                  "- clear -> Clear the entire pipeline\n"
                                  "- run [--input-file <path>] [--output-file <path>] [\"text\"] -> Run the pipeline with specified input\n"
                                  "- export <path> -> Export the pipeline to a JSON file\n"
                                  "- import <path> -> Import a pipeline from a JSON file\n"
                                  "- help [module] -> Show help message";

    static void parse_params(std::istringstream& stream, ParamsMap& params) {
        auto extract_value = [&stream](const std::string& initial_val) -> std::string {
            std::string val = initial_val;
            if (val.starts_with('"') && !val.ends_with('"')) {
                std::string rest;
                std::getline(stream, rest, '"');
                val += rest;
            } else if (val.starts_with('"') && val.ends_with('"') && val.size() >= 2) {
                val = val.substr(1, val.size() - 2);
            }

            if (val.starts_with('"')) val.erase(0, 1);
            return val;
        };

        for (std::string token; stream >> token;) {
            if (auto pos = token.find('='); pos != std::string::npos) {
                std::string key = token.substr(0, pos);
                std::string raw_val = token.substr(pos + 1);
                params[key] = extract_value(raw_val);
            } else {
                const std::string val = extract_value(token);
                if (!params["inline-input"].empty()) params["inline-input"] += ' ';
                params["inline-input"] += val;
            }
        }
    }

    static ParsedCommand parse_command(const std::string& command_string) {
        ParsedCommand cmd;
        cmd.malformed = false;
        std::istringstream stream(command_string);
        std::string arg1;
        stream >> arg1;

        if (arg1 == "add") {
            cmd.type = ADD;

            std::string id;
            std::string name;
            std::string action;
            if (!(stream >> id >> name >> action)) {
                cmd.malformed = true;
                return cmd;
            } else {
                cmd.id = id;
                cmd.module_name = name;
                cmd.action = action;
            }

            parse_params(stream, cmd.params);

        } else if (arg1 == "run") {
            cmd.type = RUN;

            parse_params(stream, cmd.params);

            if (cmd.params.contains("--input-file")) {
                cmd.input_file_path = cmd.params["--input-file"];
                cmd.params.erase("--input-file");
            }
            if (cmd.params.contains("--output-file")) {
                cmd.output_file_path = cmd.params["--output-file"];
                cmd.params.erase("--output-file");
            }
            if (cmd.params.contains("inline-input")) {
                cmd.inline_input = cmd.params["inline-input"];
                cmd.params.erase("inline-input");
            }

        } else if (arg1 == "remove") {
            cmd.type = REMOVE;
            if (!(stream >> cmd.id)) cmd.malformed = true;

        } else if (arg1 == "clear") {
            cmd.type = CLEAR;

        } else if (arg1 == "list") {
            cmd.type = LIST;

        } else if (arg1 == "help") {
            cmd.type = HELP;
            stream >> cmd.module_name;

        } else if (arg1 == "export") {
            cmd.type = EXPORT;
            if (!(stream >> cmd.pipeline_path)) {cmd.malformed = true;}

        } else if (arg1 == "import") {
            cmd.type = IMPORT;
            if (!(stream >> cmd.pipeline_path)) {cmd.malformed = true;}

        } else if (arg1 == "reverse") {
            cmd.type = REVERSE;
            cmd.reverse = true;

        } else {
            cmd.malformed = true;
        }

        return cmd;
    }

    static void execute_command(const ParsedCommand& command, Pipeline& pipeline, std::string& output_text) {
        switch (command.type) {
            case ADD: {
                const auto mod = Registry::instance().create(command.module_name);
                if (mod == nullptr) {
                    output_text = "Error: Module '" + command.module_name + "' not found.";
                    break;
                }
                bool id_exists = false;
                for (const auto& step : pipeline.get_steps()) {
                    if (step.instance_id == command.id) {
                        output_text = "Error: Step ID '" + command.id + "' already exists.";
                        id_exists = true;
                        break;
                    }
                }
                if (!id_exists) {
                    pipeline.add_step(command.id, mod, command.action, command.params);
                }
                break;
            }
            case REMOVE: {
                if (!pipeline.remove_step(command.id)) {
                    output_text = "No step found with ID: " + command.id;
                }
                break;
            }
            case CLEAR: {
                pipeline.clear();
                break;
            }
            case LIST: {
                output_text = "Modules:\n";
                const auto& modules = Registry::instance().get_all();
                if (modules.empty()) {
                    output_text += "<no modules registered>\n";
                }
                for (const auto& id : modules | std::views::keys) {
                    output_text += "- " + id;

                    auto temp_mod = Registry::instance().create(id);
                    if (temp_mod) {
                        auto actions = temp_mod->get_supported_actions();
                        output_text += "  ·  ";
                        for (size_t i = 0; i < actions.size(); ++i) {
                            output_text += actions[i];
                            if (i + 1 < actions.size()) {
                                output_text += ", ";
                            }
                        }
                        output_text += '\n';
                    }
                }
                break;
            }
            case EXPORT: {
                pipeline.export_to_json(command.pipeline_path);
                output_text = "Pipeline exported to: " + command.pipeline_path;
                break;
            }
            case IMPORT: {
                pipeline = Pipeline::import_from_json(command.pipeline_path);
                output_text = "Pipeline imported from: " + command.pipeline_path;
                break;
            }
            case REVERSE: {
                auto steps = pipeline.get_steps();
                std::ranges::reverse(steps);
                pipeline.clear();
                for (auto& [instance_id, module, action, params] : steps) {
                    pipeline.add_step(instance_id, module, helpers::invert_action(action), params);
                }
                break;
            }
            case RUN: {
                DataBuffer input;
                if (!command.input_file_path.empty()) {
                    helpers::read_file(command.input_file_path, input);
                } else if (!command.inline_input.empty()) {
                    input = Module::string_to_databuffer(command.inline_input);
                }

                auto res = pipeline.run(input);
                if (!res) {
                    output_text = "Error: " + res.error().message;
                    break;
                }
                const DataBuffer& output = res.value();
                if (!command.output_file_path.empty()) {
                    helpers::write_file(command.output_file_path, output);
                } else {
                    output_text = buffer_to_safe_string(output);
                }
                break;
            }
            case HELP:
                if (!command.module_name.empty()) {
                    const auto mod = Registry::instance().create(command.module_name);
                    if (mod == nullptr) {
                        output_text = "Error: Module '" + command.module_name + "' not found.";
                        break;
                    }
                    output_text = Module::databuffer_to_string(mod->help_text());
                } else {
                    output_text = HELP_MESSAGE;
                }
                break;
            default:
                output_text = HELP_MESSAGE;
                break;
        }
    }

    static auto refresh_pipeline_menu(std::vector<std::string>& pipeline_entries, const Pipeline& pipeline, int& selected_pipeline_index) {
        pipeline_entries.clear();
        for (const auto& step : pipeline.get_steps()) {
            pipeline_entries.push_back(std::format("[{}] {} {}", step.instance_id, step.module->get_id(), step.action));
        }
        if (pipeline_entries.empty()) {
            pipeline_entries.emplace_back("< Empty Pipeline >");
        }

        if (selected_pipeline_index >= static_cast<int>(pipeline_entries.size())) { //to prevent out of bounds
            selected_pipeline_index = std::max(0, static_cast<int>(pipeline_entries.size()) - 1);
        }
    };

    int run() {
        auto screen = ScreenInteractive::Fullscreen();

        Pipeline pipeline;
        std::string input_text = "Welcome to c2p2! Check the documentation at https://github.com/Linkernel0x/c2p2/wiki";
        std::string output_text;
        std::string command_input;
        std::vector<std::string> pipeline_entries;
        int selected_pipeline_index = 0;

        auto update_result = [&] {
            if (pipeline.get_steps().empty()) {
                output_text = "Welcome to c2p2! Check the documentation at https://github.com/Linkernel0x/c2p2/wiki. \n\nThe pipeline is currently empty. Use the 'add' command to add modules to the pipeline, or 'import' to load a pipeline from a JSON file. \n\nIf you need help, read documentation or type 'help' for a quick list of available commands.";
                return;
            }

            if (auto res = pipeline.run(Module::string_to_databuffer(input_text)); !res) {
                output_text = "Error: " + res.error().message;
            } else {
                output_text = buffer_to_safe_string(res.value());
            }
        };

        refresh_pipeline_menu(pipeline_entries, pipeline, selected_pipeline_index);
        update_result();

        auto pipeline_menu = Menu(&pipeline_entries, &selected_pipeline_index);

        InputOption input_option;
        input_option.on_change = [&] {
            update_result();
        };

        InputOption command_option;
        command_option.on_enter = [&] {
            if (const auto parsed_command = parse_command(command_input); parsed_command.malformed) {
                output_text = "Error: Malformed command. Type 'help' for usage.";
            } else {
                std::string prev_output = output_text;
                execute_command(parsed_command, pipeline, output_text);

                if (!(parsed_command.type == RUN || parsed_command.type == HELP || parsed_command.type == LIST || parsed_command.type == EXPORT || parsed_command.type == IMPORT) && output_text == prev_output) {
                    update_result();
                }
            }
            command_input.clear();
            refresh_pipeline_menu(pipeline_entries, pipeline, selected_pipeline_index);
        };

        auto input_box = Input(&input_text, "Enter input text here...", input_option);
        auto command_box = Input(&command_input, " > Enter command...", command_option);

        auto output_renderer = Renderer([&] {
            return paragraph(output_text) | yframe | vscroll_indicator;
        });

        auto main_container = Container::Vertical({
            Container::Horizontal({
                pipeline_menu,
                Container::Vertical({
                    input_box,
                    output_renderer
                })
            }),
            command_box
        });

        auto main_renderer = Renderer(main_container, [&] {
               auto left_column = window(text(" Pipeline "), pipeline_menu->Render()) | size(WIDTH, EQUAL, 35);
               auto top_right = window(text(" Input "), input_box->Render()) | size(HEIGHT, LESS_THAN, 8);
               auto bottom_right = window(text(" Output "), output_renderer->Render()) | flex;

               auto right_column = vbox({ top_right, bottom_right }) | flex;
               auto main_area = hbox({ left_column, right_column }) | flex;
               auto bottom_area = window(text(" Command "), command_box->Render()) | size(HEIGHT, EQUAL, 3);

               return window(text(" c2p2 TUI "), vbox({ main_area, bottom_area }) | flex);
           });

        screen.Loop(main_renderer);
        return 0;
    }
}
