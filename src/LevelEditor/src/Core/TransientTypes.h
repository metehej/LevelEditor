#ifndef TRANSIENTTYPES_H
#define TRANSIENTTYPES_H

#include "Types.h"
#include "Properties.h"
#include "RenderTypes.h"
#include "EntityDefinitions.h"
#include "EntityInstances.h"
#include "LevelData.h"
#include "WorldTypes.h"

struct TransientData {
    Vector2<float> currentDisplacement = {0.0f, 0.0f};
    Vector2<float> currentSizeChange = {0.0f, 0.0f};
    Vector2<float> lastValidPosition = {0.0f, 0.0f};
    Vector2<float> lastValidSize = {0.0f, 0.0f};

    size_t currentEntityBrushID = 0;
    std::unique_ptr<PropertyValueVect> currentCopyBuffer = nullptr;
    EditorMode editorMode = EditorMode::PlaceEntity;
    std::unique_ptr<EntityInstance> activeEntity = nullptr;
    std::optional<InstanceID> activeEntityID = std::nullopt;
    bool placementValid = true;
    bool gridRenderValid = false;

    PropertyPayload propertyPayload;

    inline void ResetPositionalData() {
        currentDisplacement = {0.0f, 0.0f};
        currentSizeChange = {0.0f, 0.0f};
        lastValidPosition = {0.0f, 0.0f};
        lastValidSize = {0.0f, 0.0f};
        bool gridRenderValid = false;
        bool placementValid = true;
    }
};

struct FileEntityData {
    std::string name;
    std::vector<std::unordered_map<std::string, std::string>> instancesProperties;
};

struct FileLevelData {
    std::vector<FileEntityData> entitiesData;
    LevelData levelData;
};

struct FileSpriteData {
    std::string fileName;
    std::string tag;
    std::string humanName;
};

struct FileEntityDefinitionData {
    std::vector<FileSpriteData> sprites;
    EntityDefinition definition;

    /*
    * Creates sprite collection for this definition.
    * Uses "placeholder" sprite for missing or failed sprites.
    */
    void ApplySpriteData(TextureManager& textureManager) {
        if (sprites.empty()) {
            sprites.push_back({"", "placeholder", "Placeholder"});
        }
        std::vector<std::string> spriteNames;
        std::vector<std::string> spriteTags;
        std::vector<std::string> spriteHumanNames;
        for (const auto& spriteData : sprites) {
            spriteNames.push_back(spriteData.fileName);
            spriteTags.push_back(spriteData.tag);
            spriteHumanNames.push_back(spriteData.humanName);
        }
        size_t collectionID = textureManager.CreateCollection(definition.name + "_collection", spriteNames);
        auto& collection = textureManager.GetCollection(collectionID);
        for (size_t i = 0; i < sprites.size(); i++) {
            size_t spriteID = textureManager.LoadSprite(sprites[i].fileName);
            collection.SetSpriteTag(spriteID, sprites[i].tag);
            collection.SetSpriteName(spriteID, sprites[i].humanName);
        }
        auto& ids = collection.GetSpriteIDs();
        auto representationSprite = textureManager.GetSprite(ids[0]);
        definition.SetSpriteCollection(collectionID, representationSprite.GetTextureID());
    }
};

struct TextureRegion {
    std::string fileName;
    Vector2<int> size;
    
    std::string GetID() const {
        return fileName + "_" + std::to_string(size.x) + "x" + std::to_string(size.y);
    }
};

struct LevelView{
    std::string fileName;
    std::string humanName;
};

#endif