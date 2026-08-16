#pragma once
#include "module.hpp"
#include <memory>
#include <unordered_map>
#include <functional>

namespace c2p2
{
    class Registry {
    public:
        using Creator = std::function<std::shared_ptr<Module>()>;

        static Registry& instance();

        void register_module(const std::string& id, Creator creator);
        std::shared_ptr<Module> create(const std::string& id);

        [[nodiscard]] const std::unordered_map<std::string, Creator>& get_all() const {
            return creators_;
        }

    private:
        Registry() = default;
        std::unordered_map<std::string, Creator> creators_;
    };
}