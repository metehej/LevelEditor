#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

#include <exception>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <future>

#include "Types.h"
#include "Editor.h"
#include "TextureManager.h"
#include "EntityManager.h"
#include "XMLManager.h"
#include "LevelEditorScreen.h"
#include "GridManager.h"
#include "CommandManager.h"
#include "RenderManager.h"
#include "LevelManager.h"



class LevelInitializationException : public std::exception {
    private:
        std::string _message;
    public:
        LevelInitializationException(const std::string& message) : _message(message) {}
        const char* what() const noexcept override {
            return _message.c_str();
        }
};

enum class FileOperationState {
    LoadingStart,
    LoadingEntities,
    LoadingLevelData,
    SavingStart,
    SavingLevelData,
    Done
};

class LevelEditor : public Editor {
    private:
        TextureManager _textureManager; // Manages loaded textures and sprites
        EntityManager _entityManager; // Manages entity definitions and instances
        CommandManager _commandManager; // Manages editing commands
        RenderManager _renderManager; // Handles rendering of the level
        LevelManager _levelManager; // Manages transient level data and logic

        LevelData _levelData;

        std::future<std::optional<FileLevelData>> _loadLevelDataFuture;
        std::future<std::vector<FileEntityDefinitionData>> _loadEntitiesFuture;
        std::future<bool> _saveLevelFuture;

        FileOperationState _operationProgress = FileOperationState::LoadingStart;

        LevelEditorScreen _screen;

        std::unordered_map<UIEventType, UIEventHandler> _uiEventHandlers;
        void _setUIEventHandlers();

        std::unordered_map<GridEventType, GridEventHandler> _gridEventHandlers;
        void _setGridEventHandlers();

        /*
        * Sets basic handlers for situations that don't require a grid.
        */
        void _setCommonHandlers();

        /*
        * Checks the state of currently loaded item.
        * Returns true if loading is done.
        */
        bool _updateLoading();

    public:
        LevelEditor(std::string levelPath, sf::RenderWindow& window);

        /*
        * Executes the main editor loop once.
        * Empties event queues after processing.
        * Returns the next app state and context info.
        */
        AppContext Run(std::vector<sf::Event>& events, sf::Time deltaTime) override;

        void OnExit() override;
};
#endif