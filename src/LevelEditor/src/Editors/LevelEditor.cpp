#include "LevelEditor.h"

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>

#include "Config.h"
#include "Properties.h"
#include "TransientTypes.h"
#include "CommandTypes.h"
#include "LevelEditorScreen.h"
#include "LevelData.h"

LevelEditor::LevelEditor(std::string levelName, sf::RenderWindow& window) 
        : _entityManager(_textureManager), _screen(window) {

    _screen.RenderCommon();
    _levelData = LevelData(levelName);
    _setCommonHandlers();
    _nextState = AppState::LevelEditor;
}

bool LevelEditor::_updateLoading() {\
    if (_operationProgress == FileOperationState::Done) {
        return true;
    }
    switch (_operationProgress) {
        case FileOperationState::LoadingStart: {
            _loadEntitiesFuture = std::async(std::launch::async, XMLManager::LoadEntities);
            _loadLevelDataFuture = std::async(std::launch::async, XMLManager::LoadLevel, _levelData.originalFileName);
            _operationProgress = FileOperationState::LoadingEntities;
            _screen.SetPopupMessage("Loading...", "Loading Entity metadata...", {});
            break;
        }
        case FileOperationState::SavingStart: {
            // Apply last changes before saving.
            _uiEventHandlers[UIEventType::ApplyEntityProperties](UIEvent{});
            _screen.ClearInfoMessages();
            FileLevelData fLevelData;
            fLevelData.levelData = _levelData;
            auto maxID = _entityManager.GetMaxEntityID();
            if (maxID.has_value()) {
                for (int entityID = 0; entityID <= *maxID; entityID++) {
                    auto* entityDefinition = _entityManager.GetEntityDefinition(entityID);
                    if (!entityDefinition) {
                        LOG_W << "Failed to get entity definition for entityID " << entityID << " during level saving." << std::endl;
                        continue;
                    }
                    FileEntityData entityData;
                    entityData.name = entityDefinition->name;
                    for (const auto& entityInstance : _entityManager.GetEntityInstancesByID(entityID)) {
                        entityData.instancesProperties.push_back(PropertyMapToStringMap(entityInstance->GetPropertiesMap()));
                    }
                    fLevelData.entitiesData.push_back(std::move(entityData));
                }
            }
            _saveLevelFuture = std::async(std::launch::async, XMLManager::SaveLevel, std::move(fLevelData));
            _screen.ClearInfoMessages();
            _screen.ClosePopup();
            _screen.SetPopupMessage("Saving...", "Saving level data...", {});
            _operationProgress = FileOperationState::SavingLevelData;
            break;
        }
        case FileOperationState::LoadingEntities:
            if (_loadEntitiesFuture.valid() && _loadEntitiesFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto entitiesData = _loadEntitiesFuture.get();
                if (entitiesData.empty()) {
                    throw LevelInitializationException("Failed to load entity metadata.");
                }
                for (auto& entityData : entitiesData) {
                    entityData.ApplySpriteData(_textureManager);
                    _entityManager.AddEntityDefinition(std::move(entityData.definition));
                }
                _loadEntitiesFuture = std::future<std::vector<FileEntityDefinitionData>>();
                _operationProgress = FileOperationState::LoadingLevelData;
                _screen.ClosePopup();
                _screen.SetPopupMessage("Loading...", "Loading level data...", {});
            }
            break;
        case FileOperationState::LoadingLevelData:
            if (_loadLevelDataFuture.valid() && _loadLevelDataFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto levelDataOpt = _loadLevelDataFuture.get();
                if (!levelDataOpt.has_value()) {
                    LOG_E << "Failed to load level data." << std::endl;
                    _screen.AddInfoMessage("Failed to load level data. Starting with empty level.");
                    _levelData.properties["fileName"] = _levelData.originalFileName;
                } else {
                    _levelData = std::move(levelDataOpt.value().levelData);
                    for (const auto& entityData : levelDataOpt.value().entitiesData) {
                        auto entityID = _entityManager.GetEntityDefinitionID(entityData.name);
                        if (!entityID) {
                            LOG_W << "Entity definition '" << entityData.name << "' not found for an instance in level data. Skipping instances." << std::endl;
                            continue;
                        }
                        auto entityDef = _entityManager.GetEntityDefinition(*entityID);
                        for (const auto& instanceProperties : entityData.instancesProperties) {
                            PropertyValueVect properties = StringMapToPropertyVect(instanceProperties, entityDef->GetPropertyDefinitions());
                            auto instanceID = _entityManager.CreateEntityInstance(*entityID, properties);
                            if (!instanceID.has_value()) {
                                LOG_W << "Failed to create entity instance of type '" << entityData.name << "' during level loading." << std::endl;
                            }
                        }
                    }
                }
                if (!_levelManager.LoadLevelData(_entityManager, _levelData)) {
                    throw LevelInitializationException("Failed to initialize level data.");
                }
                _loadLevelDataFuture = std::future<std::optional<FileLevelData>>();
                _operationProgress = FileOperationState::Done;
                _screen.ClosePopup();
                _setGridEventHandlers(); 
                _setUIEventHandlers();
            }
            break;
        case FileOperationState::SavingLevelData:
            if (_saveLevelFuture.valid() && _saveLevelFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                bool success = _saveLevelFuture.get();
                if (!success) {
                    _screen.ClearInfoMessages();
                    _screen.ClosePopup();
                    _screen.SetPopupMessage("Error", "Failed to save level data.", {"OK"});
                    LOG_E << "Failed to save level data." << std::endl;
                    _operationProgress = FileOperationState::Done;
                } else {
                    _screen.ClosePopup();
                    _screen.ClearButtons();
                    _screen.SetPopupMessage("Success", "Level saved successfully.", {"OK"}, [this](int) {
                        _nextState = AppState::LoadoutEditor;
                    });
                    // No operation progress change here to wait for user's acknowledgment
                }
            }
             break;
    }
    return false;
}

void LevelEditor::_setUIEventHandlers() {
    _uiEventHandlers[UIEventType::SaveLevel] =  [this](const UIEvent& event) {
        _operationProgress = FileOperationState::SavingStart;
    };
    _uiEventHandlers[UIEventType::DiscardLevel] =  [this](const UIEvent& event) {
        _screen.SetPopupMessage("Discard changes?", 
                "Are you sure you want to discard all changes and return to loadout editor?", 
                {"Cancel", "Discard"}, [this](int choice) {
            if (choice == 1) {
                _nextState = AppState::LoadoutEditor;
            }
        });
    };
    _uiEventHandlers[UIEventType::Undo] =  [this](const UIEvent& event) {
        ExecutionResult result = _commandManager.Undo(_entityManager, _levelData, _levelManager.GetGridManager());
        if (result == ExecutionResult::Failure) {
            _screen.AddInfoMessage("Failed to undo action.");
        } else if (result == ExecutionResult::NoChange) {
            _screen.AddInfoMessage("Nothing to undo.");
        } else {
            _levelManager.DeselectActiveEntity(_levelData);
            _levelManager.RefreshLevel(_levelData);
        }
    };
    _uiEventHandlers[UIEventType::Redo] =  [this](const UIEvent& event) {
        ExecutionResult result = _commandManager.Redo(_entityManager, _levelData, _levelManager.GetGridManager());
        if (result == ExecutionResult::Failure) {
            _screen.AddInfoMessage("Failed to redo action.");
        } else if (result == ExecutionResult::NoChange) {
            _screen.AddInfoMessage("Nothing to redo.");
        } else {
            _levelManager.DeselectActiveEntity(_levelData);
            _levelManager.RefreshLevel(_levelData);
        }
    };
    _uiEventHandlers[UIEventType::SelectEditorModeFirst] =  [this](const UIEvent& event) {
        auto& transientData = _levelManager.GetTransientData();
        if (transientData.activeEntity) {
            auto res = _levelManager.ApplyPropertiesToActive(_entityManager);
            if (res == ApplyResult::AllSuccess || res == ApplyResult::PartialSuccess) {
                auto* entityInstance = _entityManager.GetEntityInstance(*transientData.activeEntityID);
                if (entityInstance) {
                    EntityApplyPayload payload {
                        .entityID = transientData.activeEntity->entityID,
                        .instanceID = *transientData.activeEntityID,
                        .propertiesBefore = PropertyMapToPayload(entityInstance->GetPropertiesMap()),
                        .propertiesAfter = PropertyMapToPayload(transientData.activeEntity->GetPropertiesMap())
                    };
                    auto& gridManager = _levelManager.GetGridManager();
                    if(_commandManager.ExecuteCommand(
                            Command{.payload = std::move(payload)}, 
                            _entityManager, _levelData, gridManager) == ExecutionResult::Failure) {
                        LOG_W << "Failed to apply entity property changes." << std::endl;
                    }
                }
            }
            _levelManager.DeselectActiveEntity(_levelData);
        } else {
            transientData.editorMode = EditorMode::PlaceEntity;
        }
    };
    _uiEventHandlers[UIEventType::SelectEditorModeSecond] =  [this](const UIEvent& event) {
        _levelManager.RefreshActiveEntity(_entityManager, _levelData);
        auto& transientData = _levelManager.GetTransientData();
        if (transientData.activeEntity) {
            transientData.editorMode = EditorMode::EditPrimary;
        } else {
            _screen.AddInfoMessage("No entity selected.");
        }
    };
    _uiEventHandlers[UIEventType::SelectEditorModeThird] =  [this](const UIEvent& event) {
        _levelManager.RefreshActiveEntity(_entityManager, _levelData);
        auto& transientData = _levelManager.GetTransientData();
        if (transientData.activeEntity) {
            transientData.editorMode = EditorMode::EditSecondary;
        } else {
            _screen.AddInfoMessage("No entity selected.");
        }
    };
    _uiEventHandlers[UIEventType::SelectEntityFromPalette] =  [this](const UIEvent& event) {
        if (event.entityID && _levelManager.SetActiveBrush(*event.entityID, _entityManager)) {
            _screen.AddInfoMessage("Selected new active brush.");
        } else {
            _screen.AddInfoMessage("Invalid entity selected.");
        }
    };
    _uiEventHandlers[UIEventType::ApplyEntityProperties] =  [this](const UIEvent& event) {
        auto& transientData = _levelManager.GetTransientData();
        if (transientData.activeEntity) {
            bool createCommand = false;
            auto res = _levelManager.ApplyPropertiesToActive(_entityManager);
            switch (res) {
                case ApplyResult::AllSuccess:
                    _screen.AddInfoMessage("Changes applied.");
                    createCommand = true;
                    break;
                case ApplyResult::PartialSuccess:
                    _screen.AddInfoMessage("Some changes were invalid and have been reverted.");
                    createCommand = true;
                    break;
                case ApplyResult::NoChange:
                    _screen.AddInfoMessage("No changes to apply.");
                    break;
                case ApplyResult::Failure:
                    _screen.AddInfoMessage("Failed to apply changes.");
                    break;
            }
            if (createCommand) {
                auto propBefore = _entityManager.GetEntityInstance(*transientData.activeEntityID)->GetPropertiesMap();
                EntityApplyPayload payload {
                    .entityID = transientData.activeEntity->entityID,
                    .instanceID = *transientData.activeEntityID,
                    .propertiesBefore = PropertyMapToPayload(propBefore),
                    .propertiesAfter = PropertyMapToPayload(transientData.activeEntity->GetPropertiesMap())
                };
                auto& gridManager = _levelManager.GetGridManager();
                if(_commandManager.ExecuteCommand(
                        Command{.payload = std::move(payload)}, 
                        _entityManager, _levelData, gridManager) == ExecutionResult::Failure) {
                    LOG_W << "Failed to apply entity property changes." << std::endl;
                }
                _levelManager.RefreshActiveEntity(_entityManager, _levelData);
            }
        } else {
            if (!_levelManager.HasLevelChanged()) {
                _screen.AddInfoMessage("No changes to apply.");
                return;
            }
            LevelApplyPayload payload {
                .propertiesBefore = PropertyMapToPayload(_levelData.properties),
                .propertiesAfter = PropertyMapToPayload(transientData.propertyPayload.currentValues)
            };
            auto& gridManager = _levelManager.GetGridManager();
            if(_commandManager.ExecuteCommand(
                    Command{.payload = std::move(payload)}, 
                    _entityManager, _levelData, gridManager) == ExecutionResult::Failure) {
                LOG_W << "Failed to apply level property changes." << std::endl;
            } else {
                _screen.AddInfoMessage("Changes applied.");
            }
            _levelManager.RefreshLevel(_levelData);
        }
    };
    _uiEventHandlers[UIEventType::CopyEntity] =  [this](const UIEvent& event) {
        if (_levelManager.CopyActiveEntity()) {
            _screen.AddInfoMessage("Entity properties copied. New entities will be created with these properties.");
        } else {
            _screen.AddInfoMessage("No active entity to copy.");
        }
    };

    // Add buttons
    _screen.AddButton("Save", [this]() -> UIEvent {
        return UIEvent{UIEventType::SaveLevel, std::nullopt};
    });
    _screen.AddButton("Discard", [this]() -> UIEvent {
        return UIEvent{UIEventType::DiscardLevel, std::nullopt};
    });
    _screen.AddButton("Primary", [this]() -> UIEvent {
        return UIEvent{UIEventType::SelectEditorModeSecond, std::nullopt};
    });
    _screen.AddButton("Secondary", [this]() -> UIEvent {
        return UIEvent{UIEventType::SelectEditorModeThird, std::nullopt};
    });
    _screen.AddButton("Copy", [this]() -> UIEvent {
        return UIEvent{UIEventType::CopyEntity, std::nullopt};
    });
    _screen.AddButton("Undo", [this]() -> UIEvent {
        return UIEvent{UIEventType::Undo, std::nullopt};
    });
    _screen.AddButton("Redo", [this]() -> UIEvent {
        return UIEvent{UIEventType::Redo, std::nullopt};
    });

    // Add keyboard shortcuts
    _screen.AddKeyboardEvent({sf::Keyboard::Z, true, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::Undo, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Y, true, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::Redo, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::S, true, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::SaveLevel, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Escape, false, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::DiscardLevel, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::C, true, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::CopyEntity, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Num1, false, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::SelectEditorModeFirst, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Num2, false, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::SelectEditorModeSecond, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Num3, false, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::SelectEditorModeThird, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Enter, false, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::ApplyEntityProperties, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::Delete, false, false, false, [this]() -> UIEvent {
        auto& transientData = _levelManager.GetTransientData();
        auto activeEntityID = transientData.activeEntityID;
        auto* activeEntity = transientData.activeEntity.get();
        if (activeEntityID) {
            _levelManager.DeselectActiveEntity(_levelData);
            CreateDeletePayload payload {
                .entityID = activeEntity->entityID,
                .instanceID = *activeEntityID,
                .properties = PropertyMapToPayload(activeEntity->GetPropertiesMap()),
                .isCreation = false
            };
            auto& gridManager = _levelManager.GetGridManager();
            if(_commandManager.ExecuteCommand(Command{.payload = std::move(payload)}, 
                    _entityManager, _levelData, gridManager) == ExecutionResult::Failure) {
                LOG_W << "Failed to execute command for entity deletion." << std::endl;
                _screen.AddInfoMessage("Failed to delete entity.");
            } else {
                transientData.gridRenderValid = false;
            }
            return UIEvent{UIEventType::None, std::nullopt};
        }
        return UIEvent{UIEventType::None, std::nullopt};
    }});
}

void LevelEditor::_setGridEventHandlers() {
    _gridEventHandlers = {
        {GridEventType::LeftClick, [this](const GridEvent& event) {
            if (event.handleType != GridHandleType::None) {
                return;
            }
            auto gridSize = _levelManager.GetGridManager().GetSize();
            if (event.positionOrDistance.x < 0 || event.positionOrDistance.y < 0
                    || event.positionOrDistance.x >= gridSize.x || event.positionOrDistance.y >= gridSize.y) {
                if (_levelManager.GetTransientData().activeEntity) {
                    _levelManager.DeselectActiveEntity(_levelData);
                }
                return;
            }
            auto& transientData = _levelManager.GetTransientData();
            if (transientData.editorMode == EditorMode::PlaceEntity) {
                auto instanceID = _levelManager.GetInstanceAtPosition(event.positionOrDistance);
                if (instanceID) {
                    _levelManager.SelectActiveEntity(*instanceID, _entityManager);
                    return;
                }
                auto instancePayload = _levelManager.PlaceEntity(event.positionOrDistance, _entityManager);
                if (instancePayload) {
                    CreateDeletePayload payload {
                        .entityID = transientData.currentEntityBrushID,
                        .instanceID = std::nullopt,
                        .properties = instancePayload.value(),
                        .isCreation = true
                    };
                    _commandManager.ExecuteCommand(Command{.payload = std::move(payload)}, 
                        _entityManager, _levelData, _levelManager.GetGridManager());
                    transientData.gridRenderValid = false;
                } else {
                    _screen.AddInfoMessage("Cannot place entity here.");
                }
            } else {
                auto clickedEntityID = _levelManager.GetInstanceAtPosition(event.positionOrDistance);
                if (!clickedEntityID || *clickedEntityID != transientData.activeEntityID) {
                    _levelManager.DeselectActiveEntity(_levelData);
                }
            }
        }},
        {GridEventType::RightClick, [this](const GridEvent& event) {
            auto& transientData = _levelManager.GetTransientData();
            if (transientData.editorMode != EditorMode::PlaceEntity || event.handleType != GridHandleType::None) {
                _levelManager.DeselectActiveEntity(_levelData);
            } else {
                auto clickedEntityID = _levelManager.GetInstanceAtPosition(event.positionOrDistance);
                if (!clickedEntityID) {
                    return;
                }
                auto instance = _entityManager.GetEntityInstance(*clickedEntityID);
                CreateDeletePayload payload {
                    .entityID = instance->entityID,
                    .instanceID = *clickedEntityID,
                    .properties = PropertyMapToPayload(instance->GetPropertiesMap()),
                    .isCreation = false
                };
                auto& gridManager = _levelManager.GetGridManager();
                if(_commandManager.ExecuteCommand(Command{.payload = std::move(payload)}, _entityManager, _levelData, gridManager) == ExecutionResult::Failure) {
                    LOG_W << "Failed to execute command for entity deletion." << std::endl;
                    _screen.AddInfoMessage("Failed to delete entity.");
                } else {
                    transientData.gridRenderValid = false;
                }
            }
        }},
        {GridEventType::LeftDragStart, [this](const GridEvent& event) {
            if (event.handleType == GridHandleType::None) {
                auto& transientData = _levelManager.GetTransientData();
                auto entityID = _levelManager.GetInstanceAtPosition(event.positionOrDistance);
                if (!entityID) {
                    _levelManager.DeselectActiveEntity(_levelData);
                } else if (transientData.activeEntityID && *entityID == *transientData.activeEntityID) {
                    _levelManager.RefreshActiveEntity(_entityManager, _levelData);
                } else {
                    _levelManager.SelectActiveEntity(*entityID, _entityManager);
                }
            }
        }},
        {GridEventType::LeftDragMove, [this](const GridEvent& event) {
            if (event.handleType == GridHandleType::None) {
                _levelManager.MoveActive(event.positionOrDistance);
            } else {
                _levelManager.ScaleActive(event.positionOrDistance, event.handleType);
            }
        }},
        {GridEventType::LeftDragEnd, [this](const GridEvent& event) {
            auto& transientData = _levelManager.GetTransientData();
            if (transientData.editorMode != EditorMode::PlaceEntity) {
                ApplyResult res = _levelManager.ApplyPropertiesToActive(_entityManager);
                bool createCommand = false;
                switch (res) {
                    case ApplyResult::AllSuccess:
                        createCommand = true;
                        break;
                    case ApplyResult::PartialSuccess:
                        createCommand = true;
                        break;
                    case ApplyResult::NoChange:
                        break;
                    case ApplyResult::Failure:
                        _screen.AddInfoMessage("Failed to apply changes.");
                        break;
                }
                if (createCommand) {
                    auto propBefore = _entityManager.GetEntityInstance(*transientData.activeEntityID)->GetPropertiesMap();
                    EntityApplyPayload payload {
                        .entityID = transientData.activeEntity->entityID,
                        .instanceID = *transientData.activeEntityID,
                        .propertiesBefore = PropertyMapToPayload(propBefore),
                        .propertiesAfter = PropertyMapToPayload(transientData.activeEntity->GetPropertiesMap())
                    };
                    auto& gridManager = _levelManager.GetGridManager();
                    if(_commandManager.ExecuteCommand(Command{.payload = std::move(payload)}, _entityManager, _levelData, gridManager) == ExecutionResult::Failure) {
                        LOG_W << "Failed to execute command for property changes." << std::endl;
                        _screen.AddInfoMessage("Failed to create command for changes. Changes have been reverted.");
                    }
                    _levelManager.RefreshActiveEntity(_entityManager, _levelData);
                }
            }
        }},
        {GridEventType::MiddleDragStart, [this](const GridEvent& event) {
            auto& transientData = _levelManager.GetTransientData();
            if (transientData.activeEntity) {
                _levelManager.DeselectActiveEntity(_levelData);
            }
        }},
        {GridEventType::MiddleDragMove, [this](const GridEvent& event) {
            auto& transientData = _levelManager.GetTransientData();
            auto instancePayload = _levelManager.PlaceEntity(event.positionOrDistance, _entityManager);
            if (instancePayload) {
                CreateDeletePayload payload {
                    .entityID = transientData.currentEntityBrushID,
                    .instanceID = std::nullopt,
                    .properties = instancePayload.value(),
                    .isCreation = true
                };
                auto& gridManager = _levelManager.GetGridManager();
                _commandManager.ExecuteCommand(Command{.payload = std::move(payload)},
                    _entityManager, _levelData, gridManager);
                transientData.gridRenderValid = false;
            } 
        }},
        {GridEventType::MiddleDragEnd, [this](const GridEvent& event) {
            auto newEntityID = _levelManager.GetInstanceAtPosition(event.positionOrDistance);
            if (newEntityID) {
                _levelManager.SelectActiveEntity(*newEntityID, _entityManager);
            }
        }},
        {GridEventType::HandleMovement, [this](const GridEvent& event) {
            if (event.handleType != GridHandleType::None) {
                _levelManager.ScaleActive(event.positionOrDistance, event.handleType);
            }
        }}
    };
}

void LevelEditor::_setCommonHandlers() {
    _uiEventHandlers[UIEventType::Exit] = [this](const UIEvent& event) {
        OnExit();
    };

    _screen.AddKeyboardEvent({sf::Keyboard::Escape, true, false, false, []() -> UIEvent {
        return UIEvent{UIEventType::Exit, std::nullopt};
    }});
    _screen.AddKeyboardEvent({sf::Keyboard::F1, false, false, false, [this]() -> UIEvent {
        _screen.ToggleFps();
        return UIEvent{UIEventType::None, std::nullopt};
    }});
}

AppContext LevelEditor::Run(std::vector<sf::Event>& sfEvents, sf::Time deltaTime) {
    _nextState = AppState::LevelEditor;
    if (!_updateLoading()) {
        // Following may generate GridEvents, but there are no handlers for them, so it is safe to ignore.
        _screen.ProcessInput(sfEvents); 
        _screen.RenderCommon();
        return AppContext{_nextState, _levelData.originalFileName};
    }
    std::pair<std::vector<UIEvent>, std::vector<GridEvent>> events = _screen.ProcessInput(sfEvents);
    for (const auto& uiEvent : events.first) {
        auto handlerIt = _uiEventHandlers.find(uiEvent.type);
        if (handlerIt != _uiEventHandlers.end()) {
            handlerIt->second(uiEvent);
        }
    }
    for (const auto& gridEvent : events.second) {
        auto handlerIt = _gridEventHandlers.find(gridEvent.type);
        if (handlerIt != _gridEventHandlers.end()) {
            handlerIt->second(gridEvent);
        }
    }
    auto& transientData = _levelManager.GetTransientData();
    _screen.Render(_renderManager.RenderLevel(_entityManager, transientData, _levelData), transientData.propertyPayload, _textureManager, deltaTime);
    std::string levelName = GetTypedValue<std::string>("fileName", _levelData.properties).value_or("Untitled");
    return AppContext{_nextState, levelName};
}

void LevelEditor::OnExit() {
    _screen.ClosePopup();
    _screen.SetPopupMessage("Exit editor?", "Are you sure you want to exit the editor? All unsaved changes will be lost.", 
        {"Cancel", "Exit"}, [this](int choice) {
            if (choice == 1) {
                _nextState = AppState::Exit;
            }
        });
}
