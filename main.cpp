#include <iostream>
#include "core/registry.hpp"
#include "cli/parser.hpp"
#include "tui/interface.hpp"

#include "encoding/base64/base64.hpp"
#include "encoding/hex/hex.hpp"
#include "encoding/binary/binary.hpp"
#include "encoding/html/html.hpp"
#include "encoding/url/url.hpp"
#include "encryption/aes/aes.hpp"

#include "encryption/rot13/rot13.hpp"
#include "encryption/caesar_box/caesar_box.hpp"
#include "encryption/enigma/enigma.hpp"
#include "encryption/rot47/rot47.hpp"
#include "encryption/rot8000/rot8000.hpp"
#include "encryption/vigenere/vigenere.hpp"

#include "files/archive/archive.hpp"
#include "files/exif/exif.hpp"
#include "hashing/md/md.hpp"
#include "hashing/sha/sha.hpp"


static void register_all_modules() {
    auto& reg = c2p2::Registry::instance();
    reg.register_module("caesar_box", []() { return std::make_shared<c2p2::modules::CaesarBox>(); });
    reg.register_module("vigenere", []() { return std::make_shared<c2p2::modules::Vigenere>(); });
    reg.register_module("rot13", []() { return std::make_shared<c2p2::modules::Rot13>(); });
    reg.register_module("rot47", []() { return std::make_shared<c2p2::modules::Rot47>(); });
    reg.register_module("rot8000", []() { return std::make_shared<c2p2::modules::Rot8000>(); });
    reg.register_module("enigma", []() { return std::make_shared<c2p2::modules::Enigma>(); });
    reg.register_module("aes", []() { return std::make_shared<c2p2::modules::Aes>(); });

    reg.register_module("md", []() { return std::make_shared<c2p2::modules::Md>(); });
    reg.register_module("sha", []() { return std::make_shared<c2p2::modules::Sha>(); });

    reg.register_module("archive", []() { return std::make_shared<c2p2::modules::Archive>(); });

    reg.register_module("base64", []() { return std::make_shared<c2p2::modules::Base64>(); });
    reg.register_module("hex", []() { return std::make_shared<c2p2::modules::Hex>(); });
    reg.register_module("binary", []() { return std::make_shared<c2p2::modules::Binary>(); });
    reg.register_module("html", []() { return std::make_shared<c2p2::modules::HtmlEntities>(); });
    reg.register_module("url", []() { return std::make_shared<c2p2::modules::Url>(); });
}

int main(const int argc, char* argv[]) {
    register_all_modules();

    if (argc > 1) {
        return c2p2::cli::run(argc, argv);
    }
    return c2p2::tui::run();
}
