#ifndef ENTITIES_H
#define ENTITIES_H

#include <map>

#include "EntityDefinitions.h"
#include "SFML/Graphics.hpp"
#include "RenderTypes.h"

class EntityManager;
class TextureManager;

/*
 * Represents an instance on the grid. 
 */
class EntityInstance {
    private:
        const EntityManager& _entityManager;
        PropertyValueMap _properties;
        Vector2<size_t> _position;
        Vector2<size_t> _size;
        size_t _textureID;

        /*
        * Applies properties to a copy of the instance's map and validates the result.
        * If skipValidator is true, only check names and value types.
        */
        bool _validateProperties(const PropertyValueVect& properties, bool skipValidator = false) const;

        /*
        * Extracts position and size from property map to internal variables.
        * Should be called after properties are applied to map.
        */
        void _extractPosSizeFromProperties();
    public:
        const size_t entityID;

        EntityInstance(const size_t entityID, const EntityManager& manager);

        /*
        * Create a clone instance.
        */
        EntityInstance GetClone() const;

        /*
        * Save each property into internal storage.
        * If skipValidator is true, skips property validation and only checks property types.
        * If cancelOnInvalid is true, properties are not applied if validation fails. 
        * Otherwise, invalid properties are skipped and valid ones are applied.
        * Invalid properties may cause undefined behavior in other functions.
        */
        bool ApplyProperties(const PropertyValueVect& properties, bool skipValidator = false, bool cancelOnInvalid = true);

        /*
        * Get internally stored properties as a map
        */
        const PropertyValueMap& GetPropertiesMap() const;

        /*
        * Get a specific property value
        */
        std::optional<PropertyValue> GetProperty(const std::string& propertyName) const;

        /*
        * Check if the instances refers to the same entity type.
        * Doesn't compare other parameters.
        */
        bool operator==(const EntityInstance& other) const;

        /*
        * Get Render information for this instance.
        * Does not insert
        */
        TextureRenderInfo GetTextureRenderInfo() const;
};

#endif