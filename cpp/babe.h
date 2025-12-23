#ifndef BABE_H
#define BABE_H

#include <string>
#include <vector>
#include <cstdint>

struct Image {
    // ... поля, которые у тебя есть
};

Image LoadBMP(const std::string& path);
std::vector<uint8_t> Encode(const Image& img, int quality, bool bw);
Image Decode(const std::vector<uint8_t>& data, bool skip_preview);
void renderToTerminal(const Image& img);

#endif