#ifndef SCREEN_H
#define SCREEN_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <deque>
#include <string>

#include "Properties.h"
#include "EventTypes.h"

struct InfoMessage {
    std::string message;
    double expireTime;
    size_t id;
};

struct PopUpMessage {
    std::string title;
    std::string message;
    std::deque<std::string> choices;
    std::function<void(int)> onChoiceSelected;
};

/*
* Screen with common screen functions.
*/
class Screen {
    protected:
        sf::RenderWindow& _window;
        std::deque<InfoMessage> _infoMessages;
        size_t _infoMessageIDCounter = 0;
        size_t _buttonLength = 0;
        std::vector<std::pair<std::string, std::function<UIEvent()>>> _activeMenuButtons;
        std::vector<KeyboardEvent> _activeKeyboardEvents;
        std::unique_ptr<PopUpMessage> _activePopup = nullptr;
        double _popUpOpenTime = 0.0;
        size_t _popUpCounter = 0;
        bool _showFps = false;
        float _changeTimer = 0.0f;

        std::vector<UIEvent> _newUiEvents;
        std::vector<GridEvent> _newGridEvents;

        void _loadInfoMessages();

        void _loadPopup();

        void _renderFpsLabel();

        /*
        * Creates an input based on the property type.
        * isInputValid changes the color of the input if false.
        * isInputValid should be gathered by validating the property in its context.
        * Returns true if the input has changed.
        */
        bool _createPropertyInput(const Property& property, PropertyValue& value, bool isInputValid);

        /*
        * Call when a property input is changed.
        * Checks property timer.
        * Generates a ApplyEntityProperty event if the timer has reached the treshold.
        */
        void _processPropertyInputs();

        void _createMenuButton(const std::string& label, std::function<void()> onClick);

        void _renderCommon();

        /*
        * Handles keyboard events based on set definitions.
        * Ignores events if a pop up or loading screen is active.
        */
        void _handleKeyboardEvent(const sf::Event& event);

    public:
        virtual ~Screen() = default;
        Screen(sf::RenderWindow& window) : _window(window) {}

        /*
        * Adds a temporary info message to the window.
        */
        void AddInfoMessage(std::string message);

        /*
        * Adds multiple temporary info messages to the window
        */
        void AddInfoMessage(std::vector<std::string> message);

        void ClearInfoMessages() {
            _infoMessages.clear();
        }

        /*
        * Adds a pop up message to the window, temporarily stopping all other inputs.
        * Returns false if another pop up is currently active, true otherwise.
        * Empty choices vector results in buttonless pop up which must be closed by calling ClosePopup.
        */
        bool SetPopupMessage(const std::string& title, const std::string& message, 
            const std::vector<std::string>& choices, std::function<void(int)> onChoiceSelected = [](int){});

        /*
        * Closes the current pop up.
        * The opened pop-up results in no action.
        */
        void ClosePopup() {
            _activePopup.reset();
        }

        /*
        * Renders common screen elements.
        */
        void RenderCommon();

        /*
        * Adds a button definition to screen's definitions.
        */
        void AddButton(const std::string& label, std::function<UIEvent()> onClick);

        /*
        * Removes all buttons.
        */
        void ClearButtons() {
            _activeMenuButtons.clear();
        }

        /*
        * Adds a keyboard shortcut definition to screen's definitions.
        */
        void AddKeyboardEvent(KeyboardEvent keyboardEvent);

        /*
        * Removes all set keyboard events.
        */
        void ClearKeyboardEvents() {
            _activeKeyboardEvents.clear();
        }

        /*
        * Clear keyboard event for a specific configurations.
        */
        void ClearKeyboardEvent(KeyboardEvent keyboardEvent) {
            _activeKeyboardEvents.erase(std::remove_if(_activeKeyboardEvents.begin(), _activeKeyboardEvents.end(),
                [&keyboardEvent](const KeyboardEvent& ke) {
                    return ke.key == keyboardEvent.key
                        && ke.ctrlRequired == keyboardEvent.ctrlRequired
                        && ke.shiftRequired == keyboardEvent.shiftRequired
                        && ke.altRequired == keyboardEvent.altRequired;
                }), _activeKeyboardEvents.end());
        }

        void ToggleFps();
};

#endif