#ifndef BMP_H
#define BMP_H
#include <cstdint>
#include <vector>
#include <string>
struct pixel
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};

std::vector<pixel> LoadBMP(std::string path);

#endif