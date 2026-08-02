#ifndef MOUSEHELPER_H
#define MOUSEHELPER_H

#include <cmath>

#include "Config.h"

class MouseHelper {
    private:
        bool _isDragging = false;
        bool _isDown = false;
        float _oldX, _oldY;

    public:
        MouseHelper() : _oldX(0), _oldY(0) {}

        /*
        * Notes the position as a (possible) drag start.
        */
        void OnPress(Vector2<float> position) {
            _isDown = true;
            _oldX = position.x;
            _oldY = position.y;
        }

        /*
        * Calculates, whether a drag is happening.
        * Returns the distance if dragging, otherwise returns (0, 0).
        */
        Vector2<float> OnMouseMoved(Vector2<float> position) {
            if (_isDown) {
                float deltaX = position.x - _oldX;
                float deltaY = position.y - _oldY;
                if (!_isDragging) {
                    if (deltaX * deltaX + deltaY * deltaY >= Config::MIN_DRAG_DISTANCE * Config::MIN_DRAG_DISTANCE) {
                        _isDragging = true;
                    } else {
                        return {0.0f, 0.0f};
                    }
                }
                _oldX += deltaX;
                _oldY += deltaY;
                return {deltaX, deltaY};
            }
            return {0.0f, 0.0f};
        }

        void OnRelease() {
            _isDown = false;
            _isDragging = false;
        }

        bool IsDragging() const {
            return _isDragging;
        }
};

#endif