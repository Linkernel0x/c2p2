#include "core/pipeline.hpp"
#include <algorithm>

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