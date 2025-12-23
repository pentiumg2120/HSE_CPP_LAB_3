#ifndef CODEC_H
#define CODEC_H

std::vector<uint8_t> Encode(const Image &img, int quality, bool bw);
std::vector<pixel> Decode(const std::vector<uint8_t> &data, bool skip_preview);

#endif