#ifndef ENTITY_DEFINITIONS_H
#define ENTITY_DEFINITIONS_H

#include <functional>
#include <limits>
#include <map>
#include <unordered_map>
#include <string>
#include <optional>
#include <variant>
#include <iostream>

#include "Types.h"
#include "Properties.h"
#include "WorldTypes.h"
#include "RenderTypes.h"

class TextureManager;
class EntityInstance;

struct EntityDefinition {
    private:
        PropertyMap _propertyDefinitions;
        std::vector<std::string> _requiredProperties;
        EntityPlacementKind _placementKind = EntityPlacementKind::Static;
        EntityBindingKind _bindingKind = EntityBindingKind::Editable;
        size_t _spriteCollectionID = 0;
        size_t _representationTextureID = 0;
    public:

        std::string name;
        std::string type;

        const EntityPlacementKind GetPlacementKind() const {
            return _placementKind;
        }

        const EntityBindingKind GetBindingKind() const {
            return _bindingKind;
        }

        EntityDefinition(std::string name, EntityPlacementKind placementKind, EntityBindingKind bindingKind);

        /*
        * Add properties to the entity definition. Returns a reference to the modified EntityDefinition.
        */
        EntityDefinition& AddProperties(std::vector<Property>&& properties);

        /*
        * Allows setting default value for properties after they were added.
        * Necessary for automatic properties.
        */
        EntityDefinition& SetDefaultPropertyValue(const std::string& propertyName, const std::string& defaultValue) {
            auto it = _propertyDefinitions.find(propertyName);
            if (it == _propertyDefinitions.end()) {
                LOG_I << "Property " << propertyName << " not found in entity definition " << name << std::endl;
                return *this;
            }
            it->second.SetDefault(defaultValue);
            return *this;
        }
        /*
        * Set the sprite collection for this entity. Returns a reference to the modified EntityDefinition.
        * Collection can only be set once.
        * representativeSpriteID specifies spriteID of the rendered sprite for this entity.
        */
        EntityDefinition& SetSpriteCollection(size_t collectionID, size_t representationSpriteID);

        /*
        * Get the collectionID for this EntityDefinition.
        * Returns 0 for missing setup.
        */
        size_t GetSpriteCollection() const;

        /*
        * Get the representative textureID for this entity.
        * Returns 0 fro missing setup.*/
        size_t GetRepresentativeTexture() const;

        /*
        * Get the Property definitions this entity has.
        */
        const PropertyMap& GetPropertyDefinitions() const {
            return _propertyDefinitions;
        }

        /*
        * Get the required properties for this entity.
        */
        const std::vector<std::string>& GetRequiredProperties() const;

        /*
        * Get the default values for this entity's properties.
        * Only returns the properties that have default values.
        */
        PropertyValueVect GetDefaultProperties() const;

        /*
        * Validate a single property value against the entity definition.
        * If the property is not defined for this entity, returns false.
        */
        bool ValidateProperty(
            const std::string& propertyName, const PropertyValue& value, 
            const PropertyValueMap& allProperties) const;
        
        /*
        * Validate a set of property values against the entity definition.
        * Returns false if any property is invalid or if any required property is missing.
        */
        bool ValidateProperties(const PropertyValueMap& properties) const;
};

#endif