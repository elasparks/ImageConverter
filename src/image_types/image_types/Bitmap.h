#ifndef BITMAP_H
#define BITMAP_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <color_formats/ColorFormats.h>

class Bitmap {
public:
#pragma pack(push, 1)
    struct BitmapFileHeader {
        uint16_t fileType;
        uint32_t fileSize;
        uint16_t reservedOne;
        uint16_t reservedTwo;
        uint32_t offsetToImageData;
    };
#pragma pack(pop)
#pragma pack(push, 1)
    struct BitmapInfoHeader {
        uint32_t size;
        int32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bitCount;
        uint32_t compression;
        uint32_t imageSize;
        int32_t xPixelPerMeter;
        int32_t yPixelPerMeter;
        uint32_t colorsInColorTable;
        uint32_t importantColorCount;
    };
#pragma pack(pop)
#pragma pack(push, 1)
    struct RGBQuad {
        uint8_t rgbBlue;
        uint8_t rgbGreen;
        uint8_t rgbRed;
        uint8_t rgbReserved;
    };
#pragma pack(pop)
#pragma pack(push, 1)
    struct RGBTriple {
        uint8_t rgbBlue;
        uint8_t rgbGreen;
        uint8_t rgbRed;
    };
#pragma pack(pop)
    static const int BITMAP_FILE_HEADER_SIZE = 14;
    static const int BITMAP_INFO_HEADER_SIZE = 40;
    static const int BITMAP_RGBTRIPLE_SIZE = 3;
    static const int BITMAP_RGBQUAD_SIZE = 4;
private:
    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;
    int32_t width{};
    int32_t height{};
    std::vector<RGBQuad> palette;
    std::vector<std::vector<RGBA>> pixels;

private:
    void fillFileHeader(const std::vector<char> &bytes) {
        uint16_t fileType;
        memcpy(&fileType, &bytes[0], sizeof(uint16_t));
        if (fileType != 0x4D42)
            throw std::runtime_error("Error: is not BMP file!");
        memcpy(&fileHeader, &bytes[0], BITMAP_FILE_HEADER_SIZE);
    }

    void fillInfoHeader(const std::vector<char> &bytes) {
        uint32_t actualHeaderSize;
        memcpy(&actualHeaderSize, &bytes[BITMAP_FILE_HEADER_SIZE], sizeof(uint32_t));
        if (actualHeaderSize == BITMAP_INFO_HEADER_SIZE) {
            memcpy(&infoHeader, &bytes[BITMAP_FILE_HEADER_SIZE], BITMAP_INFO_HEADER_SIZE);
        } else {
            throw std::runtime_error("Error: BitmapInfoHeader is corrupted or version is not supported!");
        }
    }

    void fillPalette(const std::vector<char> &bytes) {
        uint32_t offsetToPalette = BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE;
        uint32_t colorTableSize = (fileHeader.offsetToImageData - offsetToPalette) / BITMAP_RGBQUAD_SIZE;
        for (uint32_t i = 0; i < colorTableSize; ++i) {
            RGBQuad color{};
            memcpy(&color, &bytes[offsetToPalette + i * BITMAP_RGBQUAD_SIZE], BITMAP_RGBQUAD_SIZE);
            this->palette.push_back(color);
        }
    }

    void fillPixels(const std::vector<char> &bytes) {
        this->width = this->infoHeader.width;
        this->height = this->infoHeader.height;
        this->pixels = std::vector<std::vector<RGBA>>(this->height);
        auto imageData = std::vector(bytes.begin() + this->fileHeader.offsetToImageData, bytes.end());
        uint32_t bitWidth = this->width * this->infoHeader.bitCount;
        uint32_t bytesPerLine = (((bitWidth + 31) / 32) * 4);
        for (uint32_t row = 0; row < this->height;++row){
            this->pixels[row] = std::vector<RGBA>(this->width);
            for (uint32_t offset = 0, column=0; offset < bitWidth;++column){
                uint32_t pixelData = 0;
                for (uint32_t bitLeft = this->infoHeader.bitCount, bitsRead; bitLeft != 0; bitLeft -= bitsRead){
                    uint8_t buffer = 0;
                    uint8_t offsetBit = offset%8;
                    bitsRead = std::min(uint32_t(8 - offsetBit), bitLeft);
                    buffer = imageData[row*bytesPerLine+offset/8];
                    if (offsetBit)
                        buffer = char(buffer & ((1 << bitsRead) - 1));
                    if ((offset%8 + bitLeft) < 8)
                        buffer = char(buffer >> (8 - (offsetBit + bitLeft)));
                    pixelData = (pixelData << bitsRead) | (uint8_t)buffer;
                    offset += bitsRead;
                }
                if (this->infoHeader.bitCount == 8){
                    this->pixels[row][column].red=this->palette[pixelData].rgbRed;
                    this->pixels[row][column].green=this->palette[pixelData].rgbGreen;
                    this->pixels[row][column].blue=this->palette[pixelData].rgbBlue;
                } else
                    throw std::runtime_error("Unsupported format!");
            }
        }
        std::reverse(this->pixels.begin(), this->pixels.end());
    }

public:
    explicit Bitmap(const std::vector<char> &bytes) : fileHeader({}), infoHeader({}) {
        fillFileHeader(bytes);
        fillInfoHeader(bytes);
        fillPalette(bytes);
        fillPixels(bytes);
    }

    [[nodiscard]] const BitmapFileHeader &getBitmapFileHeader() const {
        return fileHeader;
    }

    [[nodiscard]] const BitmapInfoHeader &getBitmapInfoHeader() const {
        return infoHeader;
    }

    [[nodiscard]] const std::vector<RGBQuad> &getPalette() const {
        return palette;
    }

    [[nodiscard]] const std::vector<std::vector<RGBA>> &getPixels() const {
        return pixels;
    }

    [[nodiscard]] int32_t getWidth() const {
        return width;
    }

    [[nodiscard]] int32_t getHeight() const {
        return height;
    }
};

#endif