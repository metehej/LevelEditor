#include "CommandTypes.h"

#include "EntityManager.h"
#include "EntityDefinitions.h"
#include "EntityInstances.h"
#include "GridManager.h"
#include "LevelData.h"
#include "RenderTypes.h"
#include "Properties.h"

bool Command::Execute(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager) {
    return std::visit([&](auto&& payload) -> bool {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, EntityApplyPayload>) {
            auto entity = entityManager.GetEntityInstance(payload.instanceID);
            if (!entity) {
                return false;
            }
            if (!entity->ApplyProperties(payload.propertiesAfter, false, true)) {
                return false;
            }
            TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
            gridManager.ClearTiles(payload.instanceID);
            gridManager.ReserveTiles(renderInfo.startTile, renderInfo.size, payload.instanceID);
            return true;
        } else if constexpr (std::is_same_v<T, LevelApplyPayload>) {
            for (const auto& [propertyName, propertyValue] : payload.propertiesAfter) {
                auto it = levelData.propertyDefinitions.find(propertyName);
                if (it == levelData.propertyDefinitions.end()) {
                    return false;
                }
                levelData.properties[propertyName] = propertyValue;
            }
            return true;
        } else if constexpr (std::is_same_v<T, CreateDeletePayload>) {
            if (payload.isCreation) {
                if (!payload.instanceID) {
                    payload.instanceID = entityManager.CreateEntityInstance(payload.entityID, payload.properties);
                    if (!payload.instanceID) {
                        return false;
                    }
                } else if (!entityManager.RestoreEntityInstance(*payload.instanceID, payload.entityID, payload.properties)) {
                    return false;
                }
                auto entity = entityManager.GetEntityInstance(*payload.instanceID);
                if (!entity) {
                    return false;
                }
                TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
                if (!gridManager.ReserveTiles(renderInfo.startTile, renderInfo.size, *payload.instanceID)) {
                    entityManager.RemoveEntityInstance(*payload.instanceID);
                    return false;
                }
                return true;
            } else {
                if (!payload.instanceID) {
                    return false;
                }
                auto entity = entityManager.GetEntityInstance(*payload.instanceID);
                if (!entity) {
                    return false;
                }                
                TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
                gridManager.ClearTiles(*payload.instanceID);
                entityManager.RemoveEntityInstance(*payload.instanceID);
                return true;
            }
        }
        return false;
    }, payload);
}

bool Command::Undo(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager) {
    return std::visit([&](auto&& payload) -> bool {
        using T = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<T, EntityApplyPayload>) {
            auto entity = entityManager.GetEntityInstance(payload.instanceID);
            if (!entity) {
                return false;
            }
            if (!entity->ApplyProperties(payload.propertiesBefore, true, true)) {
                return false;
            }
            TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
            gridManager.ClearTiles(payload.instanceID);
            gridManager.ReserveTiles(renderInfo.startTile, renderInfo.size, payload.instanceID);
            return true;
        } else if constexpr (std::is_same_v<T, LevelApplyPayload>) {
            for (const auto& [propertyName, propertyValue] : payload.propertiesBefore) {
                levelData.properties[propertyName] = propertyValue;
            }
            return true;
        } else if constexpr (std::is_same_v<T, CreateDeletePayload>) {
            if (payload.isCreation) {
                if (!payload.instanceID) {
                    return false;
                }
                auto entity = entityManager.GetEntityInstance(*payload.instanceID);
                if (!entity) {
                    return false;
                }
                TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
                gridManager.ClearTiles(*payload.instanceID);
                entityManager.RemoveEntityInstance(*payload.instanceID);
                return true;
            } else {
                if (!payload.instanceID) {
                    return false;
                }
                if (!entityManager.RestoreEntityInstance(*payload.instanceID, payload.entityID, payload.properties)) {
                    return false;
                }
                auto entity = entityManager.GetEntityInstance(*payload.instanceID);
                if (!entity) {
                    return false;
                }
                TextureRenderInfo renderInfo = entity->GetTextureRenderInfo();
                if (!gridManager.ReserveTiles(renderInfo.startTile, renderInfo.size, *payload.instanceID)) {
                    entityManager.RemoveEntityInstance(*payload.instanceID);
                    return false;
                }
                return true;
            }
        }
        return false;
    }, payload);
}