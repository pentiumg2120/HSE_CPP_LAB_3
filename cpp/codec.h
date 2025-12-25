#ifndef CODEC_H
#define CODEC_H

#include "bmp.h"
#include <vector>
#include <cstdint>

struct pixel_YCbCr
{
    uint8_t Y;
    uint8_t Cb;
    uint8_t Cr;
};

struct pixel_Y
{
    uint8_t Y;
};

class BitWriter
{
public:
    std::vector<uint8_t> bytes;
    uint8_t current_byte = 0;
    uint8_t n = 0;
    BitWriter() = default;
    void writeBit(bool bit);

    void flush();

    ~BitWriter() = default;
};

class BitReader
{
public:
    std::vector<uint8_t> bytes;
    size_t byte_idx = 0;
    uint8_t bit_idx = 0;

    BitReader(const std::vector<uint8_t> &d)
    {
        bytes = d;
    }

    bool readBit();

    ~BitReader() = default;
};

std::pair<int, int> setBlocksForQuality(int quality);
pixel_YCbCr rgbToYCbCr(pixel p);
std::pair<std::vector<std::vector<uint8_t>>, std::pair<int, int>> extractYCbCrPlanes(const std::vector<pixel> &img, int width, int height);
bool canUseBigBlock(std::vector<uint8_t> *plane, int stride, int x0, int y0, int macroBlock, int encQuality);
std::pair<uint8_t, uint8_t> encodeBlock(std::vector<uint8_t> *plane, int stride, int x0, int y0, int bw, int bh, BitWriter &bwWriter);
std::vector<uint8_t> Encode(std::vector<pixel>, int quality, bool bw);
std::vector<pixel> Decode(const std::vector<uint8_t> &data, bool skip_preview);
#endif