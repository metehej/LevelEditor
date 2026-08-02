using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Core.Graphics;

/// <summary>
/// Allows drawing of a texture region
/// </summary>
public class Sprite(TextureRegion region, Vector2? scale = null)
{
    protected TextureRegion _region = region;

    /// <summary>
    /// The region of the sprite to draw
    /// Can only be switched for regions of the same size
    /// </summary>
    public TextureRegion Region
    {
        get => _region;
        set
        {
            if (_region.Width == value.Width && _region.Height == value.Height)
            {
                _region = value;
            }
        }
    }

    private OriginLocation _spriteOrigin = OriginLocation.TopLeft;
    public Vector2 SpriteScale = scale ?? Vector2.One;
    public Vector2 SpritePosition { get; set; } = Vector2.Zero;
    public float DrawLayer { get; set; } = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Sprites);
    public Color Tint { get; set; } = Color.White;
    public float Rotation { get; set; } = 0;
    public SpriteEffects Effect { get; set; } = SpriteEffects.None;
    /// <summary>
    /// Change the origin
    /// Keeps the sprite in the same place by adjusting the position based on the new origin
    /// </summary>
    public virtual OriginLocation SpriteOrigin
    {
        get => _spriteOrigin;
        set
        {
            SpritePosition += (GetOrigin(value) - GetOrigin(_spriteOrigin)) * SpriteScale;
            _spriteOrigin = value;
        }
    }
    // Scene-coordinate based information
    public float Width => _region.Width;
    public float Height => _region.Height;
    // Drawn-coordinate based information
    public float RenderedWidth => _region.Width * CoreModule.UiScale.X;
    public float RenderedHeight => _region.Height * CoreModule.UiScale.Y;

    /// <summary>
    /// Return the location of the specified origin position scaled to the current scale
    /// </summary>
    public Vector2 GetOrigin(OriginLocation anchor)
    {
        Vector2 origin = Vector2.Zero;
        origin.X = ((int)anchor % 3) switch
        {
            // Set origin X
            2 => Width,
            1 => Width * 0.5F,
            _ => 0
        };

        origin.Y = (int)anchor switch
        {
            // Set origin Y
            < 3 => 0,
            < 6 => Height * 0.5F,
            _ => Height
        };
        return origin;
    }

    /// <summary>
    /// Draw the sprite using specified or internal parameters
    /// </summary>
    public virtual void Draw(GameTime gameTime, SpriteBatch batch, Vector2? offset = null, Vector2? uiScale = null, Color? tint = null, float? rotation = null, SpriteEffects? effect = null, float? drawLayer = null)
    {
        _region.Draw(batch, (offset ?? Vector2.Zero) + SpritePosition * (uiScale ?? Vector2.One), GetOrigin(SpriteOrigin),
            new Vector2(RenderedWidth * SpriteScale.X, RenderedHeight * SpriteScale.Y),
            tint ?? Tint, rotation ?? Rotation, effect ?? Effect, drawLayer ?? DrawLayer);
    }

    /// <summary>
    /// Create a deep copy of Sprite
    /// </summary>
    public virtual Sprite GetCopy()
    {
        var sprite = new Sprite(_region, SpriteScale)
        {
            SpriteOrigin = _spriteOrigin,
            DrawLayer = DrawLayer,
            Tint = Tint,
            Rotation = Rotation,
            Effect = Effect,
            SpritePosition = SpritePosition
        };
        return sprite;
    }
}

/// <summary>
/// A sprite that periodically changes it's texture region
/// </summary>
public class AnimatedSprite : Sprite
{
    private readonly TextureRegion[] _regions;
    private float _timePerFrame;
    private static readonly Func<bool, TextureRegion[], float> FrameCount = 
        (loop, regions) => loop 
            ? 2 * regions.Length - 1
            : regions.Length;
    private double _frameTimeLeft;
    private int _activeFrame = 0;

    public float AnimationLength
    {
        get => FrameCount(Loop, _regions) * _timePerFrame;
        set
        {
            if (value == 0) return;
            _timePerFrame = value / FrameCount(Loop, _regions);
            _frameTimeLeft = _timePerFrame;
        }
    }

    private bool _loop = false;
    public bool Loop
    {
        get => _loop;
        set
        {
            var length = AnimationLength;
            _loop = value;
            AnimationLength = length;
        }
    }

    /// <summary>
    /// Creates an animated sprite
    /// Defaults to 1 second long animation that doesn't loop
    /// </summary>
    public AnimatedSprite(TextureRegion[] regions, Vector2? scale = null) : base(regions[0], scale)
    {
        _regions = regions;
        AnimationLength = 1;
    }

    /// <summary>
    /// Update current frame if enough time has passed 
    /// </summary>
    public void Update(GameTime gameTime)
    {
        _frameTimeLeft -= gameTime.ElapsedGameTime.TotalSeconds;
        if (_frameTimeLeft > 0) return;
        _activeFrame = (int)( _activeFrame + 1 - _frameTimeLeft / _timePerFrame) % (int)FrameCount(Loop, _regions);
        _frameTimeLeft = _timePerFrame + _frameTimeLeft % _timePerFrame;
        _region = _regions[Loop
            ? _regions.Length - 1 - Math.Abs(_activeFrame - _regions.Length + 1)
            : _activeFrame];
    }

    /// <summary>
    /// Draw the animated sprite, updating the current frame if necessary.
    /// </summary>
    public override void Draw (GameTime gameTime, SpriteBatch batch, Vector2? offset = null, Vector2? uiScale = null, Color? tint = null, float? rotation = null, SpriteEffects? effect = null, float? drawLayer = null)
    {
        if (CoreModule.CurrentScene.UpdateSceneObjects) Update(gameTime);
        base.Draw(gameTime, batch, offset, uiScale, tint, rotation, effect, drawLayer);
    }

    /// <summary>
    /// Create a deep copy of AnimatedSprite
    /// </summary>
    public override AnimatedSprite GetCopy()
    { 
        return new AnimatedSprite(_regions, SpriteScale)
        {
            SpriteOrigin = SpriteOrigin,
            SpritePosition = SpritePosition,
            DrawLayer = DrawLayer,
            Tint = Tint,
            Rotation = Rotation,
            Effect = Effect,
            AnimationLength = AnimationLength,
            Loop = Loop
        };
    }
}