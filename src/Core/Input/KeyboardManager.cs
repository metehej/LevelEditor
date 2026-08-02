using System.Linq;
using Microsoft.Xna.Framework.Input;

namespace Core.Input;

/// <summary>
/// Keyboard manager
/// </summary>
public class KeyboardManager
{
    private KeyboardState _lastState = Keyboard.GetState(), _currentState = Keyboard.GetState();

    /// <summary>
    /// Refresh Keyboard state
    /// </summary>
    public void Update()
    {
        _lastState = _currentState;
        _currentState = Keyboard.GetState();
    }

    /// <summary>
    /// Return true if Key is currently pressed
    /// </summary>
    public bool IsKeyDown(Keys key)
    {
        return _currentState.IsKeyDown(key);
    }

    /// <summary>
    /// Return true if key wasn't pressed last update and now is
    /// </summary>
    public bool WasKeyPressed(Keys key)
    {
        return !_lastState.IsKeyDown(key) && _currentState.IsKeyDown(key);
    }

    /// <summary>
    /// Return true if all keys weren't pressed last update and now are 
    /// </summary>
    public bool WereKeysPressed(Keys[] keys)
    {
        return keys.All(WasKeyPressed);
    }

    /// <summary>
    /// Return true if any of the keys satisfies WasKeyPressed condition
    /// </summary>
    public bool WasAnyKeyPressed(Keys[] keys)
    {
        return keys.Any(WasKeyPressed);
    }

    /// <summary>
    /// Return true if key was pressed last update and now isn't
    /// </summary>
    public bool WasKeyReleased(Keys key)
    {
        return _lastState.IsKeyDown(key) && !_currentState.IsKeyDown(key);
    }

    /// <summary>
    /// Return true if any of the keys satisfies WasKeyPressed condition
    /// </summary>
    public bool WasAnyKeyReleased(Keys[] keys)
    {
        return keys.Any(WasKeyReleased);
    }

    /// <summary>
    /// Return true if all keys are pressed
    /// </summary>
    public bool AreKeysDown(Keys[] keys)
    {
        return keys.All(IsKeyDown);
    }

    /// <summary>
    /// Test if any of the combinations satisfies AreKeysDown
    /// </summary>
    public bool AreKeysDown(Keys[][] keyCombinations)
    {
        return keyCombinations.Any(AreKeysDown);
    }

    /// <summary>
    /// Return true if any of the keys is pressed
    /// </summary>
    public bool IsAnyKeyDown(Keys[] keys)
    {
        return keys.Any(IsKeyDown);
    }
}
