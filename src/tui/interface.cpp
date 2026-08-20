#include "tui/interface.hpp"
#include "core/pipeline.hpp"
#include "core/registry.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <format>
#include <fstream>
#include <algorithm>
#include <cctype>

namespace c2p2::tui {

using namespace ftxui;

static std::string buffer_to_safe_string(const DataBuffer& buffer) {
    std::string safe_str;
    safe_str.reserve(buffer.size());
    for (std::byte b : buffer) {
        auto c = static_cast<unsigned char>(b);
        if (std::isprint(c) || c == '\n' || c == '\t' || c == '\r') {
            safe_str += static_cast<char>(c);
        } else {
            safe_str += '.';
        }
    }
    return safe_str;
}

static DataBuffer read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) return {};

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    DataBuffer buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

static bool write_file(const std::string& filepath, const DataBuffer& buffer) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    return true;
}

static std::string invert_action(const std::string& action) {
    if (action == "encode") return "decode";
    if (action == "decode") return "encode";
    if (action == "encrypt") return "decrypt";
    if (action == "decrypt") return "encrypt";
    return action;
}


int run() {
    auto screen = ScreenInteractive::Fullscreen();

    Pipeline pipeline;

    std::vector<std::string> pipeline_entries;
    int selected_pipeline_index = 0;
    std::string command_input;
    std::string input_text = "Welcome to c2p2! To begin run 'help' for a list of commands and ' list' to see available modules.";
    std::string output_text;

    auto append_buffer_from_text = [](const std::string& text) {
        DataBuffer buffer;
        buffer.reserve(text.size());
        for (unsigned char c : text) {
            buffer.push_back(static_cast<std::byte>(c));
        }
        return buffer;
    };

    auto refresh_pipeline_menu = [&]() {
        pipeline_entries.clear();
        const auto& steps = pipeline.get_steps();
        for (const auto& step : steps) {
            std::string line = std::format("[{}] {} {}", step.instance_id, step.module->get_id(), step.action);
            pipeline_entries.emplace_back(std::move(line));
        }
        if (pipeline_entries.empty()) {
            pipeline_entries.push_back("< Empty Pipeline >");
        }
    };

    refresh_pipeline_menu();

    auto execute_pipeline = [&]() {
        if (pipeline.get_steps().empty()) {
            output_text = input_text;
            return;
        }

        auto in_buffer = append_buffer_from_text(input_text);

        auto res = pipeline.run(in_buffer);
        if (res) {
            output_text = buffer_to_safe_string(*res);
        } else {
            output_text = "Error: " + res.error().message;
        }
    };

    auto sync_pipeline_ui = [&]() {
        refresh_pipeline_menu();
        if (selected_pipeline_index >= static_cast<int>(pipeline_entries.size())) {
            selected_pipeline_index = std::max(0, static_cast<int>(pipeline_entries.size()) - 1);
        }
        execute_pipeline();
    };

    MenuOption menu_option;
    auto pipeline_menu = Menu(&pipeline_entries, &selected_pipeline_index, menu_option);

    InputOption input_box_option;
    input_box_option.on_change = [&] {
        execute_pipeline();
    };
    auto input_box = Input(&input_text, "Type input text here...", input_box_option);

    InputOption command_option;
    command_option.on_enter = [&] {
        if (command_input.empty()) return;

        std::stringstream ss(command_input);
        std::string cmd;
        ss >> cmd;

        if (cmd == "add") {
            std::string id, module_name, action;
            if (ss >> id >> module_name >> action && !action.empty()) {
                auto mod = Registry::instance().create(module_name);
                if (mod) {
                    ParamsMap params;
                    std::string key_val;
                    while (ss >> key_val) {
                        auto pos = key_val.find('=');
                        if (pos != std::string::npos) {
                            params[key_val.substr(0, pos)] = key_val.substr(pos + 1);
                        }
                    }

                    auto valid_actions = mod->get_supported_actions();
                    bool action_valid = std::find(valid_actions.begin(), valid_actions.end(), action) != valid_actions.end();

                    if (!action_valid) {
                        output_text = std::format("Error: Action '{}' not valid for module '{}'!", action, module_name);
                        command_input.clear();
                        return;
                    }

                    for (const auto& step : pipeline.get_steps()) {
                        if (step.instance_id == id) {
                            output_text = "Error: ID already exists!";
                            command_input.clear();
                            return;
                        }
                    }
                    pipeline.add_step(id, mod, action, params);
                    sync_pipeline_ui();
                } else {
                    output_text = "Error: Module not found!";
                }
            } else {
                output_text = "Error: Usage: add <id> <module> <action> [key=value ...]";
            }
        }
        else if (cmd == "remove") {
            std::string id;
            if (ss >> id) {
                if (!pipeline.remove_step(id)) {
                    output_text = "Error: step not found!";
                }
                sync_pipeline_ui();
            } else {
                output_text = "Error: Usage: remove <id>";
            }
        }
        else if (cmd == "list") {
            output_text = "Modules:\n";
            const auto& modules = Registry::instance().get_all();
            if (modules.empty()) {
                output_text += "<no modules registered>\n";
            }
            for (const auto& [id, _] : modules) {
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
        }
        else if (cmd == "clear") {
            pipeline.clear();
            sync_pipeline_ui();
        }
        else if (cmd == "run")
        {
            std::string token;
            DataBuffer in_buffer;
            std::string output_file_path;
            std::string inline_input;

            while (ss >> token) {
                if (token == "--input-file") {
                    std::string path;
                    if (ss >> path) in_buffer = read_file(path);
                } else if (token == "--output-file") {
                    if (!(ss >> output_file_path)) {
                        output_text = "Error: missing output file path";
                        command_input.clear();
                        return;
                    }
                } else {
                    if (!inline_input.empty()) inline_input += ' ';
                    inline_input += token;
                }
            }

            if (in_buffer.empty()) {
                std::string text_to_use = inline_input.empty() ? input_text : inline_input;
                in_buffer = append_buffer_from_text(text_to_use);
            }

            auto res = pipeline.run(in_buffer);
            if (res) {
                if (!output_file_path.empty()) {
                    if (write_file(output_file_path, *res)) {
                        output_text = "Result successfully saved in " + output_file_path;
                    } else {
                        output_text = "Error while saving file!";
                    }
                } else {
                    output_text = buffer_to_safe_string(*res);
                }
            } else {
                output_text = "Error: " + res.error().message;
            }
        } else if (cmd == "export") {
            std::string path;
            if (ss >> path) {
                pipeline.export_to_json(path);
                output_text = "Pipeline exported to " + path;
            } else {
                output_text = "Error: Please provide a file path for export.";
            }
        } else if (cmd == "import") {
            std::string path;
            if (ss >> path) {
                pipeline = Pipeline::import_from_json(path);
                sync_pipeline_ui();
                output_text = "Pipeline imported from " + path;
            } else {
                output_text = "Error: Please provide a file path for import.";
            }
        } else if (cmd == "reverse") {
            auto steps = pipeline.get_steps();
            std::reverse(steps.begin(), steps.end());
            pipeline.clear();
            for (auto& s : steps) {
                pipeline.add_step(s.instance_id, s.module, invert_action(s.action), s.params);
            }
            sync_pipeline_ui();
        } else if (cmd == "help") {
            std::string module;
            if (ss >> module) {
                auto mod = Registry::instance().create(module);
                if (mod) {
                    output_text = buffer_to_safe_string(mod->help_text());
                } else {
                    output_text = "Error: unknown module";
                }
            } else {
                output_text = "Available commands:\n"
                              "- add <id> <module> <action> [params...] -> Add a new module to the pipeline\n"
                              "- remove <id> -> Remove a module from the pipeline\n"
                              "- list -> List all modules in the pipeline\n"
                              "- clear -> Clear the entire pipeline\n"
                              "- run [--input-file <path>] [--output-file <path>] [\"text\"] -> Run the pipeline with specified input\n"
                              "- export <path> -> Export the pipeline to a JSON file\n"
                              "- import <path> -> Import a pipeline from a JSON file\n"
                              "- help [module] -> Show help message";
            }
        } else {
            output_text = " Unknown command: " + cmd;
        }

        command_input.clear();
    };

    auto command_box = Input(&command_input, " >", command_option);

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

        return window(text(" c2p2 "), vbox({ main_area, bottom_area }) | flex);
    });

    execute_pipeline();

    screen.Loop(main_renderer);
    return 0;
}

}