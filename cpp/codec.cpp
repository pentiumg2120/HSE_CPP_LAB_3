#include <exception>
#include "codec.h"
#include <cstdint>
#include <vector>
#include <stdexcept>

std::pair<int, int> setBlocksForQuality(int quality)
{
    if (quality < 0 or quality > 100)
    {
        throw std::invalid_argument("Quality must be between 0 and 100");
    }
    int smallBlock, macroBlock;
    quality = quality / 20;
    if (quality >= 4)
    {
        smallBlock = 0;
        macroBlock = 1;
    }
    else if (quality == 3)
    {
        smallBlock = 1;
        macroBlock = 3;
    }

    else if (quality == 2)
    {
        smallBlock = 2;
        macroBlock = 4;
    }
    else if (quality == 1)
    {
        smallBlock = 3;
        macroBlock = 6;
    }
    else
    {
        smallBlock = 4;
        macroBlock = 8;
    }

    return {smallBlock, macroBlock};
}

pixel_YCbCr rgbToYCbCr(pixel p)
{
    int64_t rr = (int64_t)p.red;
    int64_t gg = (int64_t)p.green;
    int64_t bb = (int64_t)p.blue;

    int64_t Y = (77 * rr + 150 * gg + 29 * bb) >> 8;
    int64_t Cb = (((-43 * rr - 85 * gg + 128 * bb)) >> 8) + 128;
    int64_t Cr = (((128 * rr - 107 * gg - 21 * bb)) >> 8) + 128;

    return pixel_YCbCr{static_cast<uint8_t>(Y),
                       static_cast<uint8_t>(Cb),
                       static_cast<uint8_t>(Cr)};
}

std::pair<std::vector<std::vector<uint8_t>>, std::pair<int, int>> extractYCbCrPlanes(const std::vector<pixel> &img, int width, int height)
{
    std::vector<std::vector<uint8_t>> planes(3, std::vector<uint8_t>(width * height));

    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            pixel p = img[i * width + j];
            pixel_YCbCr ycbcr = rgbToYCbCr(p);
            planes[0][i * width + j] = ycbcr.Y;
            planes[1][i * width + j] = ycbcr.Cb;
            planes[2][i * width + j] = ycbcr.Cr;
        }
    }

    return {planes, {width, height}};
}

bool canUseBigBlock(std::vector<uint8_t> *plane, int stride, int x0, int y0, int macroBlock, int encQuality)
{
    uint8_t minV, maxV;

    for (int yy = 0; yy < macroBlock; ++yy)
    {
        for (int xx = 0; xx < macroBlock; ++xx)
        {
            int v = (*plane)[(y0 + yy) * stride + (x0 + xx)];
            if (xx == 0 and yy == 0)
                minV = v, maxV = v;
            else
            {
                if (v < minV)
                    minV = v;
                if (v > maxV)
                    maxV = v;
            }
        }
    }
    return maxV - minV < 64 - (encQuality / 20) * 12;
}

void BitWriter::writeBit(bool bit)
{
    current_byte <<= 1;
    if (bit)
    {
        current_byte |= 1;
    }
    n++;

    if (n == 8)
    {
        bytes.push_back(current_byte);
        current_byte = 0;
        n = 0;
    }
}

void BitWriter::flush()
{
    if (n > 0)
    {
        current_byte <<= (8 - n);
        bytes.push_back(current_byte);
        current_byte = 0;
        n = 0;
    }
}

bool BitReader::readBit()
{
    if (byte_idx >= bytes.size())
        return false;

    bool bit = (bytes[byte_idx] & (1 << (7 - bit_idx))) != 0;

    bit_idx++;
    if (bit_idx == 8)
    {
        bit_idx = 0;
        byte_idx++;
    }
    return bit;
}

std::pair<uint8_t, uint8_t> encodeBlock(std::vector<uint8_t> *plane, int stride, int x0, int y0, int bw, int bh, BitWriter &bwWriter)
{
    uint64_t sum{};
    for (int yy = 0; yy < bh; ++yy)
    {
        for (int xx = 0; xx < bw; ++xx)
        {
            sum += (*plane)[(y0 + yy) * stride + (x0 + xx)];
        }
    }

    uint8_t thr = sum / bw * bh;
    uint64_t fgsum{}, bgsum{};
    uint32_t fgcnt{}, bgcnt{};

    for (int yy = 0; yy < bh; ++yy)
    {
        for (int xx = 0; xx < bw; ++xx)
        {
            uint8_t v = (*plane)[(y0 + yy) * stride + (x0 + xx)];
            bool isFg = v >= thr;
            bwWriter.writeBit(isFg);

            if (isFg)
            {
                fgsum += v;
                fgcnt++;
            }
            else
            {
                bgsum += v;
                bgcnt++;
            }
        }
    }
    if (fgcnt == 0 or bgcnt == 0)
        return {thr, thr};
    return {(uint8_t)fgsum / fgcnt, (uint8_t)bgsum / bgcnt};
}
