#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <vector>
#include <deque>
#include <unordered_map>
#include <memory>
#include <optional>
#include <map>
#include <string>


#include "WorldTypes.h"
#include "RenderTypes.h"
#include "EntityInstances.h"
#include "EntityDefinitions.h"
#include "Types.h"

class TextureManager;
struct SpriteCollection;
struct Sprite;

class EntityManager {
    private:
        TextureManager& _textureManager;

        std::vector<std::unique_ptr<EntityDefinition>> _entityDefinitions;
        EntityPalette _palette;
        std::unordered_map<std::string, size_t> _entityNameToID;
        std::vector<std::vector<InstanceID>> _entityIDToInstanceIDs;

        std::vector<std::unique_ptr<EntityInstance>> _entities;
        std::vector<size_t> _entitySlots;
        std::vector<size_t> _entityGenerations;

        bool _checkInstanceIDValid(InstanceID instanceID) const;

        EntityInstance* _getEntityInstance(InstanceID instanceID) const;
    
    public:
        EntityManager(TextureManager& textureManager) : _textureManager(textureManager) {}

        /*
        * Add a new entity definition. Returns the ID of the new entity definition.
        * If an entity definition with the same name already exists, returns the ID of the existing entity definition.
        */
        size_t AddEntityDefinition(EntityDefinition&& definition);
        
        /*
        * Get all EntityDefinitions.
        */
        const std::vector<const EntityDefinition*> GetEntityDefinitions() const;

        /*
        * Returns last valid entityID.
        * Returns std::nullopt if there are no entity definitions.
        * All entity definitions with ID <= the returned ID are valid.
        */
        std::optional<size_t> GetMaxEntityID() const;

        /*
        * Get an entityID by it's name.
        * Returns std::nullopt if not found.
        */
        std::optional<size_t> GetEntityDefinitionID(const std::string& entityName) const;
        
        /*
        * Get EntityDefinition by name.
        * Returns nullptr if not found.
        * Pointers are valid until RemoveAllEntityDefinitions is called.
        */
        const EntityDefinition* GetEntityDefinition(const std::string& entityName) const {
            auto entityID = GetEntityDefinitionID(entityName);
            if (entityID) {
                return GetEntityDefinition(*entityID);
            }
            return nullptr;
        }

        /*
        * Get EntityDefinition by entityID.
        * Returns nullptr if not found.
        * Pointers are valid until RemoveAllEntityDefinitions is called.
        */
        const EntityDefinition* GetEntityDefinition(size_t entityID) const;

        /*
        * Clear EntityDefinition registry.
        * Automatically removes all EntityInstances.
        */
        void RemoveAllEntityDefinitions();

        /*
        * Create an entity defined by EntityDefinition and sets its properties.
        * 
        * Returns std::nullopt if the entity definition is not found or if the properties are invalid.
        */
        std::optional<InstanceID> CreateEntityInstance(size_t entityID, const PropertyValueVect& properties);

        /*
        * Get a vector of all currently existing instances.
        * Pointers are valid until their instance is removed.
        * If ignoredID is provided, the specific instanceID is not included.
        */
        std::vector<const EntityInstance*> GetEntityInstances(const std::optional<InstanceID> ignoredID = std::nullopt) const;

        /*
        * Get all valid InstanceIDs. 
        * If ignoredID is provided, the specific instanceID is not included.
        */
        std::vector<InstanceID> GetEntityInstanceIDs(const std::optional<InstanceID> ignoredID = std::nullopt) const;

        /*
        * Get all instances of a specific EntityDefinition
        * Pointers are valid until their instance is removed.
        * If ignoredID is provided, the specific instanceID is not included.
        */
        std::vector<const EntityInstance*> GetEntityInstancesByID(size_t entityID, std::optional<InstanceID> ignoredID = std::nullopt) const;

        /*
        * Get a specific EntityInstance
        * Returns nullptr if not found
        * Pointer is valid until the instance is removed.
        * If ignoredID is provided, the specific instanceID is nullptr.
        */
        EntityInstance* GetEntityInstance(InstanceID instanceID, std::optional<InstanceID> ignoredID = std::nullopt);

        /*
        * Get a specific EntityInstance
        * Returns nullptr if not found
        * Pointer is valid until the instance is removed.
        * If ignoredID is provided, the specific instanceID is nullptr.
        */
        const EntityInstance* GetEntityInstance(InstanceID instanceID, std::optional<InstanceID> ignoredID = std::nullopt) const;

        /*
        * Remove a specific EntityInstance from existance.
        */
        void RemoveEntityInstance(InstanceID instanceID);

        /*
        * Remove all EntityInstances.
        */
        void RemoveAllEntityInstances();

        /*
        * Get the SpriteCollection for a specific EntityDefinition.
        * Returns a placeholder SpriteCollection if the entityID is not found 
        * or if the entity definition doesn't have a SpriteCollection.
        */
        const SpriteCollection& GetSpriteCollection(size_t entityID) const;

        /*
        * Get the texture of a representative sprite for a specific EntityDefinition.
        * Returns a placeholder texture if the entityID is not found
        * or if the entity definition doesn't have a representative Sprite
        */
        const sf::Texture& GetRepresentativeTexture(size_t entityID) const;

        /*
        * Returns palette information for this manager
        */
        const EntityPalette& GetPalette() const {
            return _palette;
        };

        /*
        * Forcibly restores a deleted entity instance into its exact previous slot and generation.
        */
        bool RestoreEntityInstance(InstanceID instanceID, size_t entityID, const PropertyValueVect& properties);
};

#endif