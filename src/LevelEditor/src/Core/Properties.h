#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

template <typename EnumT>
struct EnumTraits;

enum class PropertyType {
    Integer,
    Float,
    Boolean,
    String,
    Enum
};

using PropertyValue = std::variant<int, float, bool, std::string>;
using PropertyValueMap = std::unordered_map<std::string, PropertyValue>;
using PropertyValidator = std::function<bool(const PropertyValue&, const PropertyValueMap&)>;
using PropertyValueVect = std::vector<std::pair<std::string, PropertyValue>>;

inline bool IsIntegerValue(const PropertyValue& value) {
    return std::holds_alternative<int>(value);
}

inline bool IsFloatValue(const PropertyValue& value) {
    return std::holds_alternative<float>(value) || std::holds_alternative<int>(value);
}

inline bool IsBooleanValue(const PropertyValue& value) {
    return std::holds_alternative<bool>(value);
}

inline bool IsStringValue(const PropertyValue& value) {
    return std::holds_alternative<std::string>(value);
}

inline double ToNumber(const PropertyValue& value) {
    if (const auto* integerValue = std::get_if<int>(&value)) {
        return static_cast<double>(*integerValue);
    }
    return static_cast<double>(std::get<float>(value));
}

inline PropertyType ToPropertyType(const std::string& propertyTypeString) {
    if (propertyTypeString == "Integer") {
        return PropertyType::Integer;
    }
    if (propertyTypeString == "Float") {
        return PropertyType::Float;
    }
    if (propertyTypeString == "Boolean") {
        return PropertyType::Boolean;
    }
    if (propertyTypeString == "Enum") {
        return PropertyType::Enum;
    }
    return PropertyType::String;
}

inline bool MatchesKind(PropertyType type, const PropertyValue& value) {
    switch (type) {
        case PropertyType::Integer:
            return IsIntegerValue(value);
        case PropertyType::Float:
            return IsFloatValue(value);
        case PropertyType::Boolean:
            return IsBooleanValue(value);
        case PropertyType::String:
            return IsStringValue(value);
        case PropertyType::Enum:
            return IsStringValue(value);
    }
    return false;
}

struct Property {
    std::string propertyName;
    std::string humanName;
    PropertyType type = PropertyType::String;
    std::optional<PropertyValue> defaultValue = std::nullopt;
    bool required = false;
    bool settable = false;
    std::optional<double> minValue = std::nullopt;
    std::optional<double> maxValue = std::nullopt;
    std::vector<std::string> allowedValues;
    std::optional<PropertyValidator> validator = std::nullopt;

    Property() = default;

    Property(std::string propertyName, std::string humanName, PropertyType type)
        : propertyName(std::move(propertyName)), humanName(std::move(humanName)), type(type) {}

    bool Validate(const PropertyValue& value, const PropertyValueMap& allProperties) const;

    bool SetDefault(const std::string& defaultValue);
};

using PropertyMap = std::unordered_map<std::string, Property>;

template <typename EnumT>
inline std::vector<std::pair<EnumT, std::string>> GetEnumEntries() {
    return EnumTraits<EnumT>::Values();
}

template <typename EnumT>
inline std::vector<std::string> GetEnumNames() {
    std::vector<std::string> names;
    for (const auto& [value, name] : GetEnumEntries<EnumT>()) {
        (void)value;
        names.push_back(name);
    }
    return names;
}

template <typename EnumT>
inline std::optional<EnumT> ParseEnumValue(const std::string& valueName) {
    for (const auto& [value, name] : GetEnumEntries<EnumT>()) {
        if (name == valueName) {
            return value;
        }
    }
    return std::nullopt;
}

template <typename EnumT>
inline std::string EnumToString(EnumT value) {
    for (const auto& [entryValue, name] : GetEnumEntries<EnumT>()) {
        if (entryValue == value) {
            return name;
        }
    }
    throw std::invalid_argument("Unknown enum value");
}

template <typename EnumT>
inline std::optional<EnumT> GetEnumValue(const PropertyValue& value) {
    if (!std::holds_alternative<std::string>(value)) {
        return std::nullopt;
    }
    return ParseEnumValue<EnumT>(std::get<std::string>(value));
}

template <typename EnumT>
inline Property MakeEnumProperty(
    std::string propertyName,
    std::string humanName,
    EnumT defaultValue,
    bool required = false,
    bool settable = false
) {
    Property property(std::move(propertyName), std::move(humanName), PropertyType::Enum);
    property.defaultValue = EnumToString(defaultValue);
    property.required = required;
    property.settable = settable;
    property.allowedValues = GetEnumNames<EnumT>();
    return property;
}

template <typename T>
inline std::optional<T> GetTypedValue(const std::string& propertyName, const PropertyValueMap& properties) {
    auto it = properties.find(propertyName);
    if (it != properties.end() && std::holds_alternative<T>(it->second)) {
        return std::get<T>(it->second);
    }
    return std::nullopt;
}

template <typename T>
inline std::optional<T> GetTypedValueOrDefault(const std::string& propertyName, const PropertyValueMap& properties, const PropertyMap& propertyDefinitions) {
    auto it = properties.find(propertyName);
    if (it != properties.end() && std::holds_alternative<T>(it->second)) {
        return std::get<T>(it->second);
    }
    auto defIt = propertyDefinitions.find(propertyName);
    if (defIt != propertyDefinitions.end() && defIt->second.defaultValue.has_value() && std::holds_alternative<T>(defIt->second.defaultValue.value())) {
        return std::get<T>(defIt->second.defaultValue.value());
    }
    return std::nullopt;
}

template <typename T>
inline std::optional<T> GetTypedValue(const std::string& propertyName, const PropertyValueVect& properties) {
    auto it = std::find_if(
        properties.begin(),
        properties.end(),
        [&propertyName](const auto& entry) {
            return entry.first == propertyName;
        }
    );
    if (it != properties.end() && std::holds_alternative<T>(it->second)) {
        return std::get<T>(it->second);
    }
    return std::nullopt;
}

template <typename T>
inline std::optional<T> GetTypedValueOrDefault(const std::string& propertyName, const PropertyValueVect& properties, const PropertyMap& propertyDefinitions) {
    auto it = std::find_if(
        properties.begin(),
        properties.end(),
        [&propertyName](const auto& entry) {
            return entry.first == propertyName;
        }
    );
    if (it != properties.end() && std::holds_alternative<T>(it->second)) {
        return std::get<T>(it->second);
    }
    auto defIt = propertyDefinitions.find(propertyName);
    if (defIt != propertyDefinitions.end() && defIt->second.defaultValue.has_value() && std::holds_alternative<T>(defIt->second.defaultValue.value())) {
        return std::get<T>(defIt->second.defaultValue.value());
    }
    return std::nullopt;
}



inline std::string PropertyValueToString(const PropertyValue& value) {
    return std::visit([](const auto& entry) -> std::string {
        using ValueT = std::decay_t<decltype(entry)>;
        if constexpr (std::is_same_v<ValueT, bool>) {
            return entry ? "true" : "false";
        } else if constexpr (std::is_same_v<ValueT, std::string>) {
            return entry;
        } else if constexpr (std::is_same_v<ValueT, float>) {
            return std::to_string(entry);
        } else {
            return std::to_string(entry);
        }
    }, value);
}

inline PropertyValueVect PropertyMapToPayload(const PropertyValueMap& propertyMap) {
    PropertyValueVect payload;
    for (const auto& [key, value] : propertyMap) {
        payload.emplace_back(key, value);
    }
    return payload;
}

inline std::unordered_map<std::string, std::string> PropertyMapToStringMap(const PropertyValueMap& propertyMap) {
    std::unordered_map<std::string, std::string> stringMap;
    for (const auto& [key, value] : propertyMap) {
        stringMap[key] = PropertyValueToString(value);
    }
    return stringMap;
}

inline PropertyValueVect StringMapToPropertyVect(const std::unordered_map<std::string, std::string>& stringMap, const PropertyMap& propertyDefinitions) {
    PropertyValueVect propertyVect;
    for (const auto& [key, strValue] : stringMap) {
        auto defIt = propertyDefinitions.find(key);
        if (defIt != propertyDefinitions.end()) {
            const auto& definition = defIt->second;
            if (definition.type == PropertyType::Integer) {
                try {
                    int intValue = std::stoi(strValue);
                    propertyVect.emplace_back(key, intValue);
                } catch (const std::exception&) {
                    LOG_W << "Failed to parse integer property '" << key << "' with value '" << strValue << "'" << std::endl;
                }
            } else if (definition.type == PropertyType::Float) {
                try {
                    float floatValue = std::stof(strValue);
                    propertyVect.emplace_back(key, floatValue);
                } catch (const std::exception&) {
                    LOG_W << "Failed to parse float property '" << key << "' with value '" << strValue << "'" << std::endl;
                }
            } else if (definition.type == PropertyType::Boolean) {
                bool boolValue = (strValue == "true" || strValue == "1");
                propertyVect.emplace_back(key, boolValue);
            } else if (definition.type == PropertyType::Enum || definition.type == PropertyType::String) {
                propertyVect.emplace_back(key, strValue);
            } else {
                LOG_W << "Unknown property type for property '" << key << "'" << std::endl;
            }
        }
    }
    return propertyVect;
}

#endif