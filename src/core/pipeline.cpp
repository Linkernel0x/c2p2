#include "core/pipeline.hpp"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <core/registry.hpp>

namespace c2p2
{
    void Pipeline::add_step(const std::string& instance_id, std::shared_ptr<Module> module, std::string action, ParamsMap params) {
        if (!module) return;
        steps_.push_back(PipelineStep{
            .instance_id = instance_id,
            .module = std::move(module),
            .action = std::move(action),
            .params = std::move(params)
        });
    }

    bool Pipeline::remove_step(const std::string& instance_id) {
        auto it = std::remove_if(steps_.begin(), steps_.end(),
            [&instance_id](const PipelineStep& step) {
                return step.instance_id == instance_id;
            });

        if (it != steps_.end()) {
            steps_.erase(it, steps_.end());
            return true;
        }
        return false;
    }

    void Pipeline::clear() {
        steps_.clear();
    }

    void Pipeline::export_to_json(const std::string& file_path) const {
        nlohmann::json j;
        for (const auto& step : steps_) {
            j.push_back({
                {"instance_id", step.instance_id},
                {"module_name", step.module->get_id()},
                {"action", step.action},
                {"params", step.params}
            });
        }

        std::ofstream file(file_path);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    Pipeline Pipeline::import_from_json(const std::string& file_path) {
        Pipeline pipeline;
        std::ifstream file(file_path);
        nlohmann::json j;
        file >> j;

        for (const auto& step_json : j) {
            auto instance_id = step_json["instance_id"].get<std::string>();
            auto module_name = step_json["module_name"].get<std::string>();
            auto action = step_json["action"].get<std::string>();
            auto params = step_json["params"].get<ParamsMap>();

            auto module = Registry::instance().create(module_name);
            if (!module) continue;

            pipeline.add_step(instance_id, std::move(module), std::move(action), std::move(params));
        }

        return pipeline;
    }

    const std::vector<PipelineStep>& Pipeline::get_steps() const {
        return steps_;
    }

    std::expected<DataBuffer, ModuleError> Pipeline::run(const DataBuffer& initial_input) const {
        DataBuffer current_data = initial_input;

        for (const auto& step : steps_) {
            auto result = step.module->execute(step.action, current_data, step.params);

            if (!result) {
                return std::unexpected(result.error());
            }

            current_data = std::move(*result);
        }

        return current_data;
    }
}