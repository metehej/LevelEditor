#ifndef RENDERTYPES_H
#define RENDERTYPES_H

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Types.h"
#include "Properties.h"

enum class GridHandleType {
    Top,
    Left,
    Bottom,
    Right,
    None
};

struct GridRectangle {
    // Floats are fractions of a tile
    Vector2<float> position = {0.0f, 0.0f}; 
    Vector2<float> size = {0.0f, 0.0f};
    std::string colorHex = "#ffffff";
};

struct GridHandle {
    GridHandleType type;
    GridRectangle entityRectangle;
};

struct TextureRenderInfo {
    size_t textureID;
    Vector2<float> startTile;
    Vector2<float> size;
};

struct TintedTextureRenderInfo : TextureRenderInfo {
    std::string tintColorHex = "#ffffff";
    TintedTextureRenderInfo() = default;
    TintedTextureRenderInfo(TextureRenderInfo&& baseInfo) : TextureRenderInfo(std::move(baseInfo)) {}
};

struct PropertyPayload {
    PropertyMap properties;
    PropertyValueMap currentValues;
    bool propertiesChanged = false;
};

struct EntityPaletteInfo {
    std::string name;
    size_t entityID;
    size_t textureID;
};

using EntityPalette = std::map<std::string, std::vector<EntityPaletteInfo>>;

struct PropertyPanelInfo {
    std::string name;
    std::string description;
    std::optional<size_t> collectionID = std::nullopt;
    std::optional<std::reference_wrapper<const EntityPalette>> entityPalette = std::nullopt;
};

struct LevelRenderInfo {
    // Textures will be drawn in the order of these vectors
    std::vector<TextureRenderInfo> texturesToDraw;
    std::vector<TintedTextureRenderInfo> tintedTexturesToDraw;
    std::vector<GridRectangle> rectanglesToDraw;
    std::vector<GridHandle> handlesToDraw;
    PropertyPanelInfo propertyPanelInfo;
    Vector2<size_t> gridSize;
    std::string activeBrushName;
};
#endif