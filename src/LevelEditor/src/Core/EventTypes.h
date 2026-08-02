#ifndef EVENTTYPES_H
#define EVENTTYPES_H

#include <optional>

#include "Types.h"
#include "RenderTypes.h"
#include "Properties.h"

enum class GridEventType {
    None,
    LeftClick,
    RightClick,
    LeftDragStart,
    LeftDragEnd,
    LeftDragMove,
    MiddleDragStart,
    MiddleDragEnd,
    MiddleDragMove,
    HandleMovement
};

struct GridEvent {
    GridEventType type;
    Vector2<float> positionOrDistance;
    GridHandleType handleType = GridHandleType::None;
};

enum class UIEventType {
    None,
    SaveLevel,
    DiscardLevel,
    Undo,
    Redo,
    SelectEditorModeFirst,
    SelectEditorModeSecond,
    SelectEditorModeThird,
    SelectEntityFromPalette,
    ApplyEntityProperties,
    CopyEntity,
    Exit

};

struct UIEvent {
    UIEventType type;
    std::optional<size_t> entityID = std::nullopt;
};

struct KeyboardEvent {
    sf::Keyboard::Key key;
    bool ctrlRequired;
    bool shiftRequired;
    bool altRequired;
    std::variant<std::function<UIEvent()>, std::function<GridEvent()>> action;
};

using UIEventHandler = std::function<void(const UIEvent&)>;
using GridEventHandler = std::function<void(const GridEvent&)>;

#endif