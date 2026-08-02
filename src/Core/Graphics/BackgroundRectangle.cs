using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Core.Graphics;

/// <summary>
/// A simple UI Rectangle drawn as background
/// </summary>
public class BackgroundRectangle : ISceneObject
{
    public float X { get; set; }
    public float Y { get; set; }
    public float Width { get; set; }
    public float Height { get; set; }
    public Vector2 Position => new(X, Y);
    public Vector2 Size => new(Width, Height);
    public bool Enabled { get; set; } = true;
    public bool Visible { get; set; } = true;

    public float DrawLayer
    {
        get; 
        set;
    }
    public Color Color = Color.White;

    public BackgroundRectangle(float x, float y, float width, float height, float? drawLayer = null)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
        DrawLayer = drawLayer ?? CoreModule.GetDrawLayer(CoreModule.DrawLayers.Background) + 0.01F; ;
    }

    public BackgroundRectangle(Vector2 position, Vector2 size)
    {
        X = position.X;
        Y = position.Y;
        Width = size.X;
        Height = size.Y;
        DrawLayer = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Background) + 0.01F;
    }

    public void Update(GameTime gameTime, Scene parentScene)
    {
        // No update logic by default
    }

    public void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        if (!Visible) return;
        CoreModule.Instance.DrawRectangle(Position, Position + Size, Color, DrawLayer);
    }

    public void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // No debug info by default
    }
}
