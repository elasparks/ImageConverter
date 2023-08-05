#ifndef PCXFORMAT_H
#define PCXFORMAT_H

#include <algorithm>
#include <set>
#include <cmath>
#include <map>
#include <image_types/PCX.h>


class PCXFormat {
protected:
    uint8_t bitsPerPixel{};
    uint8_t colorPlanes{};
    PCX::PCXHeader headerTemplate{};

    static RGB convertRGBAToRGB(const RGBA& color){
        return RGB{color.red,color.green, color.blue};
    }

    static void validatePixelMatrix(const std::vector<std::vector<RGBA>> &rgbaPixels) {
        if (rgbaPixels.empty())
            throw std::runtime_error("Pixel matrix is empty!");
        for (uint32_t i = 1; i < rgbaPixels.size(); ++i)
            if (rgbaPixels[i].size() != rgbaPixels[i - 1].size())
                throw std::runtime_error("Pixel matrix has different row size!");
            else if (rgbaPixels[i].empty())
                throw std::runtime_error("Pixel matrix has no width!");

    }

    static ColorChannel getLongestDimension(const std::vector<RGBA> &bucket) {
        uint8_t minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue;
        minRed = maxRed = bucket[0].red;
        minGreen = maxGreen = bucket[0].green;
        minBlue = maxBlue = bucket[0].blue;

        for (const auto &color: bucket) {
            maxRed = std::max(color.red, maxRed);
            maxGreen = std::max(color.green, maxGreen);
            maxBlue = std::max(color.blue, maxBlue);
            minRed = std::min(color.red, minRed);
            minGreen = std::min(color.green, minGreen);
            minBlue = std::min(color.blue, minBlue);
        }

        double redRange = 0.299 * (maxRed - minRed);
        double greenRange = 0.587 * (maxGreen - minGreen);
        double blueRange = 0.114 * (maxBlue - minBlue);

        double maxRange = std::max({redRange, greenRange, blueRange});
        if (maxRange == redRange) return ColorChannel::RED;
        if (maxRange == greenRange) return ColorChannel::GREEN;
        return ColorChannel::BLUE;
    }

    static std::vector<std::vector<RGBA>> medianCutGetBuckets(std::vector<RGBA> colors, const uint16_t &colorsCount) {
        std::set<RGBA> colorSet(colors.begin(), colors.end());
        std::vector<RGBA> uniqueColors;
        if (colorsCount < 2)
            throw std::runtime_error("Colors can be no less than 2!");
        std::vector<std::vector<RGBA>> buckets{std::vector(colorSet.begin(), colorSet.end())};
        uint32_t currentColorsCount = 1;

        while (buckets.size() < colorsCount && colors.size() != buckets.size()) {
            std::vector<std::vector<RGBA>> newBuckets;
            for (auto &bucket: buckets) {
                if (bucket.size() > 1 && currentColorsCount < colorsCount) {
                    auto longDimension = getLongestDimension(bucket);
                    std::sort(bucket.begin(), bucket.end(), [longDimension](const RGBA &c1, const RGBA &c2) {
                        return RGBA::compare(c1, c2, longDimension);
                    });
                    uint32_t median = bucket.size() / 2;
                    newBuckets.emplace_back(bucket.begin(), bucket.begin() + median);
                    newBuckets.emplace_back(bucket.begin() + median, bucket.end());
                    ++currentColorsCount;
                } else {
                    newBuckets.emplace_back(bucket);
                }
            }
            buckets = newBuckets;
        }
        return buckets;
    }

    static auto getColorsRepetition(const std::vector<RGBA> &colors) {
        std::map<RGBA, uint32_t> repetition;
        for (const auto &color: colors)
            if (repetition.count(color) > 0)
                ++repetition[color];
            else
                repetition[color] = 1;
        return repetition;
    }

    static auto getBucketAverageColor(const std::vector<RGBA>& bucket,
                                      std::map<RGBA, uint32_t>& repetition) {
        uint32_t total = 0;
        long double red = 0, green = 0, blue = 0, alpha = 0;
        for (const auto &color: bucket)
            total += repetition[color];
        for (const auto &color: bucket) {
            red += ((long double) color.red * repetition[color]) / total;
            green += ((long double) color.green * repetition[color]) / total;
            blue += ((long double) color.blue * repetition[color]) / total;
            alpha += ((long double) color.alpha * repetition[color]) / total;
        }
        return RGBA{static_cast<uint8_t>(std::round(red)), static_cast<uint8_t>(std::round(green)),
                    static_cast<uint8_t>(std::round(blue)), static_cast<uint8_t>(std::round(alpha))};
    }

    static std::vector<RGB> medianCutGetPalette(const std::vector<RGBA> &colors, const uint16_t &colorsCount) {
        auto buckets = medianCutGetBuckets(colors, colorsCount);
        auto repetition = getColorsRepetition(colors);
        std::vector<RGB> palette;
        for (const auto &bucket: buckets) {
            RGBA resultColor = getBucketAverageColor(bucket, repetition);
            palette.push_back({resultColor.red,resultColor.green,resultColor.blue});
        }
        return palette;
    }

    static auto medianCutGetRelation(const std::vector<RGBA> &colors, const uint16_t &colorsCount) {
        auto buckets = medianCutGetBuckets(colors, colorsCount);
        auto repetition = getColorsRepetition(colors);
        std::map<RGBA, RGBA> relation;
        for (const auto &bucket: buckets) {
            RGBA resultColor = getBucketAverageColor(bucket, repetition);
            for (const auto &color: bucket)
                relation[color] = resultColor;
        }
        return relation;
    }

    PCXFormat(uint8_t bitsPerPixel, uint8_t colorPlanes) : bitsPerPixel(bitsPerPixel), colorPlanes(colorPlanes) {
        headerTemplate.manufacturer = 0x0A;
        headerTemplate.version = 5;
        headerTemplate.encoding = 1;
        headerTemplate.bitsPerPixel = this->bitsPerPixel;
        headerTemplate.colorPlanes = this->colorPlanes;
        headerTemplate.paletteType = 1;
    }

    virtual std::vector<uint8_t> getImageData(const std::vector<std::vector<RGBA>> &rgbaPixels) = 0;

    virtual std::vector<uint8_t> get256PaletteData() = 0;

public:
    virtual PCX::PCXHeader generateHeader(const std::vector<std::vector<RGBA>> &rgbaPixels) = 0;

    std::vector<uint8_t> encodeImageData(const std::vector<std::vector<RGBA>> &rgbaPixels) {
        std::vector<uint8_t> encodedImageData;
        std::vector<uint8_t> imageData = getImageData(rgbaPixels);

        uint32_t currentByte = 0;
        while (currentByte < imageData.size()) {
            uint8_t repeat = 1;
            for (uint32_t i = currentByte + 1;
                 i < imageData.size() && imageData[currentByte] == imageData[i] && repeat < 63; ++i)
                ++repeat;
            if (repeat == 1 && (imageData[currentByte] & 0xC0) != 0xC0) {
                encodedImageData.push_back(imageData[currentByte]);
            } else {
                encodedImageData.push_back(0xC0 + repeat);
                encodedImageData.push_back(imageData[currentByte]);
            }
            currentByte += repeat;
        }

        auto palette = get256PaletteData();
        encodedImageData.insert(encodedImageData.end(), palette.begin(), palette.end());

        return encodedImageData;
    }
};


#endif
