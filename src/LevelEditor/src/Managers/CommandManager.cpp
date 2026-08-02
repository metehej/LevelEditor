#include "CommandManager.h"

void CommandManager::_addToUndoStack(Command&& command) {
    _undoStack.push_back(std::move(command));
    if (_undoStack.size() > Config::STACK_LIMIT) {
        _undoStack.pop_front();
    }
}

void CommandManager::_addToRedoStack(Command&& command) {
    _redoStack.push_back(std::move(command));
    if (_redoStack.size() > Config::STACK_LIMIT) {
        _redoStack.pop_front();
    }
}

ExecutionResult CommandManager::Undo(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager) {
    if (_undoStack.empty()) {
        return ExecutionResult::NoChange;
    }
    Command command = std::move(_undoStack.back());
    _undoStack.pop_back();
    if (command.Undo(entityManager, levelData, gridManager)) {
        _addToRedoStack(std::move(command));
        return ExecutionResult::Success;
    }
    _redoStack.clear();
    return ExecutionResult::Failure;
}

ExecutionResult CommandManager::Redo(EntityManager& entityManager, LevelData& levelData, GridManager& gridManager) {
    if (_redoStack.empty()) {
        return ExecutionResult::NoChange;
    }
    Command command = std::move(_redoStack.back());
    _redoStack.pop_back();
    if (command.Execute(entityManager, levelData, gridManager)) {
        _addToUndoStack(std::move(command));
        return ExecutionResult::Success;
    }
    _redoStack.clear();
    return ExecutionResult::Failure;
}

ExecutionResult CommandManager::ExecuteCommand(Command&& command, EntityManager& entityManager, LevelData& levelData, GridManager& gridManager) {
    if (command.Execute(entityManager, levelData, gridManager)) {
        _addToUndoStack(std::move(command));
        _redoStack.clear();
        return ExecutionResult::Success;
    }
    return ExecutionResult::Failure;
}