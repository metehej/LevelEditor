#ifndef LEVELDATA_H
#define LEVELDATA_H

#include "Properties.h"
#include "XMLManager.h"

struct LevelData{
    PropertyMap propertyDefinitions;
    PropertyValueMap properties;
    std::string originalFileName; 

    LevelData(std::string levelName = "") : originalFileName(levelName.empty() ? XMLManager::GetValidLevelFileName() : levelName) {
        propertyDefinitions.emplace("gridSizeX", Property("gridSizeX", "Grid width in cells", PropertyType::Integer));
        propertyDefinitions["gridSizeX"].required = true;
        propertyDefinitions["gridSizeX"].settable = false; // Prevent changing, not supported by engine.
        propertyDefinitions["gridSizeX"].defaultValue = Config::DEFAULT_GRID_SIZE_X;

        propertyDefinitions.emplace("gridSizeY", Property("gridSizeY", "Grid height in cells", PropertyType::Integer));
        propertyDefinitions["gridSizeY"].required = true;
        propertyDefinitions["gridSizeY"].settable = false; // Prevent changing, not supported by engine.
        propertyDefinitions["gridSizeY"].defaultValue = Config::DEFAULT_GRID_SIZE_Y;

        propertyDefinitions.emplace("levelName", Property("levelName", "Level name", PropertyType::String));
        propertyDefinitions["levelName"].required = true;
        propertyDefinitions["levelName"].settable = true;
        propertyDefinitions["levelName"].defaultValue = "Untitled";

        propertyDefinitions.emplace("fileName", Property("fileName", "Level file name", PropertyType::String));
        propertyDefinitions["fileName"].required = true;
        propertyDefinitions["fileName"].settable = true;
        propertyDefinitions["fileName"].defaultValue = "untitled";
        propertyDefinitions["fileName"].validator = [name = originalFileName](const PropertyValue& value, const PropertyValueMap&) {
            if (!IsStringValue(value)) {
                return false;
            }
            const std::string& strValue = std::get<std::string>(value);
            if (strValue.empty() || strValue == "." || strValue == "..") return false;
            if (strValue.length() > 255) return false;
            auto isSafe = [](char c) {
                return std::isalnum(c) || c == '-' || c == '_';
            };
            for (char c : strValue) {
                if (!isSafe(c)) {
                    return false;
                }
            }
            return XMLManager::LevelFileExists(strValue) ? (strValue == name) : true;
        };

        propertyDefinitions.emplace("backgroundColor", Property("backgroundColor", "Background color (hex)", PropertyType::String));
        propertyDefinitions["backgroundColor"].required = false;
        propertyDefinitions["backgroundColor"].settable = true;
        propertyDefinitions["backgroundColor"].defaultValue = "#000000";
        propertyDefinitions["backgroundColor"].validator = [](const PropertyValue& value, const PropertyValueMap&) {
            if (!IsStringValue(value)) {
                return false;
            }
            const std::string& strValue = std::get<std::string>(value);
            if (strValue.size() == 0){
                return true;
            }
            try {
                ColorHelper::HexToColor(strValue);
                return true;
            } catch (const std::exception&) {
                return false;
            }
        };

        propertyDefinitions.emplace("airSupply", Property("airSupply", "Air supply for the player", PropertyType::Integer));
        propertyDefinitions["airSupply"].required = true;
        propertyDefinitions["airSupply"].settable = true;
        propertyDefinitions["airSupply"].defaultValue = 100;
        propertyDefinitions["airSupply"].minValue = 0;
        
        propertyDefinitions.emplace("airDecay", Property("airDecay", "Air decay per second", PropertyType::Float));
        propertyDefinitions["airDecay"].required = true;
        propertyDefinitions["airDecay"].settable = true;
        propertyDefinitions["airDecay"].defaultValue = 1.0f;
        propertyDefinitions["airDecay"].minValue = 0.0;
        
        for (const auto& [name, prop] : propertyDefinitions) {
            properties.emplace(name, prop.defaultValue.value());
        }
    }
    
    void SetProperty(const std::string& propertyName, const PropertyValue& value) {
        if (properties.find(propertyName) == properties.end()) {
            return;
        }
        auto definition = propertyDefinitions.find(propertyName);
        if (definition == propertyDefinitions.end()) {
            return;
        }
        if (!definition->second.Validate(value, properties)) {
            return;
        }
        properties[propertyName] = value;
    }
};

#endif