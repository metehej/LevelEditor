#include "XMLManager.h"

#include "PathHelper.h"
#include "Config.h"
#include "Types.h"
#include "LevelData.h"
#include "EntityManager.h"
#include "TextureManager.h"
#include "TransientTypes.h"
#include "ColorHelper.h"
#include <iostream>
#include <filesystem>

namespace XMLManager {
    namespace {
        std::pair<EntityPlacementKind, EntityBindingKind> _getEntityKinds(const std::string& typeName) {
            std::pair<EntityPlacementKind, EntityBindingKind> result;
            result.second = EntityBindingKind::Editable;

            if (typeName.empty()) {
                result.first = EntityPlacementKind::Static;
                return result;
            }

            switch (typeName[0]) {
                case 'W':
                    result.first = EntityPlacementKind::ExpandOmni;
                    result.second = EntityBindingKind::NonEditable;
                    break;
                case 'B':
                case 'P':
                    result.first = EntityPlacementKind::ExpandHorizontal;
                    result.second = EntityBindingKind::NonEditable;
                    break;
                case 'D':
                    if (typeName == "Door") {
                        result.first = EntityPlacementKind::Singleton;
                        result.second = EntityBindingKind::NonEditable;
                    } else {
                        result.first = EntityPlacementKind::PathBased;
                    }
                    break;
                case 'C':
                    result.first = EntityPlacementKind::Singleton;
                    break;
                default:
                    result.first = EntityPlacementKind::Static;
                    break;
            }

            return result;
        }

        bool _getDocument(const std::string& filePath, pugi::xml_document& doc) {
            auto absoluteFilePath =  GetAbsolutePath(filePath);
            if (!doc.load_file(absoluteFilePath.c_str())) {
                LOG_E << "Failed to load XML document from " << filePath << std::endl;
                return false;
            }
            return true;
        }
    }

    bool SaveLevel(FileLevelData&& levelData) {
        auto& ld = levelData.levelData;
        std::string fileName = GetTypedValue<std::string>("fileName", ld.properties)
            .value_or(XMLManager::GetValidLevelFileName());
        pugi::xml_document doc;    
        auto root = doc.append_child("Level");

        auto decl = doc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "UTF-8";

        root.append_attribute("levelName") = GetTypedValue<std::string>("levelName", ld.properties).value_or("Untitled").c_str();
        root.append_attribute("gridSizeX") = GetTypedValue<int>("gridSizeX", ld.properties).value_or(Config::DEFAULT_GRID_SIZE_X);
        root.append_attribute("gridSizeY") = GetTypedValue<int>("gridSizeY", ld.properties).value_or(Config::DEFAULT_GRID_SIZE_Y);
        root.append_attribute("backgroundColor") = GetTypedValue<std::string>("backgroundColor", ld.properties)
            .value_or(ColorHelper::ColorToHex(Config::BACKGROUND_COLOR)).c_str();
        root.append_attribute("airDecay") = GetTypedValue<float>("airDecay", ld.properties).value_or(0.0f);
        root.append_attribute("airSupply") = GetTypedValue<int>("airSupply", ld.properties).value_or(100);

        auto entitiesNode = root.append_child("Entities");

        for (const auto& entityData : levelData.entitiesData) {
            auto entityNode = entitiesNode.append_child("Entity");
            entityNode.append_attribute("name") = entityData.name.c_str();
            for (const auto& instanceProperties : entityData.instancesProperties) {
                auto instanceNode = entityNode.append_child("Instance");
                for (const auto& property : instanceProperties) {
                    auto propertyNode = instanceNode.append_child("Property");
                    propertyNode.append_attribute("name") = property.first.c_str();
                    propertyNode.append_attribute("value") = property.second.c_str();
                }
            }
        }

        std::string filePath = Config::LEVELS_PATH + fileName + ".xml";
        if (!doc.save_file(GetAbsolutePath(filePath).c_str())) {
            LOG_E << "Failed to save level to " << filePath << std::endl;
            return false;
        }

        std::string& originalFileName = ld.originalFileName;
        if (originalFileName != fileName && LevelFileExists(originalFileName)) {
            LOG_I << "Deleting old level file " << originalFileName << std::endl;
            if (!DeleteLevel(originalFileName)) {
                LOG_W << "Failed to delete old level file " << originalFileName << std::endl;
            }
        }

        return true;
    }

    std::optional<FileLevelData> LoadLevel(std::string fileName) {
        if (fileName.empty()) {
            LOG_E << "Level file name is empty." << std::endl;
            return std::nullopt;
        }
        pugi::xml_document doc;
        if (!_getDocument(Config::LEVELS_PATH + fileName + ".xml", doc)) {
            LOG_E << "Failed to load level file " << fileName << std::endl;
            return std::nullopt;
        }
        auto root = doc.child("Level");
        if (!root) {
            LOG_E << "Level file " << fileName << " missing Level node." << std::endl;
            return std::nullopt;
        }
        LevelData levelData(fileName);
        levelData.properties["fileName"] = fileName;
        levelData.properties["levelName"] = std::string(root.attribute("levelName").as_string("Untitled"));
        levelData.properties["gridSizeX"] = root.attribute("gridSizeX").as_int(Config::DEFAULT_GRID_SIZE_X);
        levelData.properties["gridSizeY"] = root.attribute("gridSizeY").as_int(Config::DEFAULT_GRID_SIZE_Y);
        levelData.properties["backgroundColor"] = std::string(root.attribute("backgroundColor").as_string(ColorHelper::ColorToHex(Config::BACKGROUND_COLOR).c_str()));
        levelData.properties["airDecay"] = root.attribute("airDecay").as_float(0.0f);
        levelData.properties["airSupply"] = root.attribute("airSupply").as_int(100);

        FileLevelData fLevelData {
            .levelData = std::move(levelData)
        };
        auto entitiesNode = root.child("Entities");
        if (!entitiesNode) {
            LOG_W << "Level file " << fileName << " missing Entities node." << std::endl;
            return fLevelData;
        }
        for (pugi::xml_node entityNode : entitiesNode.children("Entity")) {
            FileEntityData entityData;
            entityData.name = entityNode.attribute("name").as_string();
            for (pugi::xml_node instanceNode : entityNode.children("Instance")) {
                std::unordered_map<std::string, std::string> instanceProperties;
                for (pugi::xml_node propertyNode : instanceNode.children("Property")) {
                    std::string propertyName = propertyNode.attribute("name").as_string();
                    std::string propertyValueStr = propertyNode.attribute("value").as_string();
                    instanceProperties[propertyName] = propertyValueStr;
                }
                entityData.instancesProperties.push_back(std::move(instanceProperties));
            }      
            fLevelData.entitiesData.push_back(std::move(entityData));  
        }
        return fLevelData;
    }

    bool LevelFileExists(const std::string& fileName) {
        return std::filesystem::exists(GetAbsolutePath(Config::LEVELS_PATH + fileName + ".xml"));
    }

    bool CreateEmptyLevel(const std::string& fileName) {
        if (LevelFileExists(fileName)) {
            LOG_W << "Level file " << fileName << " already exists." << std::endl;
            return false;
        }
        FileLevelData emptyLevelData;
        return SaveLevel(std::move(emptyLevelData));
    }

    bool DeleteLevel(const std::string& fileName) {
        std::string filePath = GetAbsolutePath(Config::LEVELS_PATH + fileName + ".xml");
        if (!std::filesystem::exists(filePath)) {
            return true; // Consider it a success if the file doesn't exist.
        }
        try {
            return std::filesystem::remove(filePath);
        } catch (const std::exception& e) {
            LOG_E << "Error deleting level file " << fileName << ": " << e.what() << std::endl;
            return false;
        }
    }

    std::string GetValidLevelFileName() {
        std::string baseName = "untitled";
        std::string fileName = baseName;
        int counter = 1;
        while (LevelFileExists(fileName)) {
            fileName = baseName + std::to_string(counter);
            counter++;
        }
        return fileName;
    }

    std::vector<FileEntityDefinitionData> LoadEntities() {
        std::vector<FileEntityDefinitionData> entitiesData;
        for (const auto& entry : std::filesystem::directory_iterator(GetAbsolutePath(Config::ENTITIES_PATH))) {
            if (entry.is_regular_file() && entry.path().extension() == ".xml") {
                pugi::xml_document doc;
                if (!_getDocument(Config::ENTITIES_PATH + entry.path().filename().string(), doc)) {
                    LOG_W << "Failed to load entity metadata from " << entry.path() << std::endl;
                    continue;
                }
                pugi::xml_node entityNode = doc.child("Entity");
                if (!entityNode) {
                    LOG_W << "Entity metadata file " << entry.path() << " missing Entity node." << std::endl;
                    continue;
                }

                std::string name = entityNode.attribute("name").as_string();
                std::string type = entityNode.attribute("type").as_string();
                auto [placementKind, bindingKind] = _getEntityKinds(type);
                EntityDefinition definition{name, placementKind, bindingKind};
                definition.type = type;
                std::string width = entityNode.attribute("width").as_string("1");
                std::string height = entityNode.attribute("height").as_string("1");
                definition.SetDefaultPropertyValue("sizeX", width);
                definition.SetDefaultPropertyValue("sizeY", height);

                FileEntityDefinitionData entityData{
                    .definition = std::move(definition)
                };

                for (pugi::xml_node spriteNode : entityNode.children("Sprite")) {
                    std::string spriteFileName = spriteNode.attribute("fileName").as_string();
                    std::string spriteTag = spriteNode.attribute("tag").as_string();
                    std::string spriteHumanName = spriteNode.attribute("humanName").as_string();
                    entityData.sprites.push_back({spriteFileName, spriteTag, spriteHumanName});
                }
                
                std::vector<Property> properties;
                for (pugi::xml_node propertyNode : entityNode.children("Property")) {
                    std::string propertyName = propertyNode.attribute("name").as_string();
                    std::string humanName = propertyNode.attribute("hName").as_string();
                    std::string propertyType = propertyNode.attribute("type").as_string();
                    std::string defaultValue = propertyNode.attribute("defaultValue").as_string();
                    auto minValue = propertyNode.attribute("minValue");
                    auto maxValue = propertyNode.attribute("maxValue");
                    std::vector<std::string> allowedValues;
                    for (pugi::xml_node allowedValueNode : propertyNode.children("AllowedValue")) {
                        allowedValues.push_back(allowedValueNode.attribute("value").as_string());
                    }
                    Property property{propertyName, humanName, ToPropertyType(propertyType)};
                    if (!property.SetDefault(defaultValue)) {
                        property.required = true;
                    }
                    if (minValue) {
                        property.minValue = minValue.as_double(std::numeric_limits<int>::lowest());
                    }
                    if (maxValue) {
                        property.maxValue = maxValue.as_double(std::numeric_limits<int>::max());
                    }
                    if (!allowedValues.empty()) {
                        property.allowedValues = allowedValues;
                    }
                    property.settable = true; // Properties from XML must be set in property panel.
                    properties.push_back(property);
                }
                definition.AddProperties(std::move(properties));
                entitiesData.push_back(std::move(entityData));
            }
        }
        return entitiesData;
    }

    bool LoadSprite(const std::string& fileName, SpriteMetadata& metadata) {
        try {
            pugi::xml_document doc;
            if (!_getDocument(Config::SPRITE_META_PATH + fileName, doc)) {
                return false;
            }
            pugi::xml_node metaNode = doc.child("Sprite");
            metadata.name = metaNode.attribute("name").as_string();
            metadata.animated = metaNode.attribute("animated").as_bool();
            if (metadata.animated) {
                auto animationNode = metaNode.child("Animation");
                if (animationNode) {
                    metadata.animationLength = animationNode.attribute("length").as_float();
                    metadata.loopAnimation = animationNode.attribute("loop").as_bool();
                } else {
                    LOG_W << "Animated sprite " << metadata.name << " metadata missing Animation node." << std::endl;
                    return false;
                }
            }
            auto framesNode = metaNode.child("Frames");
            if (!framesNode) {
                LOG_W << metadata.name << " Sprite metadata missing Frames node." << std::endl;
                return false;
            }
            auto frames = framesNode.children("Frame");
            metadata.filenames.resize(std::distance(frames.begin(), frames.end()));
            for (pugi::xml_node frameNode : frames) {
                std::string filename = frameNode.attribute("name").as_string();
                int index = frameNode.attribute("index").as_int();
                metadata.filenames[index] = filename;
            }
            return true;
        } catch (const std::exception& e) {
            LOG_W << "Error loading sprite metadata: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool ExportTextureRegions(std::vector<TextureRegion>& regions, const std::string& outputPng, const std::string& outputXml) {
        sf::RenderTexture textureMap;
        
        if (!textureMap.create(Config::REGIONS_MAX_SIZE, Config::REGIONS_MAX_SIZE)) {
            LOG_E << "Failed to create render texture for exporting texture regions.\n";
            return false;
        }
        textureMap.clear(sf::Color::Transparent);

        std::sort(regions.begin(), regions.end(), [](const TextureRegion& a, const TextureRegion& b) {
            if (a.size.y == b.size.y) {
                return a.size.x > b.size.x;
            }
            return a.size.y > b.size.y; 
        });

        std::unordered_map<std::string, sf::Texture> loadedTextures;

        pugi::xml_document doc;
        pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "UTF-8";

        pugi::xml_node root = doc.append_child("Texture");
        root.append_attribute("name") = "textures";

        int currentX = 0;
        int currentY = 0;
        int rowHeight = 0;
        const int padding = Config::REGION_PADDING;

        for (const auto&  region : regions) {
            sf::Texture tempTex;
            if (loadedTextures.find(region.fileName) == loadedTextures.end()) {
                if (!tempTex.loadFromFile(GetAbsolutePath(Config::SPRITE_FILES_PATH + region.fileName + ".png"))) {
                    std::cerr << "WARNING: Could not load " <<  region.fileName << ". Skipping.\n";
                    continue;
                }
                tempTex.setSmooth(false);
                loadedTextures[region.fileName] = tempTex;
            }

            const sf::Texture& texture = loadedTextures[region.fileName];
            int texWidth = tempTex.getSize().x;
            int texHeight = tempTex.getSize().y;

            float scaleX = static_cast<float>(region.size.x) / texWidth;
            float scaleY = static_cast<float>(region.size.y) / texHeight;

            if (currentX + region.size.x + padding > Config::REGIONS_MAX_SIZE) {
                currentX = 0;
                currentY += rowHeight + padding;
                rowHeight = 0;
            }

            if (currentY + region.size.y > Config::REGIONS_MAX_SIZE) {
                LOG_E << "Texture regions exceed maximum atlas size. Cannot export.\n";
                return false;
            }

            sf::Sprite sprite(tempTex);
            sprite.setScale(scaleX, scaleY);
            sprite.setPosition(static_cast<float>(currentX), static_cast<float>(currentY));
            textureMap.draw(sprite);

            pugi::xml_node regionNode = root.append_child("TextureRegion");
            regionNode.append_attribute("name") =  region.GetID().c_str();
            regionNode.append_attribute("x") = currentX;
            regionNode.append_attribute("y") = currentY;
            regionNode.append_attribute("w") = region.size.x;
            regionNode.append_attribute("h") = region.size.y;

            // Advance the cursor for the next sprite
            currentX += region.size.x + padding;
            if (region.size.y > rowHeight) {
                rowHeight = region.size.y;
            }
        }

        textureMap.display();

        sf::Image finalImage = textureMap.getTexture().copyToImage();
        if (!finalImage.saveToFile(outputPng)) {
            LOG_E << "Failed to save textures to " << outputPng << "\n";
            return false;
        }

        if (!doc.save_file(outputXml.c_str())) {
            LOG_E << "Failed to save texture regions metadata to " << outputXml << "\n";
            return false;
        }
        return true;
    }
}