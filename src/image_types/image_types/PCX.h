#ifndef PCX_H
#define PCX_H

#include <vector>
#include <cinttypes>
#include <cstring>
#include <stdexcept>
#include <color_formats/ColorFormats.h>


class PCX {
public:
#pragma pack(push, 1)
    struct PCXHeader {
        uint8_t manufacturer;
        uint8_t version;
        uint8_t encoding;
        uint8_t bitsPerPixel;
        uint16_t xMin;
        uint16_t yMin;
        uint16_t xMax;
        uint16_t yMax;
        uint16_t horizonDPI;
        uint16_t verticalDPI;
        RGB palette[16];
        uint8_t reserved;
        uint8_t colorPlanes;
        uint16_t bytesPerLine;
        uint16_t paletteType;
        uint16_t HScreenSize;
        uint16_t VScreenSize;
        uint8_t filler[54];
    };
#pragma pack(pop)
    static const int PCX_HEADER_SIZE = 128;
private:
    PCXHeader header;
    std::vector<RGB> optionalPalette;
    std::vector<std::vector<RGBA>> pixels;
    uint16_t width{};
    uint16_t height{};

    void fillPCXHeader(const std::vector<char> &bytes) {
        uint8_t fileType;
        memcpy(&fileType, &bytes[0], sizeof(uint8_t));
        if (fileType != 0x0A)
            throw std::runtime_error("Error: is not PCX file!");
        memcpy(&header, &bytes[0], PCX_HEADER_SIZE);
    }

    void fillOptionalPalette(const std::vector<char> &bytes) {
        if (this->header.bitsPerPixel == 8 && this->header.colorPlanes == 1 && *(bytes.rbegin() + 256 * 3) == 12) {
            optionalPalette = std::vector<RGB>(256);
            memcpy(&optionalPalette[0], &bytes[bytes.size() - 256 * 3], optionalPalette.size() * 3);
        }
    }

    std::vector<uint8_t> decodeImageData(const std::vector<char> &bytes) {
        this->height = header.yMax - header.yMin + 1;
        this->width = header.xMax - header.xMin + 1;
        this->pixels = std::vector<std::vector<RGBA>>(this->height);
        uint32_t totalBytes = this->header.colorPlanes * this->header.bytesPerLine;
        std::vector<uint8_t> decompressedData(totalBytes * this->height);
        uint32_t compressedIndex = PCX_HEADER_SIZE;
        uint32_t decompressedIndex = 0;
        while (decompressedIndex < decompressedData.size()) {
            uint8_t repeat = 1;
            if ((bytes[compressedIndex] & 0xC0) == 0xC0) {
                repeat = bytes[compressedIndex] & 0x3F;
                ++compressedIndex;
            }
            for (int i = 0; i < repeat; ++i, ++decompressedIndex) {
                if (decompressedIndex >= decompressedData.size())
                    throw std::out_of_range("File corrupted! RLE out of range!");
                decompressedData[decompressedIndex] = bytes[compressedIndex];
            }
            ++compressedIndex;
        }
        return decompressedData;
    }

    void fillPixels(const std::vector<uint8_t> &decompressedData) {
        uint32_t bitWidth = this->width * this->header.bitsPerPixel;
        uint32_t totalBytes = this->header.colorPlanes * this->header.bytesPerLine;
        for (uint16_t row = 0; row < this->height; ++row) {
            this->pixels[row] = std::vector<RGBA>(this->width);
            for (uint8_t plane = 0; plane < this->header.colorPlanes; ++plane) {
                for (uint32_t offset = 0; offset < bitWidth; offset += this->header.bitsPerPixel) {
                    uint8_t pixelByte = decompressedData[row * totalBytes + plane * this->header.bytesPerLine +
                                                         offset / 8];
                    uint8_t pixelData = (pixelByte >> (8 - ((offset % 8) + this->header.bitsPerPixel))) &
                                        ((1 << this->header.bitsPerPixel) - 1);
                    if (this->header.colorPlanes == 1) {
                        RGBA pixel{};
                        if ((this->header.bitsPerPixel == 1) ||
                            (this->header.bitsPerPixel == 8 && this->optionalPalette.empty()))
                            pixel.red = pixel.green = pixel.blue = pixelData;
                        else if (this->header.bitsPerPixel == 8) {
                            pixel.red = this->optionalPalette[pixelData].red;
                            pixel.green = this->optionalPalette[pixelData].green;
                            pixel.blue = this->optionalPalette[pixelData].blue;
                        } else if (this->header.bitsPerPixel == 4) {
                            pixel.red = this->header.palette[pixelData].red;
                            pixel.green = this->header.palette[pixelData].green;
                            pixel.blue = this->header.palette[pixelData].blue;
                        } else
                            throw std::runtime_error("Unsupported format!");
                        pixels[row][offset / this->header.bitsPerPixel] = pixel;
                    } else if (this->header.colorPlanes == 3 || this->header.colorPlanes == 4) {
                        if (this->header.bitsPerPixel == 4 || this->header.bitsPerPixel == 8) {
                            switch (plane){
                                case 0:
                                    pixels[row][offset / this->header.bitsPerPixel].red = pixelData;
                                    break;
                                case 1:
                                    pixels[row][offset / this->header.bitsPerPixel].green = pixelData;
                                    break;
                                case 2:
                                    pixels[row][offset / this->header.bitsPerPixel].blue = pixelData;
                                    break;
                                case 3:
                                    pixels[row][offset / this->header.bitsPerPixel].alpha = pixelData;
                                    break;
                                default:
                                    throw std::runtime_error("Unsupported format!");
                            }
                        } else
                            throw std::runtime_error("Unsupported format!");
                    }
                }
            }
        }
    }

public:
    explicit PCX(const std::vector<char> &bytes) : header({}) {
        fillPCXHeader(bytes);
        fillOptionalPalette(bytes);
        fillPixels(decodeImageData(bytes));
    }

    [[nodiscard]] const std::vector<std::vector<RGBA>> &getPixels() const {
        return pixels;
    }

    [[nodiscard]] const PCXHeader &getHeader() const {
        return header;
    }

    [[nodiscard]] uint16_t getWidth() const {
        return width;
    }

    [[nodiscard]] uint16_t getHeight() const {
        return height;
    }
};

#endif