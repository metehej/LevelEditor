#ifndef WORLDTYPES_H
#define WORLDTYPES_H

#include <unordered_map>
#include <string>
#include <optional>
#include <utility>
#include <vector>

#include "Properties.h"
#include "Config.h"
#include "ColorHelper.h"

enum class EntityPlacementKind {
    Singleton,
    ExpandHorizontal,
    ExpandOmni,
    Static,
    PathBased
};

enum class EntityBindingKind {
    Editable,
    NonEditable
};

struct InstanceID {
    size_t slot;
    size_t generation;
    bool operator==(const InstanceID& other) const {
        return slot == other.slot && generation == other.generation;
    }

    bool operator!=(const InstanceID& other) const {
        return !(*this == other);
    }
};

enum class Origin {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

template <typename EnumT>
struct EnumTraits;

template <>
struct EnumTraits<Origin> {
    static std::vector<std::pair<Origin, std::string>> Values() {
        return {
            {Origin::TopLeft, "TopLeft"},
            {Origin::TopCenter, "TopCenter"},
            {Origin::TopRight, "TopRight"},
            {Origin::CenterLeft, "CenterLeft"},
            {Origin::Center, "Center"},
            {Origin::CenterRight, "CenterRight"},
            {Origin::BottomLeft, "BottomLeft"},
            {Origin::BottomCenter, "BottomCenter"},
            {Origin::BottomRight, "BottomRight"}
        };
    }
};

#endif