#include "TextureManager.h"
#include "XMLManager.h"
#include "Config.h"
#include "PathHelper.h"

TextureManager::TextureManager() {
    sf::Image whiteImage;
    whiteImage.create(1, 1, sf::Color::White);

    sf::Texture whiteTexture;
    whiteTexture.loadFromImage(whiteImage);
    _whiteRectangleSprite.setTexture(whiteTexture);

    sf::Image magentaImage;
    magentaImage.create(1, 1, sf::Color::Magenta);

    sf::Texture defaultTexture;
    defaultTexture.loadFromImage(magentaImage);
    _textures.push_back(defaultTexture);
    _textureNameToID["placeholder"] = 0;

    Sprite defaultSprite;
    defaultSprite.name = "placeholder";
    defaultSprite.textureIDs.push_back(0);
    _sprites.push_back(defaultSprite);
    _spriteNameToID["placeholder"] = 0;

    SpriteCollection defaultCollection;
    defaultCollection.spriteIDs.push_back(0);
    defaultCollection.idToName[0] = "placeholder";
    _collections.push_back(defaultCollection);
    _collectionNameToID["placeholder"] = 0;
}

size_t TextureManager::LoadTexture(const std::string& fileName) {
    auto it = _textureNameToID.find(fileName);
    if (it != _textureNameToID.end()) {
        return it->second;
    }
    std::string path = GetAbsolutePath(Config::SPRITE_FILES_PATH + fileName);
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        LOG_W << "Failed to load texture from " << path << std::endl;
        return 0;
    }
    _textures.push_back(std::move(texture));
    size_t id = _textures.size() - 1;
    _textureNameToID[fileName] = id;
    return id;
}

const sf::Texture& TextureManager::GetTexture(const size_t metaID) const {
    if (metaID >= _textures.size()) {
        LOG_I << "Texture ID " << metaID << " not found." << std::endl;
        return _textures[0];
    }
    return _textures[metaID];
}

size_t TextureManager::LoadSprite(const std::string& fileName) {
    auto it = _spriteNameToID.find(fileName);
    if (it != _spriteNameToID.end()) {
        return it->second;
    }
    SpriteMetadata metadata;
    if (!XMLManager::LoadSprite(fileName, metadata)) {
        LOG_W << "Failed to load sprite metadata from " << fileName << std::endl;
        return 0;
    }
    Sprite sprite = metadata;
    for (const auto& filename : metadata.filenames) {
        size_t textureID = LoadTexture(filename);
        sprite.textureIDs.push_back(textureID);
    }
    _sprites.push_back(sprite);
    size_t id = _sprites.size() - 1;
    _spriteNameToID[fileName] = id;
    return id;
}

const Sprite& TextureManager::GetSprite(const size_t metaID) const {
    if (metaID >= _sprites.size()) {
        LOG_I << "Sprite ID " << metaID << " not found." << std::endl;
        return _sprites[0];
    }
    return _sprites[metaID];
}

size_t TextureManager::CreateCollection(const std::string& collectionName, const std::vector<std::string>& spriteNames) {
    auto existing = _collectionNameToID.find(collectionName);
    if (existing != _collectionNameToID.end()) {
        return existing->second;
    }
    std::vector<size_t> spriteIDs;
    for (int i = 0; i < spriteNames.size(); i++) {
        auto it = _spriteNameToID.find(spriteNames[i]);
        if (it == _spriteNameToID.end()) {
            spriteIDs.push_back(LoadSprite(spriteNames[i]));
            continue;
        }
        spriteIDs.push_back(it->second);
    }
    return CreateCollection(collectionName, spriteIDs);
}

size_t TextureManager::CreateCollection(const std::string& collectionName, const std::vector<size_t>& spriteIDs) {
    auto existing = _collectionNameToID.find(collectionName);
    if (existing != _collectionNameToID.end()) {
        return existing->second;
    }
    SpriteCollection collection;
    for (size_t spriteID : spriteIDs) {
        if (spriteID >= _sprites.size()) {
            collection.spriteIDs.push_back(0);
            collection.idToName[0] = "placeholder";
            collection.idToTag[0] = "placeholder";
            continue;
        }
        collection.spriteIDs.push_back(spriteID);
    }
    _collections.push_back(collection);
    size_t id = _collections.size() - 1;
    _collectionNameToID[collectionName] = id;
    return id;
}

size_t TextureManager::GetCollectionID(const std::string& collectionName) const {
    auto it = _collectionNameToID.find(collectionName);
    if (it == _collectionNameToID.end()) {
        LOG_I << "Collection name " << collectionName << " not found." << std::endl;
        return 0;
    }
    return it->second;
}

SpriteCollection& TextureManager::_GetCollection(size_t collectionID) {
    if (collectionID >= _collections.size()) {
        LOG_I << "Collection ID " << collectionID << " not found." << std::endl;
        return _collections[0];
    }
    return _collections[collectionID];
}

const SpriteCollection& TextureManager::_GetCollection(size_t collectionID) const {
    if (collectionID >= _collections.size()) {
        LOG_I << "Collection ID " << collectionID << " not found." << std::endl;
        return _collections[0];
    }
    return _collections[collectionID];
}

SpriteCollection& TextureManager::GetCollection(size_t collectionID) {
    return _GetCollection(collectionID);
}

const SpriteCollection& TextureManager::GetCollection(size_t collectionID) const {
    return _GetCollection(collectionID);
}

sf::Sprite TextureManager::GetWhiteRectangleSprite() const {
    return _whiteRectangleSprite;
}