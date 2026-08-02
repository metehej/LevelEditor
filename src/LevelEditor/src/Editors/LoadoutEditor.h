#ifndef LOADOUTEDITOR_H
#define LOADOUTEDITOR_H

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

#include "Editor.h"


class LoadoutEditor : public Editor {
    public:
        LoadoutEditor(sf::RenderWindow& window, std::optional<std::string> infoMessage);

        AppContext Run(std::vector<sf::Event>& sfEvents, sf::Time deltaTime) override;
};

#endif