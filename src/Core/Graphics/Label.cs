using System;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;

namespace Core.Graphics;
public class Label(string text, SpriteFont font) : ISceneObject
{
    private string _text = text;
    private Vector2 _textSize = font.MeasureString(text);
    public Vector2 Scale { get; set; } = Vector2.One;
    public Vector2 Position { get; set; } = Vector2.Zero;
    private OriginLocation _origin = OriginLocation.TopLeft;

    public OriginLocation Origin
    {
        get => _origin;
        set
        {
            Position += (GetOrigin(value) - GetOrigin(_origin)) * Scale;
            _origin = value;
        }
    }

    public string Text
    {
        get => _text;
        set
        {
            _text = value;
            _textSize = font.MeasureString(_text);
        }
    }

    public float Rotation { get; set; } = 0f;

    // Whether the label should have a simple shadow 
    public bool Shaded { get; set; } = false;

    public float DrawLayer { get; set; } = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Interface);
    public SpriteEffects Effect { get; set; } = SpriteEffects.None;
    public Color Color { get; set; } = Color.White;
    public float Width => _textSize.X * Scale.X;
    public float Height => _textSize.Y * Scale.Y;

    /// <summary>
    /// Return the location of the specified origin scaled to the current scale
    /// </summary>
    public Vector2 GetOrigin(OriginLocation anchor)
    {
        var origin = Vector2.Zero;
        origin.X = ((int)anchor % 3) switch
        {
            // Set origin X
            2 => _textSize.X,
            1 => _textSize.X * 0.5F,
            _ => 0
        };

        origin.Y = (int)anchor switch
        {
            // Set origin Y
            < 3 => 0,
            < 6 => _textSize.Y * 0.5F,
            _ => _textSize.Y
        };
        return origin;
    }

    /// <summary>
    /// Scale the label to the specified width
    /// </summary>
    public void SetWidth(float width)
    {
        Scale *= width / Width;
    }
    /// <summary>
    /// Scale the label to the specified height
    /// </summary>
    public void SetHeight(float height)
    {
        Scale *= height / Height;
    }
    public bool Enabled { get; set; }
    public bool Visible { get; set; }

    public void Update(GameTime gameTime, Scene parentScene)
    {
        // Labels have no default update
    }

    public void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        batch.DrawString(font, _text, Position * uiScale + offset, Color, Rotation, GetOrigin(_origin), Scale * uiScale, Effect, DrawLayer);
        if (!Shaded) return;
        // Draw a shadow darker by 50 per value
        Color.Deconstruct(out byte r, out var g, out var b);
        r = (byte)Math.Max(0, r - 50);
        g = (byte)Math.Max(0, g - 50);
        b = (byte)Math.Max(0, b - 50);
        batch.DrawString(font, _text, (Position + Vector2.One * 2)* uiScale + offset,new Color(r, g, b), Rotation, GetOrigin(_origin),
            Scale * uiScale, Effect, DrawLayer - 0.01F);
    }

    public void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // Draw a square at the label's origin
        CoreModule.Instance.DrawSquare(Position, color: Color.Yellow, size: 5, drawLayer:CoreModule.GetDrawLayer(CoreModule.DrawLayers.Debug));
    }
}