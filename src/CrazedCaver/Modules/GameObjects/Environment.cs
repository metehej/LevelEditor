using Core;
using Core.Graphics;
using Core.Simulation;
using CrazedCaver.Modules.GameScenes;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace CrazedCaver.Modules.GameObjects;

/// <summary>
///  An entity solid from above
/// </summary>
public class Platform(Sprite sprite, Binding binding, bool copyObjects = true) : Entity(sprite, binding, copyObjects), ISolid
{
    public virtual bool IsSolid(BindingRectangle newRectangle, BindingRectangle oldRectangle)
    {
        return Binding.GetCollisionSide(newRectangle, oldRectangle, 
            newRectangle.TopLeft - oldRectangle.TopLeft
            ) == CollisionSide.Top;
    }
}

/// <summary>
/// A platform that sinks when stood on
/// </summary>
public class SinkingPlatform : Platform
{
    private const int LifetimeTicks = 30;
    private const float ScaleDecay = 1F / LifetimeTicks;
    private int _lifetimeTicksRemaining = LifetimeTicks;
    private bool _isStoodOn;

    public SinkingPlatform(Sprite sprite, Binding binding, bool copyObjects = true) : base(sprite, binding,
        copyObjects)
    {
        Sprite.Tint = Color.Gray; //Discolors the platform
        OriginalSprite.Tint = Color.Gray; //Discolors the original sprite
    }

    public override bool IsSolid(BindingRectangle newRectangle, BindingRectangle oldRectangle)
    {
        // Remember if any entity is standing on the platform
        _isStoodOn = base.IsSolid(newRectangle, oldRectangle);
        return _isStoodOn;
    }

    public override void Update(GameTime gameTime, Scene parentScene)
    {
        base.Update(gameTime, parentScene);
        if (!_isStoodOn) return;
        // Between the calls, an entity stood on the platform
        // Decay the platform
        _isStoodOn = false;
        _lifetimeTicksRemaining--;
        Sprite.SpriteScale.Y -= ScaleDecay;
        if (_lifetimeTicksRemaining > 0) return;
        // The platform has decayed completely
        Enabled = false;
        Visible = false;
    }

    public override void Reset()
    {
        base.Reset();
        _lifetimeTicksRemaining = LifetimeTicks;
        Enabled = true;
        Visible = true;
        _isStoodOn = false;
        _lifetimeTicksRemaining = LifetimeTicks;
    }
}

/// <summary>
/// A platform that forces an entity to move in a certain direction
/// </summary>
public class Belt : Platform
{
    private Vector2 _movement;
    private readonly Vector2 _originalMovement;

    public Belt(Sprite sprite, Binding binding, Vector2 movement, bool copyObjects = true) : base(sprite, binding,
        copyObjects)
    {
        _originalMovement = movement;
        OriginalSprite.Effect = movement.X > 0 ? SpriteEffects.FlipHorizontally : SpriteEffects.None;
    }

    // By how much should the entity be moved
    public Vector2 Movement
    {
        get => _movement;
        set
        {
            _movement = value;
            Sprite.Effect = value.X > 0 ? SpriteEffects.FlipHorizontally : SpriteEffects.None;
        }
    }

    public override void Reset()
    {
        base.Reset();
        _movement = _originalMovement;
    }
}

/// <summary>
/// The exit door
/// </summary>
public class Door(Sprite sprite, TextureRegion openedRegion, Binding binding, bool copyObjects = true)
    : Entity(sprite, binding, copyObjects), IInteractable
{
    private readonly TextureRegion _closedRegion = sprite.Region;
    private readonly TextureRegion _openedRegion = openedRegion;

    public bool Interact(Entity sender, CollisionSide side)
    {
        // Check if all keys are collected
        if (CoreModule.CurrentScene is not GameScene { RemainingKeys: 0 } scene) return false;
        scene.WinCondition = true;
        return true;
    }

    public override void Update(GameTime gameTime, Scene parentScene)
    {
        base.Update(gameTime, parentScene);
        Sprite.Region = parentScene is GameScene{ RemainingKeys:0 } 
            ? _openedRegion 
            : _closedRegion;
    }
}

public class Key(Sprite sprite, Binding binding, int value, bool copyObjects = true) : Entity(sprite, binding, copyObjects), IInteractable
{
    public bool Interact(Entity sender, CollisionSide side)
    {
        if (CoreModule.CurrentScene is not GameScene scene) return false;
        // Let the scene know that a key was collected
        scene.RemainingKeys--;
        Enabled = false;
        Visible = false;
        scene.Score += value;
        return false;
    }

    public override void Reset()
    {
        base.Reset();
        Enabled = true;
        Visible = true;
    }
}
