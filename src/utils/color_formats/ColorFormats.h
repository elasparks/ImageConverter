#ifndef COLORFORMATS_H
#define COLORFORMATS_H


#include <cstdint>

enum ColorChannel{
    RED,
    GREEN,
    BLUE,
    ALPHA,
};

#pragma pack(push, 1)
class RGB {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    bool operator==(const RGB &color) const {
        return this->red == color.red &&
               this->green == color.green &&
               this->blue == color.blue;
    }

    bool operator!=(const RGB &color) const {
        return this->red != color.red ||
               this->green != color.green ||
               this->blue != color.blue;
    }


    bool operator<(const RGB &color) const
    {
        uint32_t first = 0, second = 0;
        first = ((((first | this->red) << 8) | this->green) << 8) | this->blue;
        second = ((((second | color.red) << 8) | color.green) << 8) | color.blue;
        return first < second;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)

class RGBA {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    bool operator==(const RGBA &color) const {
        return this->red == color.red &&
               this->green == color.green &&
               this->blue == color.blue &&
               this->alpha == color.alpha;
    }

    bool operator!=(const RGBA &color) const {
        return !(*this == color);
    }

    bool operator<(const RGBA &color) const
    {
        uint32_t first = 0, second = 0;
        first = ((((((first | this->red) << 8) | this->green) << 8) | this->blue) << 8) | this->alpha;
        second = ((((((second | color.red) << 8) | color.green) << 8) | color.blue) << 8) | color.alpha;
        return first < second;
    }

    static bool compare(const RGBA &color1, const RGBA &color2, ColorChannel channel) {
        if (channel == ColorChannel::RED) return color1.red < color2.red;
        if (channel == ColorChannel::GREEN) return color1.green < color2.green;
        if (channel == ColorChannel::BLUE) return color1.blue < color2.blue;
        return color1.alpha < color2.alpha;
    }
};

#pragma pack(pop)


#endif
