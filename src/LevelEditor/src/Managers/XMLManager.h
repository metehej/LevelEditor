#ifndef XMLMANAGER_H
#define XMLMANAGER_H

#include <memory>
#include <pugixml.hpp>
#include <string>
#include <vector>

#include "EntityManager.h"
#include "TextureManager.h"
#include "Types.h"
#include "WorldTypes.h"

struct LevelData;
struct FileLevelData;
struct FileEntityDefinitionData;
struct LevelView;
struct TextureRegion;

namespace XMLManager {

/*
 * Save the current grid state to an XML file.
 */
bool SaveLevel(FileLevelData &&levelData);

/*
 * Load Level metadata from an XML file.
 */
std::optional<FileLevelData> LoadLevel(const std::string fileName);

/*
 * Check if a level file exists.
 */
bool LevelFileExists(const std::string &fileName);

/*
 * Create an empty level file with the given name.
 * Returns false if file already exists or if any error occurs.
 */
bool CreateEmptyLevel(const std::string &fileName);

/*
 * Delete a level.
 * Does not fail on nonexistent file, but returns false on other errors.
 */
bool DeleteLevel(const std::string &fileName);

/*
 * Returns a vector of LevelViews for all local levels.
 */
std::vector<LevelView> ListAllLevels();

/*
 * Returns a vector of LevelViews for levels present in loadout.xml.
 */
std::vector<LevelView> ListLoadoutLevels();

/*
 * Save loadout to a local file.
 */
bool SaveLoadout(const std::vector<LevelView> &levelFileNames);

/*
 * Exports levels to a local file for the game to use.
 */
bool ExportLoadout(const std::vector<LevelView> &levelFileNames);

/*
 * Generates a Level file name that can be used for a new level.
 */
std::string GetValidLevelFileName();

/*
 * Load Entities from the folder specified in Config::ENTITIES_PATH.
 * Returns an empty vector if no entity files are found
 */
std::vector<FileEntityDefinitionData> LoadEntities();

/*
 * Load sprite metadata from an external XML
 */
bool LoadSprite(const std::string &fileName, SpriteMetadata &metadata);

/*
 * Builds a texture map from requested texture regions.
 */
bool ExportTextureRegions(std::vector<TextureRegion> &regions,
                          const std::string &outputFilePath);
}; // namespace XMLManager

#endif