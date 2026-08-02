#ifndef COMMAND_TYPES_H
#define COMMAND_TYPES_H

#include "Types.h"
#include "WorldTypes.h"

class EntityManager;
class GridManager;
struct LevelData;
struct PropertyPayload;

struct EntityApplyPayload {
    size_t entityID;
    InstanceID instanceID;
    PropertyValueVect propertiesBefore;
    PropertyValueVect propertiesAfter;
};

struct LevelApplyPayload {
    PropertyValueVect propertiesBefore;
    PropertyValueVect propertiesAfter;
};

struct CreateDeletePayload {
    size_t entityID;
    std::optional<InstanceID> instanceID;
    PropertyValueVect properties;
    bool isCreation;
};

struct Command {
    public:
        std::variant<EntityApplyPayload, LevelApplyPayload, CreateDeletePayload> payload;

        /*
        * Executes command based on payload.
        * Returns false if any part of the process fails.
        * No changes are done on failure.
        */
        bool Execute(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager);

        /*
        * Reverse-executes the command based on payload.
        * Returns false if any part of the process fails.
        * No changes are done on failure.
        */
        bool Undo(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager);
};


#endif