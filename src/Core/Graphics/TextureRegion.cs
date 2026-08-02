using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Core.Graphics;
/// <summary>
/// Stores a single named region in texture atlas
/// </summary>
public class TextureRegion
{
    private readonly Texture2D _texture;
    private readonly Rectangle _region;

    public int Width => _region.Width;
    public int Height => _region.Height;

    public TextureRegion(Texture2D source, int x, int y, int width, int height)
    {
        _texture = source;
        _region = new Rectangle(x, y, width, height);
    }

    public TextureRegion(Texture2D source, Vector2 origin, Vector2 size)
    {
        _texture = source;
        _region = new Rectangle(origin.ToPoint(), size.ToPoint());
    }

    public TextureRegion(Texture2D source, Rectangle region)
    {
        _texture = source;
        _region = region;
    }
    /// <summary>
    /// Draws the texture region with the specified parameters
    /// </summary>
    public void Draw(SpriteBatch batch, Vector2 position, Vector2 origin, Vector2 size, Color? tint = null, float rotation = 0, SpriteEffects effect = SpriteEffects.None, float layer = 0)
    {
        batch.Draw(_texture, new Rectangle(position.ToPoint(), size.ToPoint()), _region, tint ?? Color.White, rotation, origin, effect, layer);
    }
}

