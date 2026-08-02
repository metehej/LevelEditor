#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <SFML/Graphics.hpp>
#include <deque>
#include <unordered_map>
#include <iostream>
#include <optional>

class TextureManager;

struct Sprite {
    protected:
        Sprite() = default;
    public:
        std::string name; // Game-usable name for the sprite
        bool animated = false;
        float animationLength = 1.0f;
        bool loopAnimation = true;
        std::vector<size_t> textureIDs;

        const sf::Texture& GetTexture(const TextureManager& manager) const;
        
        /*
        * Get the first texture of this sprite.
        * The first texture is always representative of the sprite
        */
        const size_t GetTextureID() const {
            return textureIDs[0];
        }

        friend class TextureManager;
};

struct SpriteMetadata : Sprite {
    protected:
        SpriteMetadata() = default;

    public:
        std::vector<std::string> filenames;

        friend class TextureManager;
};

struct SpriteCollection {
    protected:
        SpriteCollection() = default;

        std::vector<size_t> spriteIDs;
        std::unordered_map<size_t, std::string> idToName; // Human-readable name for each sprite
        std::unordered_map<size_t, std::string> idToTag; // XML tag for each sprite
    public:

        const std::vector<size_t>& GetSpriteIDs() const {
            return spriteIDs;
        }

        void SetSpriteName(size_t spriteID, const std::string& name) {
            idToName[spriteID] = name;
        }

        std::string GetSpriteName(size_t spriteID) const {
            auto it = idToName.find(spriteID);
            if (it != idToName.end()) {
                return it->second;
            }
            return "Unknown";
        }
        
        void SetSpriteTag(size_t spriteID, const std::string& tag) {
            idToTag[spriteID] = tag;
        }

        std::string GetSpriteTag(size_t spriteID) const {
            auto it = idToTag.find(spriteID);
            if (it != idToTag.end()) {
                return it->second;
            }
            return "Unknown";
        }
    
        friend class TextureManager;
};

class TextureManager {
    private:
        sf::Sprite _whiteRectangleSprite;
        std::vector<sf::Texture> _textures;
        std::unordered_map<std::string, size_t> _textureNameToID;

        std::vector<Sprite> _sprites;
        std::unordered_map<std::string, size_t> _spriteNameToID;

        std::vector<SpriteCollection> _collections;
        std::unordered_map<std::string, size_t> _collectionNameToID;

        SpriteCollection& _GetCollection(size_t collectionID);
        const SpriteCollection& _GetCollection(size_t collectionID) const;

    public:
        TextureManager();

        /*
        * Load a Texture from a file and return its ID.
        * If the Texture is already loaded, return the existing Texture's ID.
        * Returns magenta placeholder TextureID 0 if loading fails.
        */
        size_t LoadTexture(const std::string& fileName);

        /*
        * Get a Texture by its ID.
        * Returns magenta placeholder Texture if texture not found.
        */
        const sf::Texture& GetTexture(const size_t metaID) const;

        /*
        * Load a Sprite from metadata file and return its ID.
        * If the Sprite is already loaded, return the existing Sprite's ID.
        * If any of Sprite Frames fail to load, Sprite will have placeholders.
        * Returns magenta placeholder SpriteID 0 if loading fails.
        */
        size_t LoadSprite(const std::string& fileName);

        /*
        * Get a Sprite by its ID.
        * Returns magenta placeholder Sprite if Sprite not found
        */
        const Sprite& GetSprite(const size_t metaID) const;

        /*
        * Create a SpriteCollection with the given name and sprite names.
        * Returns CollectionID of the created collection.
        * Returns placeholder CollectionID 0 if any sprite name is not found.
        * Returns existing CollectionID if collection name already exists.
        */
        size_t CreateCollection(const std::string& collectionName, const std::vector<std::string>& spriteNames);
        
        /*
        * Create a SpriteCollection with the given name and sprite IDs.
        * Returns CollectionID of the created collection.
        * Returns placeholder CollectionID 0 if any sprite ID is invalid.
        * Returns existing CollectionID if collection name already exists.
        */
        size_t CreateCollection(const std::string& collectionName, const std::vector<size_t>& spriteIDs);

        /*
        * Get a SpriteCollectionID by its name.
        * Returns placeholder CollectionID 0 if collection name not found.
        */
        size_t GetCollectionID(const std::string& collectionName) const;

        /*
        * Get a SpriteCollection by its ID.
        * Returns placeholder SpriteCollection if collection ID not found.
        */
        SpriteCollection& GetCollection(size_t collectionID);

        const SpriteCollection& GetCollection(size_t collectionID) const;

        sf::Sprite GetWhiteRectangleSprite() const;
};
#endif