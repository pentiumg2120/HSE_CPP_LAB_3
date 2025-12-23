#include <fstream>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include "bmp.h"

std::vector<pixel> LoadBMP(std::string path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        throw std::invalid_argument("Invalid file path: " + path);
    }

    char header[14];
    file.read(header, 14);

    if (header[0] != 'B' and header[1] != 'M')
    {
        throw std::invalid_argument("Missgender file: " + path);
    }

    uint32_t offset = *reinterpret_cast<uint32_t *>(&header[10]);

    for (int i = 0; i < 14; i++)
    {
        std::cout << (char)header[i];
    }
    char buf_4_bytes[4];
    file.read(buf_4_bytes, 4);
    uint32_t DIB_header_size = *reinterpret_cast<unsigned long *>(buf_4_bytes);
    std::vector<char> DIB_header(DIB_header_size - 4);
    file.read(reinterpret_cast<char *>(DIB_header.data()), DIB_header_size - 4);
    uint32_t width = *reinterpret_cast<uint32_t *>(&DIB_header[0]);
    int32_t height = *reinterpret_cast<int32_t *>(&DIB_header[4]);
    uint32_t bpp = *reinterpret_cast<uint32_t *>(&DIB_header[10]);
    int8_t flag = 1;
    if (bpp != 32)
    {
        throw std::invalid_argument("Only 32 bpp bmp files supported");
    }

    //
    if (height < 0)
    {
        height = -height;
        flag = 0;
    }

    file.seekg(offset);

    std::vector<pixel> pixels(width * height);
    pixel buff_px;
    for (uint32_t i = 0; i < height; i++)
    {
        for (uint32_t j = 0; j < width; j++)
        {

            file.read(reinterpret_cast<char *>(&buff_px), sizeof(pixel));
            if (flag == 1)
            {
                pixels[(height - 1 - i) * width + j] = buff_px;
            }
            else
            {
                pixels[i * width + j] = buff_px;
            }
        }
    }
    file.close();

    // std::cout << std::endl;
    // std::cout << "Offset: " << offset << std::endl;
    // std::cout << "Width: " << width << " Height: " << height << " BPP: " << bpp << std::endl;

    return pixels;
}

int main()
{
    std::string path = "1.bmp";
    LoadBMP(path);
}