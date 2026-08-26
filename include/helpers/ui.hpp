#pragma once

namespace c2p2::helpers {
    static std::string invert_action(const std::string& action) { //for revers
        if (action == "compress") return "decompress";
        if (action == "decompress") return "compress";
        if (action == "encode") return "decode";
        if (action == "decode") return "encode";
        if (action == "encrypt") return "decrypt";
        if (action == "decrypt") return "encrypt";
        return action;
    }
}