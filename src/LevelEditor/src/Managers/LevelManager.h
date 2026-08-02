#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include "Types.h"
#include "TransientTypes.h"
#include "GridManager.h"

class EntityManager;
struct LevelData;

enum class ApplyResult {
    AllSuccess,
    PartialSuccess,
    NoChange,
    Failure
};

/*
* Manages transient data and logic.
*/
class LevelManager {
    private:
        GridManager _gridManager;
        TransientData _transientData;

        inline bool _validateRectangle(const Vector2<float>& position, const Vector2<float>& size, bool ignoreActive = true) const {
            if (size.x <= 0 || size.y <= 0) {
                return false;
            }
            return !_gridManager.AreTilesReserved(position, size,
                ignoreActive ? _transientData.activeEntityID : std::nullopt);
        }

        void _refreshActiveEntityProperties(const EntityManager& entityManager);

        void _refreshLevelProperties(const LevelData& levelData);

    public:
        LevelManager() = default;
        
        /*
        * Loads level properties from level data and
        * sets grid reservations based on current instances in entity manager.
        * If any entity cannot be placed, it is removed and its removal noted in log.
        * Returns false if any entity failed to load, true otherwise.
        */
        bool LoadLevelData(EntityManager& entityManager, const LevelData& levelData);

        /*
        * Returns ID of entity at position.
        * Returns std::nullopt if no entity is present or if position is out of bounds.
        */
        std::optional<InstanceID> GetInstanceAtPosition(Vector2<int> position) const;

        /*
        * Creates a clone of an entity to place inside transient data.
        * If a clone is present, fails.
        */
        bool SelectActiveEntity(InstanceID instanceID, EntityManager& entityManager);

        /*
        * Deletes clone from transient data.
        * Does not save any properties.
        * Returns false if no active entity is selected.
        */
        bool DeselectActiveEntity(const LevelData& levelData);

        /*
        * Checks if the original entity is still present in EntityManager.
        * Deselects active entity if it's original is no longer present.
        * Pulls properties from original entity to transient data if it's present.
        */
        void RefreshActiveEntity(EntityManager& entityManager, const LevelData& levelData);

        /*
        * Refreshes properties based on level.
        */
        void RefreshLevel(const LevelData& levelData);

        /*
        * Validates and applies valid properties from transient data to active entity.
        * Returns information on the result of the operation, see ApplyResult.
        * Transient data properties are refreshed.
        */
        ApplyResult ApplyPropertiesToActive(const EntityManager& entityManager);

        /*
        * Checks whether transient data contain level property changes.
        */
        bool HasLevelChanged() const;

        /*
        * Applies movement to transient properties.
        */
        void MoveActive(Vector2<float> distance);

        /*
        * Applies scaling to transient properties.
        * GridHandleType determines the direction of scaling.
        */
        void ScaleActive(Vector2<float> sizeChange, GridHandleType handleType);

        /*
        * Returns a reference to transient data.
        */
        TransientData& GetTransientData();

        GridManager& GetGridManager() {
            return _gridManager;
        }

        /*
        * Returns payload for creating a new entity based on active entity.
        * Returns std::nullopt if the resulting entity cannot be placed.
        */
        std::optional<PropertyValueVect> PlaceEntity(Vector2<int> position, EntityManager& entityManager);

        /*
        * Copies properties and id of active entity to transient data.
        */
        bool CopyActiveEntity();

        /*
        * Sets a new brush.
        * Automatically purges copy data.
        * If EntityID not found, returns false.
        */
        bool SetActiveBrush(size_t entityID, const EntityManager& entityManager);
};
#endif