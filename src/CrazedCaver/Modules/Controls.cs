using Core.Input;
using Microsoft.Xna.Framework.Input;

namespace CrazedCaver.Modules;

/// <summary>
/// Contains most of the input controls for the game
/// Allows checking inputs directly
/// </summary>
public static class Controls
{
    public static KeyboardManager Keyboard { get; } = new();
    public static MouseManager Mouse { get; } = new();

    /// <summary>
    /// Update all Input Managers
    /// </summary>
    public static void Update()
    {
        Keyboard.Update();
        Mouse.Update();
    }

    /// <summary>
    /// Exit keybinds
    /// </summary>
    public static bool Exit()
    {
        return Keyboard.WasKeyPressed(Keys.Escape);
    }

    public static bool Debug()
    {
        return Keyboard.WasKeyPressed(Keys.F3);
    }

    /// <summary>
    /// Check if any MoveLeft key was pressed.
    /// </summary>
    public static bool MoveLeft()
    {
        return Keyboard.IsAnyKeyDown([Keys.E, Keys.T, Keys.U, Keys.O, Keys.A, Keys.Left]);
    }

    /// <summary>
    /// Check if any MoveRight key was pressed
    /// </summary>
    public static bool MoveRight()
    {
        return Keyboard.IsAnyKeyDown([Keys.R, Keys.Y, Keys.I, Keys.P, Keys.D, Keys.Right]);
    }

    /// <summary>
    /// Check if any Jump key was pressed.
    /// </summary>
    public static bool Jump()
    {
        return Keyboard.IsAnyKeyDown([Keys.W, Keys.Space, Keys.Up]);
    }

    public static bool Stop()
    {
        return Keyboard.IsAnyKeyDown([Keys.S, Keys.Down, Keys.LeftShift]);
    }

    /// <summary>
    /// Check if any Pause key was pressed.
    /// </summary>
    public static bool Pause()
    {
        return Keyboard.WasAnyKeyPressed([Keys.Q, Keys.OemQuestion]);
    }

    /// <summary>
    /// Check if any fullscreen key was pressed.
    /// </summary>
    public static bool FullScreen()
    {
        return Keyboard.WasKeyPressed(Keys.F11);
    }

    /// <summary>
    /// Check if background switch key was pressed
    /// </summary>
    public static bool BackgroundSwitch()
    {
        return Keyboard.WasKeyPressed(Keys.F1);
    }

    /// <summary>
    /// Check if cheat key was pressed
    /// Doesn't give any other information, the implementation is up to the game logic
    /// </summary>
    public static bool CheatKey()
    {
        return Keyboard.WasKeyPressed(Keys.RightAlt);
    }
}
