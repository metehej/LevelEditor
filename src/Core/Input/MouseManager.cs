using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace Core.Input;

/// <summary>
/// Manages mouse input
/// Only provides position information
/// </summary>
public class MouseManager
{
    private MouseState _currentState = Mouse.GetState();

    public Point Position
    {
        get => new(_currentState.X, _currentState.Y);
        set => SetPosition(value.X, value.Y);
    }
    public int X
    {
        get => _currentState.X;
        set => SetPosition(value, _currentState.Y);
    }
    public int Y
    {
        get => _currentState.Y;
        set => SetPosition(_currentState.X, value);
    }

    /// <summary>
    /// Refresh Mouse state
    /// </summary>
    public void Update()
    {
        _currentState = Mouse.GetState();
    }

    /// <summary>
    /// Set the mouse position to the specified coordinates
    /// </summary>
    private void SetPosition(int x, int y)
    {
        Mouse.SetPosition(x, y);
        Update();
    }
}
