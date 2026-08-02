#ifndef LEVELEDITORSCREEN_H
#define LEVELEDITORSCREEN_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

#include "Screen.h"
#include "TextureManager.h"
#include "Types.h"
#include "RenderTypes.h"
#include "EventTypes.h"
#include "MouseHelper.h"

struct GridInfo {
    // Grid constants
    float maxWidthPercent;
    float maxHeightPercent;
    size_t cellMargin;
    float centerX;
    float centerY;

    // Grid dimensions
    size_t sizeX;
    size_t sizeY;
    size_t tileSize;

    // Calculated values
    size_t gridWidth;
    size_t gridHeight;
    size_t realTileSize;
    size_t initialX;
    size_t initialY;
    size_t gridCenterX;
    size_t gridCenterY;

    GridInfo(size_t sizeX, size_t sizeY);

    bool UpdateGrid(size_t newWindowWidth, size_t newWindowHeight);
    bool ResizeGrid(size_t newSizeX, size_t newSizeY) {
        if (sizeX == newSizeX && sizeY == newSizeY) {
            return false;
        }
        sizeX = newSizeX;
        sizeY = newSizeY;
        return true;
    }
};

class LevelEditorScreen : public Screen {
    private:
        GridInfo _gridInfo;
        std::vector<GridHandle> _activeHandles;
        GridHandle _activeHandle = {GridHandleType::None, GridRectangle()};

        MouseHelper _mouseHLeft;
        MouseHelper _mouseHMiddle;
        
        void _renderProperties(PropertyPanelInfo& propertyPanelInfo, PropertyPayload& payload, const TextureManager& textureManager);

        void _renderGrid(LevelRenderInfo& levelRenderInfo, const TextureManager& textureManager);

        void _renderBrushText(const std::string& brushName);

        void _resize(const sf::Event::SizeEvent& sizeEvent);

        void _drawTextureInfo(
            const TextureRenderInfo& renderInfo,
            const sf::Texture& texture,
            const sf::Color& tint);

        void _drawGridRectangle(const GridRectangle& rectangle);

        void _drawHandle(const GridHandle& handle);

    public:
        LevelEditorScreen(sf::RenderWindow& window) 
            : Screen(window), _gridInfo(1, 1) {
                _gridInfo.UpdateGrid(window.getSize().x, window.getSize().y);
        };

        void Render(LevelRenderInfo& levelRenderInfo, PropertyPayload& payload, const TextureManager& textureManager, sf::Time deltaTime);

        std::pair<std::vector<UIEvent>, std::vector<GridEvent>> ProcessInput(const std::vector<sf::Event>& event);

        inline bool IsMouseDragging() const {
            return _mouseHLeft.IsDragging() || _mouseHMiddle.IsDragging();
        }
};

#endif