#ifndef CONFIG_H
#define CONFIG_H

#include <SFML/Graphics.hpp>
#include <string>

namespace Config {
    inline static constexpr float SCREEN_RATIO = 16.0f / 9.0f;
    inline static constexpr int WINDOW_WIDTH = 1280;
    inline static constexpr int WINDOW_HEIGHT = static_cast<int>(WINDOW_WIDTH / SCREEN_RATIO);
    inline static constexpr int DEFAULT_GRID_SIZE_X = 24;
    inline static constexpr int DEFAULT_GRID_SIZE_Y = 16;
    inline static constexpr int TILE_PIXEL_SIZE = 32;
    inline static constexpr int TILE_MARGIN = 2;
    inline static const sf::Color GRID_COLOR = sf::Color(200, 200, 200);
    inline static const sf::Color GRID_LINE_COLOR = sf::Color(100, 100, 100);
    inline static const sf::Color BACKGROUND_COLOR = sf::Color(50, 50, 50);
    inline static const sf::Color BORDER_COLOR = sf::Color(20, 20, 20);

    inline static const std::string LEVELS_PATH = "data/levels/";
    inline static const std::string LEVEL_LOADOUT_PATH = "data/loadout.xml";
    inline static const std::string ENTITIES_PATH = "data/entities/";
    inline static const std::string SPRITE_FILES_PATH = "data/sprites/files/";
    inline static const std::string SPRITE_META_PATH = "data/sprites/metadata/";
    inline static const std::string LOG_DIRECTORY = "logs/";
    inline static const std::string LOG_FILE = LOG_DIRECTORY + "editor.log";
    inline static const std::string TEXTURE_OUTPUT_PATH = "output/textures.png";
    inline static const std::string TEXTURE_REGIONS_OUTPUT_PATH = "output/textureRegions.xml";
    inline static const int REGIONS_MAX_SIZE = 4096;
    inline static const int REGION_PADDING = 2;
    inline static constexpr size_t LOG_SIZE = 5 * 1024 * 1024; 

    inline static constexpr float MIN_DRAG_DISTANCE = 0.2f; // in tiles

    inline static const sf::Color INFO_MESSAGE_COLOR = sf::Color(173, 107, 66, 240);
    inline static const sf::Color INFO_MESSAGE_BORDER = sf::Color(77, 43, 22, 240);
    inline static constexpr float INFO_MESSAGE_DURATION = 4.0f; // seconds

    inline static const sf::Color HANDLE_COLOR = sf::Color(200, 200, 100);
    inline static const sf::Color ACTIVE_HANDLE_COLOR = sf::Color(255, 255, 150);
    inline static const sf::Color HANDLE_OUTLINE_COLOR = sf::Color(50, 50, 0);

    inline static const sf::Color INCORRECT_VALUE_COLOR = sf::Color(255, 100, 100);

    inline static constexpr float MESSAGE_BOX_WIDTH_P = 0.4f;
    inline static constexpr float MESSAGE_BOX_WIDTH_MIN = 300.0f;
    inline static constexpr float MESSAGE_BOX_MAX_CHARACTERS = 55;
    inline static constexpr int MAX_MESSAGE_BOXES = 5;
    inline static constexpr float POPUP_WIDTH = 350;
    inline static constexpr float POPUP_SPACING = 10;

    inline static constexpr float PROPERTY_PANEL_WIDTH = 0.25;
    inline static constexpr float BUTTON_WIDTH = 0.05;
    inline static constexpr float HANDLE_HEIGHT = 0.3;
    inline static constexpr float MAX_BUTTON_SIZE = 80.0f;
    inline static constexpr float MIN_BUTTON_SIZE = 50.0f;

    inline static constexpr float PROPERTY_CHANGE_DELAY = 0.5f; // seconds

    inline static constexpr size_t STACK_LIMIT = 100;
};

#endif