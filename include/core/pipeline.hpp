#pragma once
#include "module.hpp"
#include <memory>
#include <vector>
#include <string>
#include <expected>

namespace c2p2 {

    struct PipelineStep {
        std::string instance_id;
        std::shared_ptr<Module> module;
        std::string action;
        ParamsMap params;
    };

    class Pipeline {
    public:
        Pipeline() = default;

        void add_step(const std::string& instance_id, std::shared_ptr<Module> module, std::string action, ParamsMap params = {});

        bool remove_step(const std::string& instance_id);

        void clear();

        void export_to_json(const std::string& file_path) const;

        static Pipeline import_from_json(const std::string& file_path) ;

        [[nodiscard]] const std::vector<PipelineStep>& get_steps() const;

        [[nodiscard]] std::expected<DataBuffer, ModuleError> run(const DataBuffer& initial_input) const;

    private:
        std::vector<PipelineStep> steps_;
    };

}