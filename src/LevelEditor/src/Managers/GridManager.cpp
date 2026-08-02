#include "GridManager.h"

bool GridManager::Resize(Vector2<size_t> newSize) {
    if (newSize.x == _size.x && newSize.y == _size.y) {
        return false;
    }
    if (newSize.x == 0 || newSize.y == 0) {
        newSize = {1, 1};
    }
    _size = newSize;
    ClearAllTiles();
    return true;
}

bool GridManager::ReserveTiles(Vector2<int> position, Vector2<size_t> size, const InstanceID instanceID) {
    Vector2<int> posEnd = position + size - Vector2<int>{1, 1};
    if (position.x < 0 || position.y < 0 || posEnd.x >= _size.x || posEnd.y >= _size.y) {
        return false;
    }
    
    for (size_t y = position.y; y <= posEnd.y; y++) {
        for (size_t x = position.x; x <= posEnd.x; x++) {
            _tiles[y * _size.x + x] = instanceID;
        }
    }
    return true;
}

bool GridManager::ReserveTilesSafely(Vector2<int> position, Vector2<size_t> size, const InstanceID instanceID) {
    if (AreTilesReserved(position, size)) {
        return false;
    }
    return ReserveTiles(position, size, instanceID);
}

bool GridManager::AreTilesReserved(Vector2<int> position, Vector2<size_t> size,  const std::optional<InstanceID> ignoredInstanceID) const {
    Vector2<int> posEnd = position + size - Vector2<size_t>{1, 1};
    if (position.x < 0 || position.y < 0 || posEnd.x >= _size.x || posEnd.y >= _size.y) {
        return true;
    }
    for (size_t y = position.y; y <= posEnd.y; y++) {
        for (size_t x = position.x; x <= posEnd.x; x++) {
            auto tile = _tiles[y * _size.x + x];
            if (tile && tile != ignoredInstanceID) {
                return true;
            }
        }
    }
    return false;
}

void GridManager::ClearTiles(Vector2<int> position, Vector2<size_t> size, const InstanceID instanceID) {
    size -= Vector2<size_t>{1, 1};
    Vector2<size_t> posStart = _getValidTile(position);
    Vector2<size_t> posEnd = _getValidTile(position + size);

    for (size_t y = posStart.y; y <= posEnd.y; y++) {
        for (size_t x = posStart.x; x <= posEnd.x; x++) {
            if (_tiles[y * _size.x + x] == instanceID) {
                _tiles[y * _size.x + x] = std::nullopt;
            }
        }
    }
}

void GridManager::ClearAllTiles() {
    _tiles.clear();
    _tiles.resize(_size.x * _size.y);
}   

void GridManager::ClearTiles(const InstanceID instanceID) {
    ClearTiles({0, 0}, _size, instanceID);
}