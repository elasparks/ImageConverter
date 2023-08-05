#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <image_types/Bitmap.h>
#include <image_formats/pcx/PCXPalette16Color.h>

std::vector<char> readAllBytes(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open() || file.bad())
        throw std::runtime_error("Error: could not open file!");
    int length = file.tellg();
    std::vector<char> bytes(length);
    file.seekg(0, std::ios::beg);
    file.read(&bytes[0], length);
    file.close();
    return bytes;
}

void saveBytesToFile(const std::vector<char> &bytes, const std::string &path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open() || file.bad())
        throw std::runtime_error("Error: could not open file!");
    file.write(&bytes[0], bytes.size());
    file.close();
}

void showPixels(const std::vector<std::vector<RGBA>>& pixels){
    auto width = pixels[0].size();
    auto height = pixels.size();
    sf::RenderWindow window(sf::VideoMode(width, height), "Picture");
    sf::Image image;
    sf::Texture texture;
    texture.create(width, height);
    image.create(width, height);

    for (int x = 0; x < width; ++x){
        for (int y = 0; y < height; ++y){
            image.setPixel(x, y, sf::Color(pixels[y][x].red,pixels[y][x].green, pixels[y][x].blue));
        }
    }

    texture.update(image);
    sf::Sprite sprite(texture);

    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.draw(sprite);
        window.display();
    }
}

int main(int argc, char* argv[]) {
    auto bytes = readAllBytes(argv[1]);
    Bitmap bitmap(bytes);
    showPixels(bitmap.getPixels());
    PCXPalette16Color palette16Color;
    std::vector<char> image(PCX::PCX_HEADER_SIZE);
    auto header = palette16Color.generateHeader(bitmap.getPixels());
    memcpy(&image[0],&header,PCX::PCX_HEADER_SIZE);
    auto encodedImageData = palette16Color.encodeImageData(bitmap.getPixels());
    image.insert(image.end(),encodedImageData.begin(), encodedImageData.end());
    PCX pcx(image);
    showPixels(pcx.getPixels());
    saveBytesToFile(image,std::string("16color[")+argv[1]+"].pcx");
    return 0;
}
