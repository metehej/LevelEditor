#include "Screen.h"
#include "Config.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <imgui.h>
#include <imgui-SFML.h>
#include <limits>
#include "misc/cpp/imgui_stdlib.h"

#include "Properties.h"

void Screen::_loadInfoMessages() {
    double currentTime = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    if (_activePopup) {
        double popupElapsed = currentTime - _popUpOpenTime;
        for (auto& message : _infoMessages) {
            message.expireTime += popupElapsed;
        }
        _popUpOpenTime = currentTime;
    }

    auto it = _infoMessages.begin();
    while (it != _infoMessages.end()) {
        if (it->expireTime <= currentTime) {
            it = _infoMessages.erase(it);
        } else {
            it++;
        }
    }
    it = _infoMessages.begin();
    
    float windowWidth = std::max(Config::MESSAGE_BOX_WIDTH_P * _window.getSize().x, Config::MESSAGE_BOX_WIDTH_MIN);
    float maxTextWidth = windowWidth - (ImGui::GetStyle().WindowPadding.x * 2.0f); 
    
    float windowHeight = ImGui::GetTextLineHeightWithSpacing() + (ImGui::GetStyle().WindowPadding.y * 2.0f); 
    float bottomOffset = static_cast<float>(_window.getSize().y) - windowHeight;
    
    std::string ellipsis = "...";
    float ellipsisWidth = ImGui::CalcTextSize(ellipsis.c_str()).x;

    while (it != _infoMessages.end() && bottomOffset >= 0) {
        std::string originalMessage = it->message;
        std::string displayMessage = originalMessage;
        
        float textWidth = ImGui::CalcTextSize(displayMessage.c_str()).x;
        bool isTruncated = false;

        if (textWidth > maxTextWidth) {
            isTruncated = true;
            size_t estimatedChars = originalMessage.length() * (maxTextWidth - ellipsisWidth) / textWidth;
            
            do {
                displayMessage = originalMessage.substr(0, estimatedChars) + ellipsis;
                if (estimatedChars == 0) break;
                --estimatedChars;
            } while (ImGui::CalcTextSize(displayMessage.c_str()).x > maxTextWidth);
        }

        ImGui::SetNextWindowPos(ImVec2(0, bottomOffset), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always); 
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Config::INFO_MESSAGE_COLOR);
        ImGui::PushStyleColor(ImGuiCol_Border, Config::INFO_MESSAGE_BORDER);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::min(1.0f, static_cast<float>(it->expireTime - currentTime)));
        
        ImGui::Begin(("##InfoMessage" + std::to_string(it->id)).c_str(), nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings 
            | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        
        ImGui::TextUnformatted(displayMessage.c_str());

        if (isTruncated && ImGui::IsWindowHovered()) {
            ImGui::SetTooltip("%s", originalMessage.c_str());
        }

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
        
        ++it;
        bottomOffset -= windowHeight;
    }
    while (it != _infoMessages.end()) {
        it = _infoMessages.erase(it);
    }
}

void Screen::_loadPopup() {
    if (_activePopup) {
        // Overlay
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always); 
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
        ImGui::Begin("##PopupOverlay" + _popUpCounter, nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        
        ImGui::End();
        ImGui::PopStyleColor();

        // Popup
        ImGui::SetNextWindowPos(ImVec2(_window.getSize().x / 2.0f, _window.getSize().y / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(Config::POPUP_WIDTH, 0), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Config::POPUP_SPACING, Config::POPUP_SPACING));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Config::BACKGROUND_COLOR);
        ImGui::PushStyleColor(ImGuiCol_Border, Config::BORDER_COLOR);
        ImGui::Begin("##PopupMessage" + _popUpCounter, nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

        ImGui::SetWindowFocus(); 

        ImGui::Text("%s", _activePopup->title.c_str());
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::Text("%s", _activePopup->message.c_str());
        ImGui::PopTextWrapPos();

        float maxButtonWidth = 0;
        for (const auto& choice : _activePopup->choices) {
            float buttonWidth = ImGui::CalcTextSize(choice.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            if (buttonWidth > maxButtonWidth) {
                maxButtonWidth = buttonWidth;
            }
        }

        float totalButtonsWidth = (maxButtonWidth * _activePopup->choices.size()) + (ImGui::GetStyle().ItemSpacing.x * (_activePopup->choices.size() - 1));
        float startX = (ImGui::GetWindowSize().x - totalButtonsWidth) / 2.0f;

        ImGui::SetCursorPosX(startX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + Config::POPUP_SPACING);
        
        if (_activePopup->choices.empty()) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + Config::POPUP_SPACING);
            ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
            ImGui::Text("This pop up will be closed when the action is completed.");
            ImGui::PopTextWrapPos();
        }

        for (size_t i = 0; i < _activePopup->choices.size(); ++i) {
            if (ImGui::Button(_activePopup->choices[i].c_str(), ImVec2(maxButtonWidth, 0))) {
                _activePopup->onChoiceSelected(static_cast<int>(i));
                _activePopup.reset();
                break; 
            }
            if (i < _activePopup->choices.size() - 1) {
                ImGui::SameLine();
            }
        }

        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }
}

void Screen::_renderFpsLabel() {
    if (!_showFps) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin(
        "##FpsLabel",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove
    );
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}

bool Screen::_createPropertyInput(const Property& property, PropertyValue& value, bool isInputValid) {
    ImGui::PushID(property.propertyName.c_str());
    if (!isInputValid) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Config::INCORRECT_VALUE_COLOR);
    }

    const char* label = property.humanName.c_str();
    bool propertyChanged = false;
    switch (property.type) {
        case PropertyType::Integer: {
            auto* integerValue = std::get_if<int>(&value);
            if (!integerValue) {
                integerValue = new int(0);
                value = *integerValue;
            }
            int editedValue = *integerValue;
            int minValue = property.minValue.has_value() ? static_cast<int>(property.minValue.value()) : std::numeric_limits<int>::min();
            int maxValue = property.maxValue.has_value() ? static_cast<int>(property.maxValue.value()) : std::numeric_limits<int>::max();
            editedValue = std::clamp(editedValue, minValue, maxValue);
            bool valueChanged = ImGui::InputInt(label, &editedValue);
            if (valueChanged) {
                editedValue = std::clamp(editedValue, minValue, maxValue);
                if (editedValue != *integerValue) {
                    // Prevents marking if clamped value reverts the change
                    *integerValue = editedValue;
                    propertyChanged = true;
                    _changeTimer = 0.0f;
                }
            }
            break;
        }
        case PropertyType::Float: {
            auto* floatValue = std::get_if<float>(&value);
            if (!floatValue) {
                floatValue = new float(0.0f);
                value = *floatValue;
            }
            float editedValue = *floatValue;
            float minValue = property.minValue.has_value() ? static_cast<float>(property.minValue.value()) : -std::numeric_limits<float>::max();
            float maxValue = property.maxValue.has_value() ? static_cast<float>(property.maxValue.value()) : std::numeric_limits<float>::max();
            editedValue = std::clamp(editedValue, minValue, maxValue);
            bool valueChanged = ImGui::InputFloat(label, &editedValue);
            if (valueChanged) {
                editedValue = std::clamp(editedValue, minValue, maxValue);
                if (editedValue != *floatValue) {
                    // Prevents marking if clamped value reverts the change
                    *floatValue = editedValue;
                    propertyChanged = true;
                    _changeTimer = 0.0f;
                }
            }
            break;
        }
        case PropertyType::Boolean: {
            auto* boolValue = std::get_if<bool>(&value);
            if (!boolValue) {
                boolValue = new bool(false);
                value = *boolValue;
            }
            bool editedValue = *boolValue;
            if (ImGui::Checkbox(label, &editedValue)) {
                *boolValue = editedValue;
                propertyChanged = true;
                _changeTimer = 0.0f;
            }
            break;
        }
        case PropertyType::String:
        case PropertyType::Enum: {
            auto* stringValue = std::get_if<std::string>(&value);
            if (!stringValue) {
                stringValue = new std::string("");
                value = *stringValue;
            }

            if (!property.allowedValues.empty()) {
                if (ImGui::BeginCombo(label, stringValue->c_str())) {
                    for (const std::string& option : property.allowedValues) {
                        const bool isSelected = (*stringValue == option);
                        if (ImGui::Selectable(option.c_str(), isSelected)) {
                            *stringValue = option;
                            propertyChanged = true;
                            _changeTimer = 0.0f;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            } else {
                if (ImGui::InputText(label, stringValue)) {
                    propertyChanged = true;
                    _changeTimer = 0.0f;
                }
            }
            break;
        }
    }

    if (!isInputValid) {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
    return propertyChanged;
}

void Screen::_processPropertyInputs() {
    _changeTimer += ImGui::GetIO().DeltaTime;
    if (_changeTimer >= Config::PROPERTY_CHANGE_DELAY && ImGui::IsAnyItemActive() == false) {
        _newUiEvents.push_back(UIEvent{.type = UIEventType::ApplyEntityProperties});
        _changeTimer = 0.0f;
    }
}

void Screen::_createMenuButton(const std::string& label, std::function<void()> onClick) {
    ImGui::PushID(label.c_str());
    if (ImGui::Button(label.c_str())) {
        onClick();
    }
    ImGui::PopID();
}

void Screen::_renderCommon() {
    _loadInfoMessages();
    if (_activeMenuButtons.size() > 0) {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("##MenuBar", nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_MenuBar);
        for (const auto& [label, action] : _activeMenuButtons) {
            _createMenuButton(label, [this, action]() {
                UIEvent event = action();
                _newUiEvents.push_back(event);
            });
        }
        ImGui::End();
    }
    _loadPopup();
    _renderFpsLabel();
}

void Screen::AddInfoMessage(std::string message) {
    double currentTime = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    _infoMessages.push_front({std::move(message), currentTime + Config::INFO_MESSAGE_DURATION, _infoMessageIDCounter++});
}

void Screen::AddInfoMessage(std::vector<std::string> message) {
    double currentTime = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    for (auto it = message.rbegin(); it != message.rend(); ++it) {
        _infoMessages.push_front({std::move(*it), currentTime + Config::INFO_MESSAGE_DURATION, _infoMessageIDCounter++});
    }
}

bool Screen::SetPopupMessage(const std::string& title, const std::string& message, const std::vector<std::string>& choices, std::function<void(int)> onChoiceSelected) {
    if (_activePopup) {
        return false;
    }
    _activePopup = std::make_unique<PopUpMessage>(PopUpMessage{title, message, std::deque<std::string>(choices.begin(), choices.end()), onChoiceSelected});
    _popUpOpenTime = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    _popUpCounter++;
    return true;
}

void Screen::AddButton(const std::string& label, std::function<UIEvent()> onClick) {
    if (label.length() > _buttonLength) {
        _buttonLength = label.length();
        for (auto& [existingLabel, _] : _activeMenuButtons) {
            if (existingLabel.length() < _buttonLength) {
                existingLabel.append(_buttonLength - existingLabel.length(), ' ');
            }
        }
        _activeMenuButtons.push_back({label, onClick});
    } else {
        std::string paddedLabel = label + std::string(_buttonLength - label.length(), ' ');
        _activeMenuButtons.push_back({std::move(paddedLabel), onClick});
        return;
    }
}

void Screen::AddKeyboardEvent(KeyboardEvent keyboardEvent) {
    _activeKeyboardEvents.push_back(std::move(keyboardEvent));
}

void Screen::_handleKeyboardEvent(const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) {
        return;
    }
    for (const auto& keyboardEvent : _activeKeyboardEvents) {
        if (keyboardEvent.key == event.key.code) {
            bool valid = true;
            valid &= (keyboardEvent.ctrlRequired == event.key.control);
            valid &= (keyboardEvent.shiftRequired == event.key.shift);
            valid &= (keyboardEvent.altRequired == event.key.alt);
            if (valid) {
                if (std::holds_alternative<std::function<UIEvent()>>(keyboardEvent.action)) {
                    UIEvent uiEvent = std::get<std::function<UIEvent()>>(keyboardEvent.action)();
                    if (uiEvent.type != UIEventType::None) {
                        _newUiEvents.push_back(uiEvent);
                    }
                } else {
                    GridEvent gridEvent = std::get<std::function<GridEvent()>>(keyboardEvent.action)();
                    if (gridEvent.type != GridEventType::None) {
                        _newGridEvents.push_back(gridEvent);
                    }
                }
                return;
            }
        }
    }
}

void Screen::ToggleFps() {
    _showFps = !_showFps;
}

void Screen::RenderCommon() {
    ImGui::NewFrame();
    _window.clear(Config::BACKGROUND_COLOR);
    _renderCommon();
    ImGui::SFML::Render(_window);
    _window.display();
}