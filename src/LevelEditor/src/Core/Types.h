#ifndef TYPES_H
#define TYPES_H

#include <string>

template <typename T>
struct Vector2 {
    T x;
    T y;

    constexpr Vector2() = default;
    constexpr Vector2(T x, T y) : x(x), y(y) {}

    constexpr bool operator== (const Vector2& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!= (const Vector2& other) const {
        return !(*this == other);
    }

    Vector2 operator+ (const Vector2& other) const {
        return {x + other.x, y + other.y};
    }
    
    Vector2 operator- (const Vector2& other) const {
        return {x - other.x, y - other.y};
    }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& operator/=(const Vector2& other) {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    Vector2& operator*= (const Vector2& other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    bool operator> (T value) const {
        return x > value || y > value;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
    constexpr Vector2(const Vector2<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
};

enum class EditorMode {
    PlaceEntity,
    EditPrimary,
    EditSecondary
};

enum class AppState {
    LoadoutEditor,
    LevelEditor,
    Exit
};

struct AppContext {
    AppState nextState;
    std::string levelName = "";
};
#endif