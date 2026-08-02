#ifndef COLORHELPER_H
#define COLORHELPER_H

#include <SFML/Graphics.hpp>
#include <string>

class ColorHelper {
    public:
        /*
        * Converts a hex value to a sf::Color
        */
        static sf::Color HexToColor(const std::string& hex) {
            if (hex[0] == '#') {
                return HexToColor(hex.substr(1));
            }
            if (hex.size() == 6) {
                unsigned int r = std::stoul(hex.substr(0, 2), nullptr, 16);
                unsigned int g = std::stoul(hex.substr(2, 2), nullptr, 16);
                unsigned int b = std::stoul(hex.substr(4, 2), nullptr, 16);
                return sf::Color(r, g, b);
            } else if (hex.size() == 8) {
                unsigned int r = std::stoul(hex.substr(0, 2), nullptr, 16);
                unsigned int g = std::stoul(hex.substr(2, 2), nullptr, 16);
                unsigned int b = std::stoul(hex.substr(4, 2), nullptr, 16);
                unsigned int a = std::stoul(hex.substr(6, 2), nullptr, 16);
                return sf::Color(r, g, b, a);
            } else {
                throw std::invalid_argument("Invalid hex color format");
            }
        }

        /*
        * Converts a sf::Color to a hex string.
        * Includes # at the start
        * Includes the alpha channel
        */
        static std::string ColorToHex(const sf::Color& color) {
            char hex[9];
            snprintf(hex, sizeof(hex), "%02X%02X%02X%02X", color.r, color.g, color.b, color.a);
            return std::string(hex);
        }
};

#endif