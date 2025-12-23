#ifndef TERMINAL_H
#define TERMINAL_H

#include "bmp.h"
#include <vector>
#include <cstdint>
#include <cstdio>

struct Image {
    int width;
    int height;
    std::vector<uint8_t> pixels;
};

bool getTerminalSize(int& cols, int& rows);
void renderToTerminal(const std::vector<pixel> &img);
void getPixel(const Image& img, int x, int y, uint8_t& r, uint8_t& g, uint8_t& b);
#endif