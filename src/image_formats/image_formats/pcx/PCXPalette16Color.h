#ifndef PCXPALETTE16COLOR_H
#define PCXPALETTE16COLOR_H

#include <image_formats/pcx/PCXFormat.h>

class PCXPalette16Color : public PCXFormat {
protected:
    static auto convertPixelMatrixToVector(const std::vector<std::vector<RGBA>> &rgbaPixels){
        std::vector<RGBA> pixels;
        for(auto &&line : rgbaPixels)
            pixels.insert(pixels.end(), line.begin(), line.end());
        return pixels;
    }

    std::vector<uint8_t> get256PaletteData() override {
        return {};
    }

    std::vector<uint8_t> getImageData(const std::vector<std::vector<RGBA>> &rgbaPixels) override {
        auto relation = medianCutGetRelation(convertPixelMatrixToVector(rgbaPixels),16);
        auto palette = medianCutGetPalette(convertPixelMatrixToVector(rgbaPixels),16);
        std::map<RGB,uint32_t> paletteMap;
        for (uint32_t i = 0; i < palette.size();++i)
            paletteMap[palette[i]]=i;

        uint16_t bytesPerLine = (4 * rgbaPixels[0].size()+4)/8;
        std::vector<uint8_t> imageData(bytesPerLine*rgbaPixels.size());
        for (uint32_t row = 0; row < rgbaPixels.size(); ++row) {
            for (uint32_t pixel = 0; pixel < rgbaPixels[row].size(); ++pixel){
                uint32_t currentByte = row*bytesPerLine+pixel/2;
                if ((pixel&1)==0)
                    imageData[currentByte]+=paletteMap[convertRGBAToRGB(relation[rgbaPixels[row][pixel]])]<<4;
                else
                    imageData[currentByte]+=paletteMap[convertRGBAToRGB(relation[rgbaPixels[row][pixel]])];
            }
        }

        return imageData;
    }

public:
    PCXPalette16Color() : PCXFormat(4, 1) {}

    PCX::PCXHeader generateHeader(const std::vector<std::vector<RGBA>> &rgbaPixels) override {
        PCXPalette16Color::validatePixelMatrix(rgbaPixels);
        PCX::PCXHeader header = this->headerTemplate;
        header.xMax = rgbaPixels[0].size() - 1;
        header.yMax = rgbaPixels.size() - 1;
        header.bytesPerLine = (4 * rgbaPixels[0].size()+4)/8;
        auto palette = medianCutGetPalette(convertPixelMatrixToVector(rgbaPixels),16);
        memcpy(header.palette,&palette[0], sizeof(header.palette));
        return header;
    }
};


#endif
