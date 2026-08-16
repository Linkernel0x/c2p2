#include "core/registry.hpp"

namespace c2p2 {

    Registry& Registry::instance() {
        static Registry reg;
        return reg;
    }

    void Registry::register_module(const std::string& id, Creator creator) {
        creators_[id] = std::move(creator);
    }

    std::shared_ptr<Module> Registry::create(const std::string& id) {
        if (creators_.contains(id)) {
            return creators_[id]();
        }
        return nullptr;
    }

}