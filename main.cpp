#include <iostream>
#include "core/registry.hpp"
#include "cli/parser.hpp"
#include "tui/interface.hpp"

#include "encoding/rot13/rot13.hpp"
#include "encoding/base64/base64.hpp"
#include "encoding/hex/hex.hpp"
#include "encoding/htmlEntities/htmlEntities.hpp"
#include "encoding/url/url.hpp"

#include "encryption/caesar/caesar.hpp"


static void register_all_modules() {
    auto& reg = c2p2::Registry::instance();
    reg.register_module("rot13", []() { return std::make_shared<c2p2::modules::Rot13>(); });
    reg.register_module("base64", []() { return std::make_shared<c2p2::modules::Base64>(); });
    reg.register_module("hex", []() { return std::make_shared<c2p2::modules::Hex>(); });
    reg.register_module("html-entities", []() { return std::make_shared<c2p2::modules::HtmlEntities>(); });
    reg.register_module("url", []() { return std::make_shared<c2p2::modules::Url>(); });

    reg.register_module("caesar", []() { return std::make_shared<c2p2::modules::Caesar>(); });
}

int main(const int argc, char* argv[]) {
    register_all_modules();

    if (argc > 1) {
        return c2p2::cli::run(argc, argv);
    }
    return c2p2::tui::run();
}
