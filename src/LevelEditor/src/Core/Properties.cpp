#include "Properties.h"

#include <algorithm>

namespace {

bool ValidateStringConstraints(const Property& property, const std::string& strValue) {
    if (!property.allowedValues.empty() &&
        std::find(property.allowedValues.begin(), property.allowedValues.end(), strValue) == property.allowedValues.end()) {
        return false;
    }
    if (property.minValue.has_value() && strValue.length() < static_cast<size_t>(property.minValue.value())) {
        return false;
    }
    if (property.maxValue.has_value() && strValue.length() > static_cast<size_t>(property.maxValue.value())) {
        return false;
    }
    return true;
}

} // namespace

bool Property::SetDefault(const std::string& defaultValue) {
    try {
        switch (type) {
            case PropertyType::Integer:
                this->defaultValue = std::stoi(defaultValue);
                break;
            case PropertyType::Float:
                this->defaultValue = std::stof(defaultValue);
                break;
            case PropertyType::Boolean:
                this->defaultValue = (defaultValue == "true");
                break;
            case PropertyType::String:
                this->defaultValue = defaultValue;
                break;
            case PropertyType::Enum:
                if (!allowedValues.empty() && std::find(allowedValues.begin(), allowedValues.end(), defaultValue) == allowedValues.end()) {
                    throw std::invalid_argument("Invalid default value for Enum type: " + defaultValue);
                }
                this->defaultValue = defaultValue;
                break;
        }
        return true;
    } catch (const std::exception& e) {
        LOG_W << "Failed to set default value for property " << propertyName << ": " << e.what() << std::endl;
        return false;
    }
}

bool Property::Validate(const PropertyValue& value, const PropertyValueMap& allProperties) const {
    if (!MatchesKind(type, value)) {
        return false;
    }

    if (type == PropertyType::Integer || type == PropertyType::Float) {
        const double numericValue = ToNumber(value);
        if (minValue.has_value() && numericValue < minValue.value()) {
            return false;
        }
        if (maxValue.has_value() && numericValue > maxValue.value()) {
            return false;
        }
    } else if (type == PropertyType::String || type == PropertyType::Enum) {
        const std::string& strValue = std::get<std::string>(value);
        if (!ValidateStringConstraints(*this, strValue)) {
            return false;
        }
    }
    if (validator.has_value()) {
        return validator.value()(value, allProperties);
    }
    return true;
}