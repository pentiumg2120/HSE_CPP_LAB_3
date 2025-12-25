#include <exception>
#include "codec.h"
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <algorithm>

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
        smallBlock = 1;
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

std::vector<std::vector<uint8_t>> extractYCbCrPlanes(image &img)
{
    int width = img.width;
    int height = img.height;
    std::vector<std::vector<uint8_t>> planes(3, std::vector<uint8_t>(width * height));

    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            pixel p = img.pixels[i * width + j];
            pixel_YCbCr ycbcr = rgbToYCbCr(p);
            planes[0][i * width + j] = ycbcr.Y;
            planes[1][i * width + j] = ycbcr.Cb;
            planes[2][i * width + j] = ycbcr.Cr;
        }
    }

    return planes;
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

void BitWriter::writeByte(uint8_t byte)
{
    flush();
    bytes.push_back(byte);
}

void BitWriter::writeString(std::string s)
{
    for (char i : s)
    {
        writeByte(static_cast<uint8_t>(i));
    }
}

void BitWriter::writeUint16(uint16_t value)
{
    writeByte(static_cast<uint8_t>(value >> 8));
    writeByte(static_cast<uint8_t>(value & 0xFF));
}

void BitWriter::writeUint32(uint32_t value)
{
    writeByte(static_cast<uint8_t>(value >> 24));
    writeByte(static_cast<uint8_t>(value >> 16));
    writeByte(static_cast<uint8_t>(value >> 8));
    writeByte(static_cast<uint8_t>(value & 0xFF));
}

void BitWriter::writeVector(std::vector<uint8_t> &vec)
{
    flush();
    for (uint8_t byte : vec)
    {
        writeByte(byte);
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

uint8_t BitReader::readByte()
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
    {
        if (readBit())
        {
            byte |= (1 << (7 - i));
        }
    }
    return byte;
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

    uint8_t thr = sum / (bw * bh);
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

encodedChannel encodeChannel(std::vector<uint8_t> &plane, int stride, int fullW, int fullH, bool useMacro, int smallBlock, int macroBlock, int encQuality)
{
    uint32_t blockCount{};
    BitWriter sizeW, typeW, patternW;
    std::vector<uint8_t> fgVals;
    std::vector<uint8_t> bgVals;

    for (int my = 0; my < fullH; my += macroBlock)
    {
        for (int mx = 0; mx < fullW; mx += macroBlock)
        {
            if (useMacro and canUseBigBlock(&plane, stride, mx, my, macroBlock, encQuality))
            {
                auto [fg, bg] = encodeBlock(&plane, stride, mx, my, macroBlock, macroBlock, patternW);
                fgVals.push_back(fg);
                bgVals.push_back(bg);
                typeW.writeBit(1);
                sizeW.writeBit(1);
                blockCount++;
            }
            else
            {
                sizeW.writeBit(0);
                for (int by = 0; by < macroBlock; by += smallBlock)
                {
                    for (int bx = 0; bx < macroBlock; bx += smallBlock)
                    {
                        uint8_t fg, bg;
                        if (smallBlock > 1)
                        {
                            auto res = encodeBlock(&plane, stride, mx + bx, my + by, smallBlock, smallBlock, patternW);
                            fg = res.first;
                            bg = res.second;
                        }
                        else
                        {
                            uint8_t v = plane[(my + by) * stride + (mx + bx)];
                            fg = v;
                            bg = v;
                        }
                        fgVals.push_back(fg);
                        bgVals.push_back(bg);
                        typeW.writeBit(smallBlock > 1);
                        blockCount++;
                    }
                }
            }
        }
    }
    sizeW.flush();
    typeW.flush();
    patternW.flush();
    return encodedChannel{blockCount, sizeW.bytes, typeW.bytes, patternW.bytes, fgVals, bgVals};
}

std::vector<uint8_t> Encode(image img, int quality, bool bwmode)
{
    int width = img.width;
    int height = img.height;
    std::pair<int, int> blockSizes = setBlocksForQuality(quality);
    int smallBlock = blockSizes.first;
    int macroBlock = blockSizes.second;

    auto planes = extractYCbCrPlanes(img);

    uint8_t mask = 0;
    if (!bwmode)
    {
        mask |= (1 << 0); // Y
        mask |= (1 << 1); // Cb
        mask |= (1 << 2); // Cr
    }
    else
    {
        mask |= (1 << 0); // Y
    }

    BitWriter bwWriter;

    bwWriter.writeString("BABE\n");
    bwWriter.writeUint16(smallBlock);
    bwWriter.writeUint16(macroBlock);
    bwWriter.writeByte(mask);
    bwWriter.writeUint32(width);
    bwWriter.writeUint32(height);

    int fullW = (width / macroBlock) * macroBlock;
    int fullH = (height / macroBlock) * macroBlock;

    int lol = 1;
    if (!bwmode)
    {
        lol = 3;
    }
    for (int i = 0; i < lol; ++i)
    {
        encodedChannel Channel = encodeChannel(planes[i], width, fullW, fullH, macroBlock > smallBlock, smallBlock, macroBlock, quality / 20);
        bwWriter.writeUint32(Channel.blockCount);

        bwWriter.writeUint32((uint32_t)Channel.sizeBuf.size());
        bwWriter.writeVector(Channel.sizeBuf);
        bwWriter.writeUint32((uint32_t)Channel.typeBuf.size());
        bwWriter.writeVector(Channel.typeBuf);
        bwWriter.writeUint32((uint32_t)Channel.patternBuf.size());
        bwWriter.writeVector(Channel.patternBuf);
        bwWriter.writeUint32((uint32_t)Channel.fgVals.size());
        bwWriter.writeVector(Channel.fgVals);
        bwWriter.writeUint32((uint32_t)Channel.bgVals.size());
        bwWriter.writeVector(Channel.bgVals);
    }
    bwWriter.flush();
    return bwWriter.bytes;
}

image Decode(const std::vector<uint8_t> &data, bool skip_preview)
{
    const char *expected_header = "BABE\n";
    const size_t header_len = 5;

    if (data.size() < header_len)
    {
        throw std::runtime_error("File too short");
    }

    for (size_t i = 0; i < header_len; ++i)
    {
        if (data[i] != static_cast<uint8_t>(expected_header[i]))
        {
            throw std::runtime_error("Invalid file signature");
        }
    }

    size_t pos = header_len;

    if (pos + 2 > data.size())
        throw std::runtime_error("Truncated file");
    uint16_t smallBlock = (static_cast<uint16_t>(data[pos]) << 8) | data[pos + 1];
    pos += 2;

    if (pos + 2 > data.size())
        throw std::runtime_error("Truncated file");
    uint16_t macroBlock = (static_cast<uint16_t>(data[pos]) << 8) | data[pos + 1];
    pos += 2;

    if (pos >= data.size())
        throw std::runtime_error("Truncated file");
    uint8_t channel_mask = data[pos];
    pos += 1;

    if (pos + 8 > data.size())
        throw std::runtime_error("Truncated file");
    uint32_t width = (static_cast<uint32_t>(data[pos]) << 24) |
                     (static_cast<uint32_t>(data[pos + 1]) << 16) |
                     (static_cast<uint32_t>(data[pos + 2]) << 8) |
                     (static_cast<uint32_t>(data[pos + 3]));
    pos += 4;

    uint32_t height = (static_cast<uint32_t>(data[pos]) << 24) |
                      (static_cast<uint32_t>(data[pos + 1]) << 16) |
                      (static_cast<uint32_t>(data[pos + 2]) << 8) |
                      (static_cast<uint32_t>(data[pos + 3]));
    pos += 4;

    if (width == 0 || height == 0 || width > 10000 || height > 10000)
    {
        throw std::runtime_error("Invalid image dimensions");
    }

    std::vector<uint8_t> Y_plane(width * height, 0);
    std::vector<uint8_t> Cb_plane(width * height, 128);
    std::vector<uint8_t> Cr_plane(width * height, 128);

    class BitReader
    {
        const std::vector<uint8_t> &buf;
        size_t byte_idx = 0;
        uint8_t bit_idx = 0;

    public:
        BitReader(const std::vector<uint8_t> &b) : buf(b) {}

        std::pair<bool, bool> readBit()
        {
            if (byte_idx >= buf.size())
                return {false, false};
            bool bit = (buf[byte_idx] & (1 << (7 - bit_idx))) != 0;
            bit_idx++;
            if (bit_idx == 8)
            {
                bit_idx = 0;
                byte_idx++;
            }
            return {bit, true};
        }
    };

    for (int channel = 0; channel < 3; channel++)
    {
        bool has_channel = (channel == 0) || (channel_mask & (1 << channel));
        if (!has_channel)
            continue;

        if (pos + 4 > data.size())
            throw std::runtime_error("Truncated file");
        pos += 4;

        auto readChunk = [&](size_t &pos) -> std::vector<uint8_t>
        {
            if (pos + 4 > data.size())
                throw std::runtime_error("Truncated file");
            uint32_t len = (static_cast<uint32_t>(data[pos]) << 24) |
                           (static_cast<uint32_t>(data[pos + 1]) << 16) |
                           (static_cast<uint32_t>(data[pos + 2]) << 8) |
                           (static_cast<uint32_t>(data[pos + 3]));
            pos += 4;
            if (pos + len > data.size())
                throw std::runtime_error("Truncated file");
            std::vector<uint8_t> chunk(data.begin() + pos, data.begin() + pos + len);
            pos += len;
            return chunk;
        };

        std::vector<uint8_t> size_data = readChunk(pos);
        std::vector<uint8_t> type_data = readChunk(pos);
        std::vector<uint8_t> pattern_data = readChunk(pos);
        std::vector<uint8_t> fg_vals = readChunk(pos);
        std::vector<uint8_t> bg_vals = readChunk(pos);

        BitReader size_reader(size_data);
        BitReader type_reader(type_data);
        BitReader pattern_reader(pattern_data);
        size_t fg_idx = 0, bg_idx = 0;

        uint32_t full_width = (width / macroBlock) * macroBlock;
        uint32_t full_height = (height / macroBlock) * macroBlock;

        for (uint32_t block_y = 0; block_y < full_height; block_y += macroBlock)
        {
            for (uint32_t block_x = 0; block_x < full_width; block_x += macroBlock)
            {
                auto [is_big, ok] = size_reader.readBit();
                if (!ok)
                    break;

                if (is_big)
                {
                    if (fg_idx >= fg_vals.size() || bg_idx >= bg_vals.size())
                        break;
                    uint8_t fg = fg_vals[fg_idx++];
                    uint8_t bg = bg_vals[bg_idx++];

                    type_reader.readBit();

                    for (uint32_t y = 0; y < macroBlock; y++)
                    {
                        for (uint32_t x = 0; x < macroBlock; x++)
                        {
                            auto [bit, bit_ok] = pattern_reader.readBit();
                            uint8_t value = bit ? fg : bg;
                            size_t idx = (block_y + y) * width + (block_x + x);
                            if (channel == 0)
                                Y_plane[idx] = value;
                            else if (channel == 1)
                                Cb_plane[idx] = value;
                            else
                                Cr_plane[idx] = value;
                        }
                    }
                }
                else
                {
                    for (uint32_t sub_y = 0; sub_y < macroBlock; sub_y += smallBlock)
                    {
                        for (uint32_t sub_x = 0; sub_x < macroBlock; sub_x += smallBlock)
                        {
                            if (fg_idx >= fg_vals.size() || bg_idx >= bg_vals.size())
                                break;
                            uint8_t fg = fg_vals[fg_idx++];
                            uint8_t bg = bg_vals[bg_idx++];

                            auto [is_pattern, tp_ok] = type_reader.readBit();
                            bool use_pattern = tp_ok ? is_pattern : false;

                            for (uint32_t y = 0; y < smallBlock; y++)
                            {
                                for (uint32_t x = 0; x < smallBlock; x++)
                                {
                                    uint8_t value = fg;
                                    if (use_pattern)
                                    {
                                        auto [bit, bit_ok] = pattern_reader.readBit();
                                        value = bit ? fg : bg;
                                    }
                                    size_t idx = (block_y + sub_y + y) * width + (block_x + sub_x + x);
                                    if (idx < width * height)
                                    {
                                        if (channel == 0)
                                            Y_plane[idx] = value;
                                        else if (channel == 1)
                                            Cb_plane[idx] = value;
                                        else
                                            Cr_plane[idx] = value;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<pixel> result;
    result.reserve(width * height);

    for (size_t i = 0; i < width * height; i++)
    {
        int32_t Y = Y_plane[i];
        int32_t Cb = Cb_plane[i] - 128;
        int32_t Cr = Cr_plane[i] - 128;

        int32_t R = Y + ((91881 * Cr) >> 16);
        int32_t G = Y - ((22554 * Cb + 46802 * Cr) >> 16);
        int32_t B = Y + ((116130 * Cb) >> 16);

        result.push_back(pixel{(uint8_t)std::clamp(B, 0, 255),
                               (uint8_t)std::clamp(G, 0, 255),
                               (uint8_t)std::clamp(R, 0, 255),
                               255});
    }

    return image{width, height, result};
}
