using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Core.Graphics;

/// <summary>
/// Represents an image that is not a part of physics simulation
/// </summary>
// This class is basically an ISceneObject version of a sprite
public class Image(Sprite sprite) : ISceneObject, IResettable
{
    private Sprite _sprite = sprite;
    // A copy of Sprite kept for resetting
    private readonly Sprite _originalSprite = sprite.GetCopy();

    public Color Tint
    {
        get => _sprite.Tint;
        set => _sprite.Tint = value;
    }

    public bool Enabled { get; set; } = true;
    public bool Visible { get; set; } = true;
    
    /// <summary>
    /// Usual way to draw this image
    /// </summary>
    public void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        if (Visible) _sprite.Draw(gameTime, batch, offset, uiScale, tint: Tint);
    }

    /// <summary>
    /// Draw with additional parameters
    /// Allows access to sprite's extended Draw()
    /// Used for manual drawing, isn't called by Scene
    /// </summary>
    public void DrawExtended(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale, Color? tint = null, float? rotation = null, SpriteEffects? effect = null, float? drawLayer = null)
    {
        if (Visible) _sprite.Draw(gameTime, batch, offset, uiScale, tint ?? Tint, rotation, effect, drawLayer);
    }

    /// <summary>
    /// Draws debug information for this image
    /// </summary>
    public void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // Show image origin
        CoreModule.Instance.DrawSquare(_sprite.SpritePosition, drawLayer:CoreModule.GetDrawLayer(CoreModule.DrawLayers.Debug));
    }

    public void Update(GameTime gameTime, Scene parentScene)
    {
        // No default updates for Image
    }
    public void Reset()
    {
        _sprite = _originalSprite.GetCopy();
    }
}
