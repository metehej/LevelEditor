#include "WorldTypes.h"

#include "EntityDefinitions.h"
#include "EntityInstances.h"
#include "TextureManager.h"

EntityDefinition::EntityDefinition(std::string name, EntityPlacementKind placementKind, EntityBindingKind bindingKind) 
        : name(name), _placementKind(placementKind), _bindingKind(bindingKind) {

    // Add position and size properties (required for all entities)
    Property posXProperty("posX", "X position on grid", PropertyType::Integer);
    posXProperty.required = true;
    Property posYProperty("posY", "Y position on grid", PropertyType::Integer);
    posYProperty.required = true;
    Property widthProperty("sizeX", "Width in grid cells", PropertyType::Integer);
    widthProperty.required = true;
    Property heightProperty("sizeY", "Height in grid cells", PropertyType::Integer);
    heightProperty.required = true;
    AddProperties({posXProperty, posYProperty, widthProperty, heightProperty});

    switch (placementKind) {
        case EntityPlacementKind::PathBased: {
            Property startXProperty("startX", "Start X position", PropertyType::Integer);
            startXProperty.required = true;
            Property startYProperty("startY", "Start Y position", PropertyType::Integer);
            startYProperty.required = true;

            Property endXProperty("endX", "End X position", PropertyType::Integer);
            endXProperty.required = true;
            Property endYProperty("endY", "End Y position", PropertyType::Integer);
            endYProperty.required = true; 
            
            AddProperties({startXProperty, startYProperty, endXProperty, endYProperty});
            break;
        }
        default:
            break;
    }

    switch (bindingKind) {
        case EntityBindingKind::Editable: {
            Property scaleXProperty("bindScaleX", "Binding scale X", PropertyType::Float);
            scaleXProperty.required = true;
            scaleXProperty.settable = true;
            scaleXProperty.defaultValue = 1.0f;
            scaleXProperty.minValue = 0.01f;
            scaleXProperty.maxValue = 1.0f;
            Property scaleYProperty("bindScaleY", "Binding scale Y", PropertyType::Float);
            scaleYProperty.required = true;
            scaleYProperty.settable = true;
            scaleYProperty.defaultValue = 1.0f;
            scaleYProperty.minValue = 0.01f;
            scaleYProperty.maxValue = 1.0f;

            Property bindOriginProperty = MakeEnumProperty<Origin>("bindOrigin", "Binding origin", Origin::Center, true, true);

            AddProperties({scaleXProperty, scaleYProperty, bindOriginProperty});
            break;
        }
        default:
            break;
    }
}

EntityDefinition& EntityDefinition::AddProperties(std::vector<Property>&& properties) {
    for (auto& property : properties) {
        std::string key = property.propertyName;
        bool req = property.required;
        auto it = _propertyDefinitions.insert_or_assign(std::move(key), std::move(property)).first;
        it->second.propertyName = it->first; 
        if (req) {
            _requiredProperties.push_back(it->first);
        }
    }
    return *this;
}

EntityDefinition& EntityDefinition::SetSpriteCollection(size_t collectionID, size_t representationTextureID) {
    if (_spriteCollectionID != 0) {
        LOG_I << "Sprite collection already set for entity " << name << std::endl;
        return *this;
    }
    _spriteCollectionID = collectionID;
    _representationTextureID = representationTextureID;
    return *this;
}

size_t EntityDefinition::GetSpriteCollection() const {
    return _spriteCollectionID;
}

size_t EntityDefinition::GetRepresentativeTexture() const {
    return _representationTextureID;
}

const std::vector<std::string>& EntityDefinition::GetRequiredProperties() const {
    return _requiredProperties;
}

PropertyValueVect EntityDefinition::GetDefaultProperties() const {
    PropertyValueVect defaultProperties;
    for (const auto& [name, prop] : _propertyDefinitions) {
        if (prop.defaultValue.has_value()) {
            defaultProperties.emplace_back(name, prop.defaultValue.value());
        }
    }
    return defaultProperties;
}

bool EntityDefinition::ValidateProperty(
        const std::string& propertyName, const PropertyValue& value, 
        const PropertyValueMap& allProperties) const {
    auto it = _propertyDefinitions.find(propertyName);
    if (it == _propertyDefinitions.end()) {
        return false;
    }
    const Property& prop = it->second;
    return prop.Validate(value, allProperties);
}

bool EntityDefinition::ValidateProperties(const PropertyValueMap& properties) const {
    for (const auto& prop : _propertyDefinitions) {
        const std::string& propName = prop.first;
        const Property& propDef = prop.second;

        auto it = properties.find(propName);
        if (it == properties.end()) {
            if (propDef.required) {
                return false;
            }
            continue;
        }
        const PropertyValue& value = it->second;
        if (!propDef.Validate(value, properties)) {
            return false;
        }
    }

    for (const auto& [name, _] : properties) {
        if (_propertyDefinitions.find(name) == _propertyDefinitions.end()) {
            return false;
        }
    }

    return true;
}

