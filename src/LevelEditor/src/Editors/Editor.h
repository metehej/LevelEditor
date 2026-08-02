#ifndef EDITOR_H
#define EDITOR_H

#include <memory>
#include <SFML/Graphics.hpp>
#include "Types.h"

class Editor {
    protected:
        AppState _nextState = AppState::Exit;
    public:
        virtual ~Editor() = default;

        /*
         * Runs the main editor loop, handling rendering and user input.
         * Returns when editing is finished.
         */
        virtual AppContext Run(std::vector<sf::Event>& events, sf::Time deltaTime) = 0;

        /*
        * Last function call prior to the window closing.
        */
        virtual void OnExit() = 0;
};

#endif