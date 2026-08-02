#include "RenderManager.h"

#include "EntityManager.h"

void RenderManager::_refreshActiveEntity(const EntityManager& entityManager, const TransientData& transientData) {
    if (transientData.editorMode == EditorMode::PlaceEntity) {
        return;
    } else {
        auto* activeEntity = transientData.activeEntity.get();
        auto activeEntityID = transientData.activeEntityID;
        if (!activeEntity || !activeEntityID) {
            return;
        }
        TintedTextureRenderInfo activeInfo = activeEntity->GetTextureRenderInfo();
        float sizeXFactor = 1.0f + transientData.currentSizeChange.x / activeInfo.size.x;
        float sizeYFactor = 1.0f + transientData.currentSizeChange.y / activeInfo.size.y;
        Vector2<float> sizeFactor = {sizeXFactor, sizeYFactor};
        activeInfo.startTile += transientData.currentDisplacement;
        activeInfo.size *= sizeFactor;
        if (!transientData.placementValid) {
            activeInfo.tintColorHex = "#ff0000d1";
        }
        _currentRenderInfo->tintedTexturesToDraw.push_back(std::move(activeInfo));
        GridRectangle rectangleInfo;
        if (transientData.editorMode == EditorMode::EditPrimary) {
            rectangleInfo = _getBindingRectangle(activeEntity, entityManager.GetEntityDefinition(activeEntity->entityID));
        } else {
            rectangleInfo = _getPathRectangle(activeEntity, entityManager.GetEntityDefinition(activeEntity->entityID));
        }
        rectangleInfo.colorHex = "#ddff0070";
        rectangleInfo.position += transientData.currentDisplacement;
        rectangleInfo.size *= sizeFactor;
        _currentRenderInfo->rectanglesToDraw.push_back(rectangleInfo);
    }
}

void RenderManager::_refreshPropertyPanel(const EntityManager& entityManager, const TransientData& transientData, const LevelData& levelData) {
    auto* activeEntity = transientData.activeEntity.get();
    if (transientData.editorMode != EditorMode::PlaceEntity && activeEntity) {
        PropertyPanelInfo& propPanel = _currentRenderInfo->propertyPanelInfo;
        auto entityDef = entityManager.GetEntityDefinition(activeEntity->entityID);
        propPanel.name = "Entity Properties";
        propPanel.description = "Editing entity " + entityDef->name + " of type " + entityDef->type + ".";
        propPanel.collectionID = entityDef->GetSpriteCollection();
        propPanel.entityPalette = std::nullopt;
    } else {
        PropertyPanelInfo& propPanel = _currentRenderInfo->propertyPanelInfo;
        propPanel.name = "Level Properties";
        propPanel.description = "Editing level " + GetTypedValue<std::string>("levelName", levelData.properties).value_or("Untitled");
        propPanel.entityPalette = entityManager.GetPalette();
    } 
}

void RenderManager::_refreshHandles(const EntityManager& entityManager, const TransientData& transientData) {
    _currentRenderInfo->handlesToDraw.clear();
    auto* activeEntity = transientData.activeEntity.get();
    if (!activeEntity || transientData.editorMode == EditorMode::PlaceEntity) {
        return;
    }
    GridRectangle handleBase;
    if (transientData.editorMode == EditorMode::EditPrimary) {
        auto renderInfo = activeEntity->GetTextureRenderInfo();
        handleBase.position = Vector2<float>{renderInfo.startTile.x, renderInfo.startTile.y};
        handleBase.size = Vector2<float>{renderInfo.size.x, renderInfo.size.y};
    } else if (transientData.editorMode == EditorMode::EditSecondary) {
        handleBase = _getPathRectangle(activeEntity, entityManager.GetEntityDefinition(activeEntity->entityID));
    }
    handleBase.position += transientData.currentDisplacement;
    handleBase.size += transientData.currentSizeChange;
    if (transientData.editorMode == EditorMode::EditPrimary) {
        _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Top, handleBase});
        _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Left, handleBase});
        _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Bottom, handleBase});
        _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Right, handleBase});
    } else if (transientData.editorMode == EditorMode::EditSecondary) {
        const auto* def = entityManager.GetEntityDefinition(activeEntity->entityID);
        if (!def || def->GetPlacementKind() != EntityPlacementKind::PathBased) {
            return;
        }
        auto entityPlacement = activeEntity->GetTextureRenderInfo();
        if (handleBase.position.x >= entityPlacement.startTile.x &&
                handleBase.position.x + handleBase.size.x <= entityPlacement.startTile.x + entityPlacement.size.x) {
            // Vertical extension possible
            _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Top, handleBase});
            _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Bottom, handleBase});
                
        }
        if (handleBase.position.y >= entityPlacement.startTile.y &&
                handleBase.position.y + handleBase.size.y <= entityPlacement.startTile.y + entityPlacement.size.y) {
            // Horizontal extension possible
            _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Left, handleBase});
            _currentRenderInfo->handlesToDraw.push_back({GridHandleType::Right, handleBase});
        }
    }
}

GridRectangle RenderManager::_getBindingRectangle(const EntityInstance* instance, const EntityDefinition* definition) const {
    GridRectangle rect;
    auto bindingKind = definition ? definition->GetBindingKind() : EntityBindingKind::NonEditable;
    if (instance && bindingKind == EntityBindingKind::Editable) {
        // Get position and size
        TextureRenderInfo renderInfo = instance->GetTextureRenderInfo();
        rect.position = renderInfo.startTile;
        rect.size = renderInfo.size;
        auto it = instance->GetProperty("bindScaleX");
        if (it.has_value() && std::holds_alternative<float>(it.value())) {
            rect.size.x *= std::get<float>(it.value());
        }
        it = instance->GetProperty("bindScaleY");
        if (it.has_value() && std::holds_alternative<float>(it.value())) {
            rect.size.y *= std::get<float>(it.value());
        }
        it = instance->GetProperty("bindOrigin");
        Origin origin = Origin::TopLeft;
        if (it.has_value()) {
            auto parsedOrigin = GetEnumValue<Origin>(it.value());
            if (parsedOrigin.has_value()) {
                origin = parsedOrigin.value();
            }
        }
        // Calculate offset based on origin
        float marginX = renderInfo.size.x - rect.size.x;
        float marginY = renderInfo.size.y - rect.size.y;
        switch (origin) {
            case Origin::TopCenter:
            case Origin::Center:
            case Origin::BottomCenter:
                rect.position.x += marginX / 2;
                break;
            case Origin::TopRight:
            case Origin::CenterRight:
            case Origin::BottomRight:
                rect.position.x += marginX;
                break;
            default:                
                break;
        }
        switch (origin) {
            case Origin::CenterLeft:
            case Origin::Center:
            case Origin::CenterRight:
                rect.position.y += marginY / 2;
                break;
            case Origin::BottomLeft:
            case Origin::BottomCenter:
            case Origin::BottomRight:
                rect.position.y += marginY;
                break;
            default:                
                break;
        }
    }
    return rect;
}

GridRectangle RenderManager::_getPathRectangle(const EntityInstance* instance, const EntityDefinition* definition) const {
    GridRectangle rect;
    auto placementKind = definition ? definition->GetPlacementKind() : EntityPlacementKind::Static;
    if (instance && placementKind == EntityPlacementKind::PathBased) {
        auto it = instance->GetProperty("startX");
        if (it.has_value() && std::holds_alternative<int>(it.value())) {
            rect.position.x = std::get<int>(it.value());
        }
        it = instance->GetProperty("startY");
        if (it.has_value() && std::holds_alternative<int>(it.value())) {
            rect.position.y = std::get<int>(it.value());
        }
        it = instance->GetProperty("endX");
        if (it.has_value() && std::holds_alternative<int>(it.value())) {
            rect.size.x = std::get<int>(it.value()) - rect.position.x;
        }
        it = instance->GetProperty("endY");
        if (it.has_value() && std::holds_alternative<int>(it.value())) {
            rect.size.y = std::get<int>(it.value()) - rect.position.y;
        }
    }
    return rect;
}

void RenderManager::_getEntityRenderInfo(const EntityManager& entityManager, const TransientData& transientData) const {
    std::vector<TextureRenderInfo> texturesToDraw;
    auto instances = entityManager.GetEntityInstances(transientData.activeEntityID);
    for (const auto& instance : instances) {
        if (!instance) {
            continue;
        }
        texturesToDraw.push_back(instance->GetTextureRenderInfo());
    }
    _currentRenderInfo->texturesToDraw = std::move(texturesToDraw);
}

LevelRenderInfo& RenderManager::RenderLevel(const EntityManager& entityManager, TransientData& transientData, const LevelData& levelData) {
    if (!transientData.gridRenderValid) {
        _currentRenderInfo = std::make_unique<LevelRenderInfo>();
        _getEntityRenderInfo(entityManager, transientData);
        _refreshActiveEntity(entityManager, transientData);
        _currentRenderInfo->activeBrushName = 
            entityManager.GetEntityDefinition(transientData.currentEntityBrushID) 
            ? entityManager.GetEntityDefinition(transientData.currentEntityBrushID)->name 
            : "Select a brush.";
        if (transientData.currentCopyBuffer) {
            _currentRenderInfo->activeBrushName += " (Copy)";
        }
        auto gridSizeX = GetTypedValue<int>("gridSizeX", levelData.properties).value_or(Config::DEFAULT_GRID_SIZE_X);
        auto gridSizeY = GetTypedValue<int>("gridSizeY", levelData.properties).value_or(Config::DEFAULT_GRID_SIZE_Y);
        _currentRenderInfo->gridSize = Vector2<int>{gridSizeX, gridSizeY};
        _refreshPropertyPanel(entityManager, transientData, levelData);
        _refreshHandles(entityManager, transientData);
        transientData.gridRenderValid = true;
    }
    return *_currentRenderInfo;
}