#ifndef GRID_H
#define GRID_H

#include <vector>
#include <unordered_map>
#include <optional>

#include "Types.h"
#include "WorldTypes.h"

/*
* Manages Tile reservations on the grid.
*/
class GridManager {
    private:
        Vector2<size_t> _size;
        std::vector<std::optional<InstanceID>> _tiles;

        Vector2<size_t> _getValidTile(Vector2<size_t> position) const {
            return {
                std::min(position.x, _size.x - 1),
                std::min(position.y, _size.y - 1)
            };
        }

        Vector2<size_t> _getTileCoordinates(size_t index) const {
            return {
                index % _size.x,
                index / _size.x
            };
        }

    public:
        GridManager(){
            _size = {1, 1};
            _tiles.resize(1);
        }

        /*
        * Creates a new grid manager.
        * Minimal grid size is 1.
        */
        GridManager(Vector2<size_t> size) : _size(size) {
            if (size.x == 0 || size.y == 0) {
                size = {1, 1};
            }
            _tiles.resize(size.x * size.y);
        }

        /*
        * Resizes the grid to new size.
        * If the size changes, returns true.
        * Drops all entity reservations on resize.
        * Minimal grid size is 1.
        */
        bool Resize(Vector2<size_t> newSize);

        /*
        * Marks tiles as reserved for a specific instance.
        * Does not check for occupancy.
        * Returns true if reservation succeeded.
        * Out of bounds coordinates result in failure.
        */
        bool ReserveTiles(Vector2<int> position, Vector2<size_t> size, const InstanceID instanceID);

        /*
        * Marks tiles as reserved for specific instance.
        * Checks for occupancy prior to marking.
        * Returns true if reservation succeeded
        * Out of bounds coordinates result in failure.
        */
        bool ReserveTilesSafely(Vector2<int> position, Vector2<size_t> size, const InstanceID instanceID);

        /*
        * Check if any tile of specific area is reserved.
        * Returns true if any tile is reserved.
        * If ignoredInstanceID is provided, ignores tiles reserved by that instance.
        * Out of bounds coordinates are considered reserved.
        */
        bool AreTilesReserved(Vector2<int> position, Vector2<size_t> size, const std::optional<InstanceID> ignoredInstanceID = std::nullopt) const;

        /*
        * Returns the InstanceID for this tile.
        * Returns nullopt if no instance reserved this tile or if coordinates are out of bounds.
        */
        inline std::optional<InstanceID> GetTile(Vector2<int> position) const {
            if (position.x < 0 || position.x >= _size.x || position.y < 0 || position.y >= _size.y) {
                return std::nullopt;
            }
            return _tiles[position.y * _size.x + position.x];
        }

        /*
        * Removes reservations from a specific rectangle. Only removes specific instance's reservations.
        */
        void ClearTiles(Vector2<int> position, Vector2<size_t> size, const InstanceID instanceID);

        /*
        * Removes all reservations from the grid.
        */
        void ClearAllTiles();

        /*
        * Clear all tiles owned by specific instance
        */
        void ClearTiles(const InstanceID instanceID);

        inline Vector2<size_t> GetSize() const {
            return _size;
        }
};

#endif