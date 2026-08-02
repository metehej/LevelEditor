#include "LevelManager.h"

#include <cmath>

#include "Config.h"
#include "Types.h"
#include "Properties.h"
#include "EntityManager.h"
#include "RenderTypes.h"
#include "LevelData.h"

void LevelManager::_refreshActiveEntityProperties(const EntityManager& entityManager) {
    auto* entity = _transientData.activeEntity.get();
    if (!entity) {
        return;
    }
    TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
    _transientData.ResetPositionalData();
    _transientData.lastValidPosition = renderInfo.startTile;
    _transientData.lastValidSize = renderInfo.size;
    PropertyPayload payload;
    payload.properties = entityManager.GetEntityDefinition(entity->entityID)->GetPropertyDefinitions();
    payload.currentValues = entity->GetPropertiesMap();
    _transientData.propertyPayload = std::move(payload);
    _transientData.placementValid = true;
    payload.propertiesChanged = false;
    _transientData.gridRenderValid = false;
}

void LevelManager::_refreshLevelProperties(const LevelData& levelData) {
    if (_transientData.activeEntity) {
        return;
    }
    PropertyPayload payload;
    payload.properties = levelData.propertyDefinitions;
    payload.currentValues = levelData.properties;
    size_t sizeX = GetTypedValue<int>("gridSizeX", levelData.properties).value_or(Config::DEFAULT_GRID_SIZE_X);
    size_t sizeY = GetTypedValue<int>("gridSizeY", levelData.properties).value_or(Config::DEFAULT_GRID_SIZE_Y);
    _gridManager.Resize({sizeX, sizeY});
    _transientData.propertyPayload = std::move(payload);
    payload.propertiesChanged = false;
    _transientData.gridRenderValid = false;
}

bool LevelManager::LoadLevelData(EntityManager& entityManager, const LevelData& levelData) {
    bool loadSuccess = true;
    if (_transientData.activeEntity) {
        DeselectActiveEntity(levelData);
    }
    _refreshLevelProperties(levelData);
    for (const auto& instance : entityManager.GetEntityInstanceIDs()) {
        auto entity = entityManager.GetEntityInstance(instance);
        auto renderInfo = entity->GetTextureRenderInfo();
        if (!_gridManager.ReserveTilesSafely(renderInfo.startTile, renderInfo.size, instance)) {
            entityManager.RemoveEntityInstance(instance);
            LOG_W << "Failed to load entity instance at position (" << renderInfo.startTile.x << ", " << renderInfo.startTile.y << ") due to invalid or occupied position. The instance has been removed." << std::endl;
        }
    }
    return loadSuccess;
}

std::optional<InstanceID> LevelManager::GetInstanceAtPosition(Vector2<int> position) const {
    return _gridManager.GetTile(position);
}

bool LevelManager::SelectActiveEntity(InstanceID instanceID, EntityManager& entityManager) {
    if (_transientData.activeEntity) {
        return false;
    }
    _transientData.editorMode = EditorMode::EditPrimary;
    auto entity = entityManager.GetEntityInstance(instanceID);
    if (!entity) {
        return false;
    }
    _transientData.activeEntity = std::make_unique<EntityInstance>(entity->GetClone());
    _transientData.activeEntityID = instanceID;
    _refreshActiveEntityProperties(entityManager);
    return true;
}

bool LevelManager::DeselectActiveEntity(const LevelData& levelData) {
    if (!_transientData.activeEntity) {
        return false;
    }
    _transientData.activeEntity.reset();
    _transientData.activeEntityID = std::nullopt;
    _transientData.editorMode = EditorMode::PlaceEntity;
    _transientData.ResetPositionalData();
    _refreshLevelProperties(levelData);
    return true;
}

void LevelManager::RefreshActiveEntity(EntityManager& entityManager, const LevelData& levelData) {
    if (!_transientData.activeEntityID) {
        return;
    }
    InstanceID instanceID = *_transientData.activeEntityID;
    DeselectActiveEntity(levelData);
    SelectActiveEntity(instanceID, entityManager);
}

void LevelManager::RefreshLevel(const LevelData& levelData) {
    _refreshLevelProperties(levelData);
}

ApplyResult LevelManager::ApplyPropertiesToActive(const EntityManager& entityManager) {
    ApplyResult result = ApplyResult::Failure;
    if (!_transientData.activeEntity || !_transientData.activeEntityID) {
        return result;
    }
    auto textureInfo = _transientData.activeEntity->GetTextureRenderInfo();
    if (!_transientData.propertyPayload.propertiesChanged
            && _transientData.lastValidPosition == textureInfo.startTile
            && _transientData.lastValidSize == textureInfo.size) {
        if (_transientData.currentDisplacement != Vector2<float>{0.0f, 0.0f} 
                || _transientData.currentSizeChange != Vector2<float>{0.0f, 0.0f}) {
            _transientData.ResetPositionalData();
            _transientData.gridRenderValid = false;
            _transientData.placementValid = true;
        }
        return ApplyResult::NoChange;
    }
    result = ApplyResult::AllSuccess;
    PropertyValueVect propertiesToApply;
    TextureRenderInfo originalRenderInfo = _transientData.activeEntity->GetTextureRenderInfo();
    if (originalRenderInfo.startTile != _transientData.lastValidPosition) {
        if (_transientData.editorMode == EditorMode::EditPrimary) {
            propertiesToApply.push_back({"posX", static_cast<int>(_transientData.lastValidPosition.x)});
            propertiesToApply.push_back({"posY", static_cast<int>(_transientData.lastValidPosition.y)});
        } else if (_transientData.editorMode == EditorMode::EditSecondary) {
            propertiesToApply.push_back({"startX", static_cast<int>(_transientData.lastValidPosition.x)});
            propertiesToApply.push_back({"startY", static_cast<int>(_transientData.lastValidPosition.y)});
        }
    }
    if (originalRenderInfo.size != _transientData.lastValidSize) {
        if (_transientData.editorMode == EditorMode::EditPrimary) {
            propertiesToApply.push_back({"sizeX", static_cast<int>(_transientData.lastValidSize.x)});
            propertiesToApply.push_back({"sizeY", static_cast<int>(_transientData.lastValidSize.y)});
        } else if (_transientData.editorMode == EditorMode::EditSecondary) {
            propertiesToApply.push_back({"endX", static_cast<int>(_transientData.lastValidPosition.x + _transientData.lastValidSize.x)});
            propertiesToApply.push_back({"endY", static_cast<int>(_transientData.lastValidPosition.y + _transientData.lastValidSize.y)});
        }
    }
    for (const auto& [propertyName, propertyValue] : _transientData.propertyPayload.currentValues) {
        auto it = _transientData.propertyPayload.properties.find(propertyName);
        if (it == _transientData.propertyPayload.properties.end()) {
            continue;
        }
        const Property& propDef = it->second;
        if (MatchesKind(propDef.type, propertyValue) 
                && propDef.Validate(propertyValue, _transientData.propertyPayload.currentValues)) { 
            if (_transientData.activeEntity->GetProperty(propertyName) != propertyValue) {
                propertiesToApply.push_back({propertyName, propertyValue});
            }
        } else {
            result = ApplyResult::PartialSuccess;
        }
    }
    if (propertiesToApply.empty()) {
        return ApplyResult::NoChange;
    }
    _transientData.activeEntity->ApplyProperties(propertiesToApply, true, false);
    _refreshActiveEntityProperties(entityManager);
    return result;
}

bool LevelManager::HasLevelChanged() const {
    if (!_transientData.activeEntity && _transientData.propertyPayload.propertiesChanged) {
        return true;
    }
    return false;
}

void LevelManager::MoveActive(Vector2<float> distance) {
    if (!_transientData.activeEntity) {
        return;
    }
    _transientData.currentDisplacement += distance;
    TextureRenderInfo renderInfo = _transientData.activeEntity->GetTextureRenderInfo();
    Vector2<float> newPosition = {
        std::round(renderInfo.startTile.x + _transientData.currentDisplacement.x),
        std::round(renderInfo.startTile.y + _transientData.currentDisplacement.y)
    };
    if (_validateRectangle(newPosition, renderInfo.size)) {
        _transientData.lastValidPosition = newPosition;
        _transientData.placementValid = true;
    } else {
        _transientData.placementValid = false;
    }
    _transientData.gridRenderValid = false;
}

void LevelManager::ScaleActive(Vector2<float> sizeChange, GridHandleType handleType) {
    if (!_transientData.activeEntity) {
        return;
    }
    Vector2<float> potentialDisplacement = _transientData.currentDisplacement;
    Vector2<float> potentialSizeChange = _transientData.currentSizeChange;

    switch (handleType) {
        case GridHandleType::Top:
            potentialDisplacement.y += sizeChange.y;
            potentialSizeChange.y -= sizeChange.y;
            break;
        case GridHandleType::Left:
            potentialDisplacement.x += sizeChange.x;
            potentialSizeChange.x -= sizeChange.x;
            break;
        case GridHandleType::Bottom:
            potentialSizeChange.y += sizeChange.y;
            break;
        case GridHandleType::Right:
            potentialSizeChange.x += sizeChange.x;
            break;
        default:
            return;
    }

    TextureRenderInfo renderInfo = _transientData.activeEntity->GetTextureRenderInfo();
    Vector2<float> newSize = {
        std::round(renderInfo.size.x + potentialSizeChange.x),
        std::round(renderInfo.size.y + potentialSizeChange.y)
    };
    if (newSize.x < 1.0f) {
        potentialSizeChange.x = _transientData.currentSizeChange.x;
        potentialDisplacement.x = _transientData.currentDisplacement.x;
    }
    if (newSize.y < 1.0f) {
        potentialSizeChange.y = _transientData.currentSizeChange.y;
        potentialDisplacement.y = _transientData.currentDisplacement.y;
    }

    _transientData.currentSizeChange = potentialSizeChange;
    _transientData.currentDisplacement = potentialDisplacement;
    newSize = {
        std::round(renderInfo.size.x + potentialSizeChange.x),
        std::round(renderInfo.size.y + potentialSizeChange.y)
    };
    Vector2<float> newPosition = {
        std::round(renderInfo.startTile.x + potentialDisplacement.x),
        std::round(renderInfo.startTile.y + potentialDisplacement.y)
    };

    if (_validateRectangle(newPosition, newSize)) {
        _transientData.lastValidSize = newSize;
        _transientData.lastValidPosition = newPosition;
        _transientData.placementValid = true;
    } else {
        _transientData.placementValid = false;
    }
    _transientData.gridRenderValid = false;
}

TransientData& LevelManager::GetTransientData() {
    return _transientData;
}

std::optional<PropertyValueVect> LevelManager::PlaceEntity(Vector2<int> position, EntityManager& entityManager) {
    size_t entityID = _transientData.currentEntityBrushID;
    auto entityDef = entityManager.GetEntityDefinition(entityID);
    if (!entityDef) {
        return std::nullopt;
    }
    PropertyValueVect properties = entityDef->GetDefaultProperties();
    if (_transientData.currentCopyBuffer) {
        for (const auto& [propertyName, propertyValue] : *_transientData.currentCopyBuffer) {
            auto it = std::find_if(
                properties.begin(),
                properties.end(),
                [&propertyName](const auto& entry) {
                    return entry.first == propertyName;
                }
            );
            if (it != properties.end()) {
                it->second = propertyValue;
            }
        }
    }
    int sizeX = GetTypedValue<int>("sizeX", properties).value_or(1);
    int sizeY = GetTypedValue<int>("sizeY", properties).value_or(1);
    Vector2<int> size{sizeX, sizeY};
    if (!_validateRectangle(position, size, false)) {
        return std::nullopt;
    }
    auto replaceValues = [&properties](std::string_view PropertyName, int value) {
        auto it = std::find_if(
            properties.begin(),
            properties.end(),
            [&PropertyName](const auto& entry) {
                return entry.first == PropertyName;
            }
        );
        if (it != properties.end()) {
            it->second = value;
        } else {
            properties.push_back({std::string(PropertyName), value});
        }
    };
    replaceValues("posX", position.x);
    replaceValues("posY", position.y);
    replaceValues("sizeX", sizeX);
    replaceValues("sizeY", sizeY);
    return properties;
}

bool LevelManager::CopyActiveEntity() {
    if (!_transientData.activeEntity) {
        return false;
    }
    _transientData.currentCopyBuffer = std::make_unique<PropertyValueVect>(PropertyMapToPayload(_transientData.activeEntity->GetPropertiesMap()));
    _transientData.currentEntityBrushID = _transientData.activeEntity->entityID;
    return true;
}

bool LevelManager::SetActiveBrush(size_t entityID, const EntityManager& entityManager) {
    auto entityDef = entityManager.GetEntityDefinition(entityID);
    if (!entityDef) {
        return false;
    }
    _transientData.currentEntityBrushID = entityID;
    _transientData.currentCopyBuffer.reset();
    _transientData.gridRenderValid = false;
    return true;
}