#include "terminal.h"
#include <cstdio>
#include <algorithm>

void renderToTerminal(const std::vector<pixel> &img, int width, int height)
{
    int termW = 400, termH = 200;
    // getTerminalSize(termW, termH); in future

    double scaleW = static_cast<double>(termW) / width;
    double scaleH = static_cast<double>(termH * 2) / height;
    double scale = std::min(scaleW, scaleH);
    if (scale > 1.0)
    {
        scale = 1.0;
    }

    int newW = static_cast<int>(width * scale);
    int newH = static_cast<int>(height * scale);
    if (newH % 2 != 0)
    {
        newH--;
    }
    for (int y = 0; y < newH; y += 2)
    {
        for (int x = 0; x < newW; x++)
        {
            int srcX = static_cast<int>(x / scale);
            int srcY1 = static_cast<int>(y / scale);
            int srcY2 = static_cast<int>((y + 1) / scale);

            uint8_t r1, g1, b1, r2, g2, b2;
            getPixel(img, srcX, srcY1, width, height, r1, g1, b1);
            getPixel(img, srcX, srcY2, width, height, r2, g2, b2);

            std::printf("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm▀",
                        r1, g1, b1, r2, g2, b2);
        }
        std::printf("\x1b[0m\n");
    }
}

void getPixel(std::vector<pixel> img, int x, int y, int width, int height, uint8_t &r, uint8_t &g, uint8_t &b)
{
    pixel p = img[y * width + x];
    r = p.red;
    g = p.green;
    b = p.blue;
}