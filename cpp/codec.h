#ifndef CODEC_H
#define CODEC_H

#include "bmp.h"
#include <vector>
#include <cstdint>
#include <string>

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
    void writeByte(uint8_t byte);
    void writeString(std::string s);
    void writeUint16(uint16_t value);
    void writeUint32(uint32_t value);
    void writeVector(std::vector<uint8_t> &vec);
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
    uint8_t readByte();

    ~BitReader() = default;
};

std::pair<int, int> setBlocksForQuality(int quality);
pixel_YCbCr rgbToYCbCr(pixel p);
std::vector<std::vector<uint8_t>> extractYCbCrPlanes(const std::vector<pixel> &img, int width, int height);
bool canUseBigBlock(std::vector<uint8_t> *plane, int stride, int x0, int y0, int macroBlock, int encQuality);
std::pair<uint8_t, uint8_t> encodeBlock(std::vector<uint8_t> *plane, int stride, int x0, int y0, int bw, int bh, BitWriter &bwWriter);

struct encodedChannel
{
    int blockCount;
    std::vector<uint8_t> sizeBuf;
    std::vector<uint8_t> typeBuf;
    std::vector<uint8_t> patternBuf;
    std::vector<uint8_t> fgVals;
    std::vector<uint8_t> bgVals;
};
encodedChannel encodeChannel(std::vector<uint8_t> &plane, int stride, int w4, int h4, int fullW, int fullH, bool useMacro, int smallBlock, int macroBlock, int encQuality);
std::vector<uint8_t> Encode(image img, int quality, bool bwmode);
image Decode(const std::vector<uint8_t> &data, bool skip_preview);
#endif