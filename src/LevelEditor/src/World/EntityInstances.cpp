#include <stdexcept>
#include <math.h>

#include "EntityInstances.h"
#include "EntityManager.h"
#include "TextureManager.h"
#include "EntityDefinitions.h"

void EntityInstance::_extractPosSizeFromProperties() {
    auto posXIt = _properties.find("posX");
    auto posYIt = _properties.find("posY");
    auto widthIt = _properties.find("sizeX");
    auto heightIt = _properties.find("sizeY");

    if (posXIt != _properties.end() && posYIt != _properties.end()) {
        _position.x = static_cast<size_t>(ToNumber(posXIt->second));
        _position.y = static_cast<size_t>(ToNumber(posYIt->second));
    } else {
        LOG_W << "Failed to extract position from properties: posX and posY are required." << std::endl;
    }

    if (widthIt != _properties.end() && heightIt != _properties.end()) {
        _size.x = static_cast<size_t>(ToNumber(widthIt->second));
        _size.y = static_cast<size_t>(ToNumber(heightIt->second));
    } else {
        LOG_W << "Failed to extract size from properties: width and height are required." << std::endl;
    }
}

EntityInstance::EntityInstance(const size_t entityID, const EntityManager& manager) : entityID(entityID), _entityManager(manager) {
    const EntityDefinition* entityDef = _entityManager.GetEntityDefinition(entityID);
    if (!entityDef) {
        throw std::invalid_argument("Entity definition with ID '" + std::to_string(entityID) + "' not found.");
    }
    auto defaults = entityDef->GetDefaultProperties();
    for (const auto& [propName, propValue] : defaults) {
        _properties[propName] = propValue;
    }
    _textureID = entityDef->GetRepresentativeTexture();
}

EntityInstance EntityInstance::GetClone() const {
    return *this;
}

bool EntityInstance::ApplyProperties(const PropertyValueVect& properties, bool skipValidator, bool cancelOnInvalid) {
    PropertyValueMap newProperties = _properties;
    auto* entityDef = _entityManager.GetEntityDefinition(entityID);
    if (!entityDef) {
        LOG_W << "Failed to apply properties: Entity definition with ID '" << entityID << "' not found." << std::endl;
        return false;
    }
    const auto& propertyDefs = entityDef->GetPropertyDefinitions();
    // Check names and types
    for (const auto& [propName, propValue] : properties) {
        auto it = propertyDefs.find(propName);
        if (it == propertyDefs.end()) {
            LOG_I << "Property '" << propName << "' not found in entity definition." << std::endl;
            if (cancelOnInvalid) {
                return false;
            }
            continue;
        }
        const Property& propDef = it->second;
        if (!MatchesKind(propDef.type, propValue)) {
            LOG_I << "Invalid value for property '" << propName << "'." << std::endl;
            if (cancelOnInvalid) {
                return false;
            }
            continue;
        }
        newProperties[propName] = propValue;
    }
    if (skipValidator) {
        _properties = newProperties;
        _extractPosSizeFromProperties();
        return true;
    }
    // Full validation
    if (cancelOnInvalid && !entityDef->ValidateProperties(newProperties)) {
        LOG_I << "Invalid properties for entity instance of entityID '" << entityID << "'." << std::endl;
        return false;
    }
    while(!entityDef->ValidateProperties(newProperties)) {
        // Find and erase the first invalid property
        bool foundInvalid = false;
        for (const auto& [propName, propValue] : newProperties) {
            if (!entityDef->ValidateProperty(propName, propValue, newProperties)) {
                LOG_I << "Invalid value for property " << propName << "." << std::endl;
                newProperties.erase(propName);
                auto it = _properties.find(propName);
                if (it != _properties.end() && entityDef->ValidateProperty(propName, it->second, newProperties)) {
                    newProperties[propName] = it->second;
                }
                foundInvalid = true;
                break;
            }
        }
        if (!foundInvalid) {
            // Prevent an infinite loop
            LOG_W << "Failed to validate properties, but no invalid property found. This may indicate a problem with the validators." << std::endl;
            return false;
        }
    }
    _properties = newProperties;
    _extractPosSizeFromProperties();
    return true;
}

const PropertyValueMap& EntityInstance::GetPropertiesMap() const {
    return _properties;
}

std::optional<PropertyValue> EntityInstance::GetProperty(const std::string& propertyName) const {
    auto it = _properties.find(propertyName);
    if (it == _properties.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool EntityInstance::operator==(const EntityInstance& other) const{
    return entityID == other.entityID;
}

TextureRenderInfo EntityInstance::GetTextureRenderInfo() const {
    return TextureRenderInfo{
        .textureID = _textureID,
        .startTile = _position,
        .size = _size,
    };
}