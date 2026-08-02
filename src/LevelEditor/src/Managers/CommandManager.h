#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

#include "Types.h"
#include "CommandTypes.h"

enum class ExecutionResult {
    Success,
    Failure,
    NoChange
};


class CommandManager {
    private:
        std::deque<Command> _undoStack;
        std::deque<Command> _redoStack;


        void _addToUndoStack(Command&& command);

        void _addToRedoStack(Command&& command);

    public:
        CommandManager() = default;

        /*
        * Removes all commands from stack.
        */
        void ResetCommandHistory() {
            _undoStack.clear();
            _redoStack.clear();
        }
        
        /*
        * Undoes the first command on stack.
        */
        ExecutionResult Undo(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager);

        /*
        * Redoes the first command on stack.
        */
        ExecutionResult Redo(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager);

        /*
        * Executes a command and appends it to undo stack.
        * Wipes redo stack.
        * If execution fails, command is disposed.
        */
        ExecutionResult ExecuteCommand(Command&& command, EntityManager& entityManager, LevelData& levelData, GridManager& gridManager);
};

#endif