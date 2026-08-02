cmake_minimum_required(VERSION 3.15)
project(LevelEditor)

# Setup C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define Dependencies (FetchContent)
include(FetchContent)

# -- SFML
FetchContent_Declare(sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1 # Using stable 2.6.1
)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sfml)

# -- ImGui
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# -- plog
FetchContent_Declare(plog
    GIT_REPOSITORY https://github.com/SergiusTheBest/plog
    GIT_TAG        1.1.10
    GIT_SHALLOW    true
)
FetchContent_Populate(plog)
add_library(plog INTERFACE)
target_include_directories(plog INTERFACE ${plog_SOURCE_DIR}/include)

# ImGui-SFML Config
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# -- ImGui-SFML
FetchContent_Declare(imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v2.6 # Version matching SFML 2.6
)
FetchContent_MakeAvailable(imgui-sfml)

# -- PugiXML
FetchContent_Declare(pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

# Executable
add_executable(LevelEditor src/main.cpp) 
add_subdirectory(src/Core)
add_subdirectory(src/Editors)
add_subdirectory(src/Managers)
add_subdirectory(src/Screens)
add_subdirectory(src/Utils)
add_subdirectory(src/World)

# Include directories (so bare #include "File.h" works across subdirectories)

target_include_directories(LevelEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Editors
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Managers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Screens
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/World
)

# Link Libraries
target_link_libraries(LevelEditor
    PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system 
    ImGui-SFML::ImGui-SFML 
    pugixml
    plog
)

# Define a macro for the compiler
target_compile_definitions(${PROJECT_NAME} PRIVATE ROOT_PATH="${CMAKE_CURRENT_BINARY_DIR}/")

target_precompile_headers(LevelEditor PRIVATE
    src/Core/Log.h
)

# Copy data
add_custom_command(TARGET LevelEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)