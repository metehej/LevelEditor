#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <memory>

#include "Types.h"
#include "LevelData.h"
#include "RenderTypes.h"
#include "TransientTypes.h"

class EntityManager;
class EntityInstance;
class EntityDefinition;

/*
* Handles rendering and packaging of data for Screen.
*/
class RenderManager {
    private:
        std::unique_ptr<LevelRenderInfo> _currentRenderInfo;

        void _refreshActiveEntity(const EntityManager& entityManager, const TransientData& transientData);

        void _refreshPropertyPanel(const EntityManager& entityManager, const TransientData& transientData, const LevelData& levelData);

        void _refreshHandles(const EntityManager& entityManager, const TransientData& transientData);

        /*
        * Get a rectangle of floats representing the binding rectangle specified instance.
        */
        GridRectangle _getBindingRectangle(const EntityInstance* instance, const EntityDefinition* definition) const;

        /*
        * Get a rectangle of floats representing a line this object walks on.
        */
        GridRectangle _getPathRectangle(const EntityInstance* instance, const EntityDefinition* definition) const;

        /*
        * Returns render info for the current entities.
        * Does not include active entity.
        */
        void _getEntityRenderInfo(const EntityManager& entityManager, const TransientData& transientData) const;

    public:
        RenderManager() = default;

        /*
        * Renders the level based on the provided render info.
        */
        LevelRenderInfo& RenderLevel(const EntityManager& entityManager, TransientData& transientData, const LevelData& levelData);
};

#endif