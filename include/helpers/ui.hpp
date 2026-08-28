#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>

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

    static bool read_file(const std::string& path, DataBuffer& buffer) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Could not open file for reading: " << path << std::endl;
            return false;
        }

        buffer.resize(std::filesystem::file_size(path));
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size()); //so the compiler is happy :)

        return true;
    }

    static bool write_file(const std::string& path, const DataBuffer& buffer) {
        std::ofstream file(path, std::ios::binary);

        if (!file) {
            std::cerr << "Error: Could not open file for writing: " << path << std::endl;
            return false;
        }

        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        return true;
    }
}