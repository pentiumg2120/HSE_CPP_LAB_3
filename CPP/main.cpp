#include "babe.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cctype>
#include <algorithm>
#include <cstdlib>


int main (int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: babe <input.bmp> [quality] [bw] OR babe <input.babe> -t\n";
        return 1;
    }
    std::string inP = argv[1];
    std::string ext = toLower(std::filesystem::path(inP).extension().string());
    
    if (ext == ".babe") {
        if (args > 2 && std::string(argv[2]) == "-t") {
            std::ifstream f(inP, std::ios::binary);
            std::vector<uint8_t> d((std::istreambuf_iterator<char>(f)),
                                    std::istreambuf_iterator<char>());
        Image img = Decode(d, false);
        renderToTerminal(img);
        }
        return 0;
    }
    int q = 70;
    if (argc >= 3) {
        char* end;
        long v = std::strtol(argv[2], &end, 10);
        if (*end == '\0') q = static_cast<int>(v);
    }

    bool bw = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "bw") bw = true;
    }

    Image img = loadBMP(inP);
    std::vector<uint8_t> enc = Encode(img, q, bw);
    std::string outP = std::filesystem::path(inP).replace_extension(".babe").string();
    std::ofstream f(outP, std::ios::binary);
    f.write(reinterpret_cast<const char*>(enc.data()), enc.size());

    std::cout << inP << "->" << outP << " (q=" << q << ")\n";
    return 0;
}