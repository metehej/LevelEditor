#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "Config.h"
#include "Types.h"
#include "LevelEditor.h"
#include "LoadoutEditor.h"



int main() {
    InitializeLogging();
    LOGI << "Starting Level Editor Tech Demo";

    sf::RenderWindow window(sf::VideoMode(Config::WINDOW_WIDTH, Config::WINDOW_WIDTH / Config::SCREEN_RATIO), "Level Editor Tech Demo");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) {
        LOG_E << "Failed to initialize ImGui-SFML, editor cannot run." << std::endl;
        return 1;
    }
    window.display();
    window.clear(Config::BACKGROUND_COLOR);
    window.setView(sf::View(sf::FloatRect(
        0.0f,
        0.0f,
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)
    )));
    
    // Prevent from saving window info
    ImGui::GetIO().IniFilename = nullptr;

    // State machine for loadout and level editors.
    AppState state = AppState::LoadoutEditor;
    AppState previousState = state;
    std::string levelNameContext = "cavern"; // TODO: remove default value

    std::unique_ptr<Editor> currentEditor = nullptr;

    Screen _loadingScreen(window);

    sf::Clock deltaClock;


    // State switching logic
    while(state != AppState::Exit) {
        // Initialize current state
        LOGI << "Next state: " << static_cast<int>(state) << std::endl;
        switch(state) {
            case AppState::LoadoutEditor: {
                // TODO: Implement LoadoutEditor and switch to it here
                state = AppState::LevelEditor;
                continue;
            }
            case AppState::LevelEditor: {
                if (!currentEditor) {
                    try {
                        currentEditor = std::make_unique<LevelEditor>(levelNameContext, window);
                    } catch (const LevelInitializationException& e) {
                        LOG_E << "Failed to initialize LevelEditor: " << e.what() << std::endl;
                        state = AppState::Exit;
                        continue;
                    }
                }
                break;
            }
            default:
                LOG_E << "Unknown AppState, exiting." << std::endl;
                state = AppState::Exit;
                continue;
        }
        // Run current state
        AppContext nextContext;
        do {
            try {
                std::vector<sf::Event> events;
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        currentEditor->OnExit();
                        continue;
                    }
                    events.push_back(event);
                }
                nextContext = currentEditor->Run(events, deltaClock.restart());
                levelNameContext = nextContext.levelName;
            } catch (const std::exception& e) {
                LOG_E << "Exception occurred while running editor: " << e.what() << std::endl;
                nextContext.nextState =  AppState::Exit;
            } catch (...) {
                LOG_E << "Unknown exception occurred while running editor." << std::endl;
                nextContext.nextState = AppState::Exit;
            }
        } while (nextContext.nextState == state);
        window.clear(Config::BACKGROUND_COLOR);
        previousState = state;
        state = nextContext.nextState;
        currentEditor.reset();
        levelNameContext = nextContext.levelName;
    }
    LOGI << "Exiting application." << std::endl;
    ImGui::SFML::Shutdown();
    window.setMouseCursor(sf::Cursor());
    window.close();
    return 0;
}