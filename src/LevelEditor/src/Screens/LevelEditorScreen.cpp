#include "LevelEditorScreen.h"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <type_traits>
#include <variant>
#include <imgui.h>
#include <imgui-SFML.h>

#include "Config.h"
#include "ColorHelper.h"
#include "EventTypes.h"

GridInfo::GridInfo(size_t sizeX, size_t sizeY) : 
    maxWidthPercent(0.7f), maxHeightPercent(0.98f), 
    sizeX(sizeX), sizeY(sizeY), 
    cellMargin(Config::TILE_MARGIN),
    centerX(0.4f), centerY(0.5f),
    tileSize(Config::TILE_PIXEL_SIZE),
    gridWidth(0), gridHeight(0), realTileSize(0),
    initialX(0), initialY(0.0f), gridCenterX(0), gridCenterY(0)
{}

bool GridInfo::UpdateGrid(size_t newWindowWidth, size_t newWindowHeight) {
    gridCenterX = newWindowWidth * centerX;
    gridCenterY = newWindowHeight * centerY;

    size_t newWidth = std::floor(maxWidthPercent * newWindowWidth);
    if (newWidth  >  gridCenterX * 2) {
        newWidth = static_cast<size_t>(gridCenterX * 2);
    }
    size_t newHeight = maxHeightPercent * newWindowHeight;

    if (sizeX == 0 || sizeY == 0 || newWindowWidth == 0 || newWindowHeight == 0) {
        return false;
    }

    Vector2<size_t> widthFirst = {newWidth, newWidth * sizeY / sizeX};
    Vector2<size_t> heightFirst = {newHeight * sizeX / sizeY, newHeight};
    if (widthFirst.x < heightFirst.x) {
        gridWidth = widthFirst.x;
        gridHeight = widthFirst.y;
    } else {
        gridWidth = heightFirst.x;
        gridHeight = heightFirst.y;
    }
    
    float maxTileSizeX = gridWidth / sizeX;
    float maxTileSizeY = gridHeight / sizeY;

    float candidateTile = std::min(maxTileSizeX, maxTileSizeY);
    float tilePixels = std::floor(candidateTile);
    if (tilePixels < 1.0f) tilePixels = 1.0f;
    realTileSize = tilePixels;

    gridWidth = realTileSize * sizeX;
    gridHeight = realTileSize * sizeY;

    initialX = gridCenterX - gridWidth / 2;
    initialY = gridCenterY - gridHeight / 2;
    LOGI << "New grid configuration: " << gridWidth << "x" << gridHeight << " with tile size " << realTileSize << std::endl;

    return true;
}

namespace {

    Vector2<float> TileSizeToPixel(const GridInfo& gridInfo, const Vector2<float>& tile) {
        return Vector2<float>(tile.x * static_cast<float>(gridInfo.realTileSize), tile.y * static_cast<float>(gridInfo.realTileSize));
    }
    
    Vector2<float> TilePositionToPixel(const GridInfo& gridInfo, const Vector2<float>& tile) {
        Vector2<float> result = TileSizeToPixel(gridInfo, tile);
        result.x += static_cast<float>(gridInfo.initialX);
        result.y += static_cast<float>(gridInfo.initialY);
        return result;
    }

    Vector2<float> PositionToTile(const GridInfo& gridInfo, const Vector2<float>& position) {
        Vector2<float> newPosition = position;
        newPosition -= Vector2<size_t>{gridInfo.initialX, gridInfo.initialY};
        newPosition /= Vector2<size_t>{gridInfo.realTileSize, gridInfo.realTileSize};
        return newPosition;
    }
}

void LevelEditorScreen::_drawTextureInfo(
        const TextureRenderInfo& renderInfo,
        const sf::Texture& texture,
        const sf::Color& tint = sf::Color::White) {
    if (_gridInfo.realTileSize <= 0.0f) {
        return;
    }
    const sf::Vector2u textureSize = texture.getSize();
    if (textureSize.x == 0 || textureSize.y == 0) {
        return;
    }

    sf::Sprite sprite(texture);
    const auto position = TilePositionToPixel(_gridInfo, renderInfo.startTile);
    const auto size = TileSizeToPixel(_gridInfo, renderInfo.size);

    sprite.setPosition(position.x, position.y);

    sprite.setScale(size.x / static_cast<float>(textureSize.x), size.y / static_cast<float>(textureSize.y));
    sprite.setColor(tint);
    _window.draw(sprite);
}

void LevelEditorScreen::_drawGridRectangle(const GridRectangle& rectangle) {
    if (_gridInfo.realTileSize <= 0.0f) {
        return;
    }

    sf::RectangleShape shape;
    const auto position = TilePositionToPixel(_gridInfo, rectangle.position);
    const auto size = TileSizeToPixel(_gridInfo, rectangle.size);
    shape.setPosition(position.x, position.y);
    shape.setSize(sf::Vector2f(size.x, size.y));
    shape.setFillColor(ColorHelper::HexToColor(rectangle.colorHex));
    shape.setOutlineThickness(1.0f);
    shape.setOutlineColor(sf::Color::Black);
    _window.draw(shape);
}

void LevelEditorScreen::_drawHandle(const GridHandle& handle) {
    if (_gridInfo.realTileSize <= 0.0f) {
        return;
    }
    GridRectangle handleRectangle = handle.entityRectangle;

    switch (handle.type) {
        case GridHandleType::Top:
            handleRectangle.position.y -= Config::HANDLE_HEIGHT;
            handleRectangle.size.y = Config::HANDLE_HEIGHT;
            break;
        case GridHandleType::Bottom:
            handleRectangle.position.y += handleRectangle.size.y;
            handleRectangle.size.y = Config::HANDLE_HEIGHT;
            break;
        case GridHandleType::Left:
            handleRectangle.position.x -= Config::HANDLE_HEIGHT;
            handleRectangle.size.x = Config::HANDLE_HEIGHT;
            break;
        case GridHandleType::Right:
            handleRectangle.position.x += handleRectangle.size.x;
            handleRectangle.size.x = Config::HANDLE_HEIGHT;
            break;
    }
    _activeHandles.push_back({handle.type, handleRectangle});
    _drawGridRectangle(handleRectangle);
}

void LevelEditorScreen::_resize(const sf::Event::SizeEvent& sizeEvent) {
    _window.setView(sf::View(sf::FloatRect(
        0.0f,
        0.0f,
        static_cast<float>(sizeEvent.width),
        static_cast<float>(sizeEvent.height)
    )));
    _gridInfo.UpdateGrid(sizeEvent.width, sizeEvent.height);
}

void LevelEditorScreen::_renderGrid(LevelRenderInfo& levelRenderInfo, const TextureManager& textureManager) {
    if (_gridInfo.ResizeGrid(levelRenderInfo.gridSize.x, levelRenderInfo.gridSize.y)) {
        _gridInfo.UpdateGrid(_window.getSize().x, _window.getSize().y);
    }

    // Draw grid cells
    sf::RectangleShape cellShape;
    cellShape.setSize(sf::Vector2f(static_cast<float>(_gridInfo.realTileSize), static_cast<float>(_gridInfo.realTileSize)));
    cellShape.setFillColor(Config::GRID_COLOR);
    cellShape.setOutlineColor(Config::GRID_LINE_COLOR);
    cellShape.setOutlineThickness(-static_cast<float>(_gridInfo.cellMargin));
    for (size_t x = 0; x < _gridInfo.sizeX; x++) {
        for (size_t y = 0; y < _gridInfo.sizeY; y++) {
            cellShape.setPosition(
                _gridInfo.initialX + x * _gridInfo.realTileSize,
                _gridInfo.initialY + y * _gridInfo.realTileSize
            );
            _window.draw(cellShape);
        }
    }

    // Draw entities and other sprites
    for (const auto& renderInfo : levelRenderInfo.texturesToDraw) {
        _drawTextureInfo(renderInfo, textureManager.GetTexture(renderInfo.textureID));
    }

    // Draw the active entity and other tinted sprites
    for (const auto& renderInfo : levelRenderInfo.tintedTexturesToDraw) {
        _drawTextureInfo(
            renderInfo, 
            textureManager.GetTexture(renderInfo.textureID), 
            ColorHelper::HexToColor(renderInfo.tintColorHex)
        );
    }

    // Draw rectangles
    for (const auto& rectangle : levelRenderInfo.rectanglesToDraw) {
        _drawGridRectangle(rectangle);
    }

    // Draw handles on top of everything else
    _activeHandles.clear();
    if (!levelRenderInfo.handlesToDraw.empty()) {
        for (const auto& handle : levelRenderInfo.handlesToDraw) {
            _drawHandle(handle);
        }
    }
}

void LevelEditorScreen::_renderBrushText(const std::string& brushName) {
    if (brushName.empty()) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(_gridInfo.gridCenterX , 10.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::Begin("ActiveBrush", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings 
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted(brushName.c_str());
    ImGui::End();
}

void LevelEditorScreen::_renderProperties(PropertyPanelInfo& propertyPanelInfo, PropertyPayload& payload, const TextureManager& textureManager) {
    const float panelWidth = Config::PROPERTY_PANEL_WIDTH * static_cast<float>(_window.getSize().x);
    ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - panelWidth, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, static_cast<float>(_window.getSize().y)), ImGuiCond_Always);
    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    ImGui::TextUnformatted(propertyPanelInfo.name.c_str());
    ImGui::Separator();
    ImGui::TextWrapped("%s", propertyPanelInfo.description.c_str());

    if (propertyPanelInfo.collectionID.has_value()) {
        const auto& collection = textureManager.GetCollection(propertyPanelInfo.collectionID.value());
        const auto& spriteIDs = collection.GetSpriteIDs();
        int collectionSize = static_cast<int>(spriteIDs.size());

        // Render sprite collection
        if (collectionSize > 0) {
            ImGui::BeginChild("CollectionInfo", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY);
            ImGui::TextUnformatted("Sprites:");
            ImGui::NewLine();
            auto childWidth = ImGui::GetContentRegionAvail().x / static_cast<float>(collectionSize);
            if (childWidth > Config::MAX_BUTTON_SIZE) {
                childWidth = Config::MAX_BUTTON_SIZE;
            } else if (childWidth < Config::MIN_BUTTON_SIZE) {
                childWidth = Config::MIN_BUTTON_SIZE;
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            for (int i = 0; i < collectionSize; i++) {
                const auto spriteID = spriteIDs[static_cast<size_t>(i)];
                const auto& texture = textureManager.GetTexture(textureManager.GetSprite(spriteID).GetTextureID());
                const auto childHeight = texture.getSize().y * (childWidth / texture.getSize().x);
                const auto& spriteName = collection.GetSpriteName(spriteID);
                ImGui::BeginGroup();
                ImGui::Image(texture, ImVec2(childWidth, childHeight));
                ImGui::TextUnformatted(spriteName.c_str());
                ImGui::EndGroup();
                if (ImGui::GetContentRegionAvail().x < childWidth || i < collectionSize - 1) {
                    ImGui::SameLine();
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();        
        } else {
            ImGui::TextUnformatted("No sprites in collection.");
            return;
        }
    }

    // Render properties
    int count = 0;
    auto& properties = payload.properties;
    auto& currentValues = payload.currentValues;
    if (!properties.empty()) {
        ImGui::Spacing();
        ImGui::TextUnformatted("Properties");
        for (const auto& [propName, prop] : properties) {
            auto it = currentValues.find(propName);
            if (it == currentValues.end()) {
                continue;
            }
            auto itDef = properties.find(propName);
            if (itDef == properties.end() || !itDef->second.settable) {
                continue;
            }
            PropertyValue& value = it->second;
            bool isInputValid = prop.Validate(value, currentValues);
            payload.propertiesChanged |= this->_createPropertyInput(prop, value, isInputValid);
            count++;
        }
        if (payload.propertiesChanged) {
            _processPropertyInputs();
        }
    }
    if (count == 0) {
        ImGui::Spacing();
        ImGui::TextUnformatted("No settable properties.");
    }

    if (propertyPanelInfo.entityPalette.has_value()) {
        ImGui::Spacing();
        ImGui::TextUnformatted("Entity Palette");
        const float childHeight = std::max(ImGui::GetContentRegionAvail().y, 200.0f);
        ImGui::BeginChild("EntityPalette", ImVec2(0.0f, childHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
        const float buttonSize = std::min(0.3f * ImGui::GetContentRegionAvail().x, Config::MAX_BUTTON_SIZE);
        for (const auto& [category, entities] : propertyPanelInfo.entityPalette->get()) {
            ImGui::SeparatorText(category.c_str());
            ImGui::NewLine();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            int count = 0;
            for (const auto& entity : entities) {
                if (ImGui::GetContentRegionAvail().x < buttonSize) {
                    ImGui::NewLine();
                }
                ImGui::ImageButton(
                    textureManager.GetTexture(entity.textureID),
                    ImVec2(buttonSize, buttonSize)
                );
                ImGui::SetItemTooltip("%s", entity.name.c_str());
                if (ImGui::IsItemClicked()) {
                    _newUiEvents.push_back(UIEvent{
                        .type = UIEventType::SelectEntityFromPalette,
                        .entityID = entity.entityID
                    });
                }
                ImGui::SameLine();
            }
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }
        ImGui::EndChild();
    }

    ImGui::End();
}


void LevelEditorScreen::Render(LevelRenderInfo& levelRenderInfo, PropertyPayload& payload, const TextureManager& textureManager, sf::Time deltaTime) {
    ImGui::SFML::Update(_window, deltaTime);
    _window.clear(Config::BACKGROUND_COLOR);
    _renderGrid(levelRenderInfo, textureManager);
    _renderProperties(levelRenderInfo.propertyPanelInfo, payload, textureManager);
    _renderBrushText(levelRenderInfo.activeBrushName);
    _renderCommon();
    ImGui::SFML::Render(_window);
    _window.display();
}

std::pair<std::vector<UIEvent>, std::vector<GridEvent>> LevelEditorScreen::ProcessInput(const std::vector<sf::Event>& events) {
    for (const auto& event : events) {
        ImGui::SFML::ProcessEvent(_window, event);
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            continue;
        }
        switch (event.type) {
            case sf::Event::Resized:
                _resize(event.size);
                break;
            case sf::Event::MouseLeft:
                if (_mouseHLeft.IsDragging()) {
                    _mouseHLeft.OnRelease();
                }
                // Prevent generating strange events
                _newGridEvents.clear();
                _newUiEvents.clear();
                return std::make_pair(std::vector<UIEvent>{}, std::vector<GridEvent>{});
            case sf::Event::MouseButtonPressed: {
                Vector2<float> tilePos = PositionToTile(_gridInfo, Vector2<int>{event.mouseButton.x, event.mouseButton.y});
                if (event.mouseButton.button == sf::Mouse::Left) {
                    _mouseHLeft.OnPress(tilePos);
                    bool handleClicked = false;
                    for (const auto& handle : _activeHandles) {
                        auto position = handle.entityRectangle.position;
                        auto size = handle.entityRectangle.size;
                        if (tilePos.x >= position.x && tilePos.x <= position.x + size.x &&
                            tilePos.y >= position.y && tilePos.y <= position.y + size.y) {
                            _activeHandle = handle;
                            handleClicked = true;
                            break;
                        }
                    }
                    if (handleClicked) {
                        break;
                    }
                    _newGridEvents.push_back(GridEvent{
                        .type = GridEventType::LeftClick,
                        .positionOrDistance = tilePos
                    });
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    _newGridEvents.push_back(GridEvent{
                        .type = GridEventType::RightClick,
                        .positionOrDistance = tilePos
                    });
                } else if (event.mouseButton.button == sf::Mouse::Middle) {
                    _mouseHMiddle.OnPress(tilePos);
                }
                break;
            }
            case sf::Event::MouseButtonReleased: {
                Vector2<float> tilePos = PositionToTile(_gridInfo, Vector2<int>{event.mouseButton.x, event.mouseButton.y});
                if (event.mouseButton.button == sf::Mouse::Left) {
                    _activeHandle = {GridHandleType::None, GridRectangle{}};
                    if (_mouseHLeft.IsDragging()) {
                        _mouseHLeft.OnRelease();
                        _newGridEvents.push_back(GridEvent{
                            .type = GridEventType::LeftDragEnd,
                            .positionOrDistance = tilePos,
                            .handleType = _activeHandle.type
                        });
                    }
                } else if (event.mouseButton.button == sf::Mouse::Middle) {
                    if (_mouseHMiddle.IsDragging()) {
                        _mouseHMiddle.OnRelease();
                        _newGridEvents.push_back(GridEvent{
                            .type = GridEventType::MiddleDragEnd,
                            .positionOrDistance = tilePos
                        });
                    }
                }
                break;
            }
            case sf::Event::MouseMoved: {
                Vector2<float> tilePos = PositionToTile(_gridInfo, Vector2<int>{event.mouseMove.x, event.mouseMove.y});
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    bool oldDrag = _mouseHLeft.IsDragging();
                    Vector2<float> dragDistance = _mouseHLeft.OnMouseMoved(tilePos);
                    bool newDrag = _mouseHLeft.IsDragging();
                    if (!oldDrag && newDrag) {
                        _newGridEvents.push_back(GridEvent{
                            .type = GridEventType::LeftDragStart,
                            .positionOrDistance = tilePos,
                            .handleType = _activeHandle.type
                        });
                    } else if (dragDistance != Vector2<float>{0.0f, 0.0f}) {
                        _newGridEvents.push_back(GridEvent{
                            .type = GridEventType::LeftDragMove,
                            .positionOrDistance = dragDistance,
                            .handleType = _activeHandle.type
                        });
                    }
                } else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                    bool oldDrag = _mouseHMiddle.IsDragging();
                    Vector2<float> dragDistance = _mouseHMiddle.OnMouseMoved(tilePos);
                    bool newDrag = _mouseHMiddle.IsDragging();
                    if (!oldDrag && newDrag) {
                        _newGridEvents.push_back(GridEvent{
                            .type = GridEventType::MiddleDragStart,
                            .positionOrDistance = tilePos    
                        });
                    } else if (dragDistance != Vector2<float>{0.0f, 0.0f}) {
                        _newGridEvents.push_back(GridEvent{
                            .type = GridEventType::MiddleDragMove,
                            .positionOrDistance = tilePos
                    }); 
                    }
                }
                break;
            }
            case sf::Event::KeyPressed:
                _handleKeyboardEvent(event);
                break;
        }
    }
    auto tilePos = PositionToTile(_gridInfo, Vector2<int>{sf::Mouse::getPosition(_window).x, sf::Mouse::getPosition(_window).y});
    // Redundant move check for edge cases
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)){
        if (_mouseHLeft.IsDragging()) {
            _newGridEvents.push_back(GridEvent{
                .type = GridEventType::LeftDragEnd,
                .positionOrDistance = PositionToTile(_gridInfo, tilePos),
                .handleType = _activeHandle.type
            });
            _mouseHLeft.OnRelease();
            _activeHandle = {GridHandleType::None, GridRectangle{}};
        }
    }
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
        if (_mouseHMiddle.IsDragging()) {
            _newGridEvents.push_back(GridEvent{
                .type = GridEventType::MiddleDragEnd,
                .positionOrDistance = PositionToTile(_gridInfo, tilePos)
            });
            _mouseHMiddle.OnRelease();
        }
    }
    auto uiEvents = std::move(_newUiEvents);
    auto gridEvents = std::move(_newGridEvents);
    _newUiEvents.clear();
    _newGridEvents.clear();
    return std::make_pair(uiEvents, gridEvents);
}