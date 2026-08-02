using System;
using Core;
using Core.Graphics;
using Core.Simulation;
using CrazedCaver.Modules.GameScenes;
using Microsoft.Xna.Framework;

namespace CrazedCaver.Modules.GameObjects;

// An enemy that does not move
public class StaticEnemy(Sprite sprite, Binding binding, bool copyObjects = true) : Entity(sprite, binding, copyObjects), IInteractable
{
    public bool Interact(Entity sender, CollisionSide side)
    {
        if (sender is not Character || CoreModule.CurrentScene is not GameScene scene) return false;
        scene.LossCondition = true;
        return true;
    }
}

// An entity that moves on a preset path
public class DynamicEnemy(Sprite sprite, Binding binding, Vector2 pathStart, Vector2 pathEnd, bool loop = false, float delay = 0, bool copyObjects = true) : MovableEntity(sprite, binding, copyObjects), IInteractable
{
    private Vector2 _firstPoint = pathStart, _secondPoint = pathEnd;
    private Vector2 _originalFirstPoint = pathStart, _originalSecondPoint = pathEnd;
    private bool _running = true;
    private TimeSpan _delayLeft;

    public override OriginLocation SpriteOrigin
    {
        get => Sprite.SpriteOrigin;
        set
        {
            // Calculate the movement based on the new origin and the current origin
            var movement = (Sprite.GetOrigin(value) - Sprite.GetOrigin(Sprite.SpriteOrigin)) * Sprite.SpriteScale;
            Binding.MoveRelativeOrigin(-movement);
            _firstPoint += movement;
            _secondPoint += movement;
            Sprite.SpriteOrigin = value;
            // Calculate the movement for the original sprite
            movement = (OriginalSprite.GetOrigin(value) - OriginalSprite.GetOrigin(OriginalSprite.SpriteOrigin)) * OriginalSprite.SpriteScale;
            OriginalBinding.MoveRelativeOrigin(-movement);
            OriginalSprite.SpriteOrigin = value;
            _originalFirstPoint += movement;
            _originalSecondPoint += movement;
        }
    }

    /// <summary>
    /// Velocity of the entity
    /// </summary>
    public new Vector2 Velocity
    {
        get => base.Velocity;
        set
        {
            base.Velocity = value;
            OriginalVelocity = value;
        }
    }

    public override void Update(GameTime gameTime, Scene parentScene)
    {
        // Check running first
        // This is necessary cause entity because disabled when waiting for a loop delay
        if (!_running)
        {
            // Delayed movement
            _delayLeft -= gameTime.ElapsedGameTime;
            if (_delayLeft > TimeSpan.Zero) return;
            _running = true;
            Visible = true;
            Enabled = true;
        }
        // Check if the entity is disabled by external code
        if (!Enabled) return;
        // Move in current direction
        Binding.Move(base.Velocity);
        // First position may be beyond the second for non-loop restart purposes
        float minX = _firstPoint.X, minY = _firstPoint.Y, maxX = _secondPoint.X, maxY = _secondPoint.Y;
        if (minX >  maxX) { (minX, maxX) = (maxX, minX); }
        if (minY > maxY) { (minY, maxY) = (maxY, minY); }
        
        if ((Binding.ObjectOrigin.X >= maxX  && base.Velocity.X > 0 )||
            (Binding.ObjectOrigin.X <= minX && base.Velocity.X < 0))
        {
            if (!loop)
            {
                // Starts over
                Binding.MoveTo(_firstPoint);
                SetDelay();
            }
            else
            {
                // Bounces back
                base.Velocity.X *= -1;
                var closest = Math.Clamp(Binding.ObjectOrigin.X, minX, maxX);
                Binding.MoveTo(new Vector2(closest, Binding.ObjectOrigin.Y));
                SetDelay(false);
            }
        }

        if ((Binding.ObjectOrigin.Y >= maxY && base.Velocity.Y > 0 )||
            (Binding.ObjectOrigin.Y <= minY && base.Velocity.Y < 0))
        {

            if (!loop)
            {
                Binding.MoveTo(_firstPoint);
                SetDelay();
            }
            else
            {
                base.Velocity.Y *= -1;
                var closest = Math.Clamp(Binding.ObjectOrigin.Y, minY, maxY);
                Binding.MoveTo(new Vector2(Binding.ObjectOrigin.X, closest));
                SetDelay(false);
            }
        }
        Sprite.SpritePosition = Binding.ObjectOrigin;
    }

    /// <summary>
    /// Set the delay before the entity starts moving again.
    /// </summary>
    private void SetDelay(bool hide = true)
    {
        _delayLeft = TimeSpan.FromSeconds(delay);
        if (_delayLeft <= TimeSpan.Zero) return;
        Visible = !hide;
        Enabled = !hide;
        _running = false;
    }

    /// <summary>
    /// Move Entity to specified position
    /// </summary>
    public override void MoveTo(Vector2 position)
    {
        var oldOrigin = Binding.ObjectOrigin;
        Binding.Move(position - Binding.BindingRectangle.TopLeft);
        Sprite.SpritePosition = Binding.ObjectOrigin;
        _firstPoint += Binding.ObjectOrigin - oldOrigin;
        _secondPoint += Binding.ObjectOrigin - oldOrigin;
    }

    /// <summary>
    /// Move Entity by vector
    /// </summary>
    public override void Move(Vector2 vector)
    {
        Binding.Move(vector);
        Sprite.SpritePosition += Binding.ObjectOrigin;
        _firstPoint += vector;
        _secondPoint += vector;
    }

    /// <summary>
    /// Interact with the entity
    /// </summary>
    public bool Interact(Entity sender, CollisionSide side)
    {
        if (sender is not Character || CoreModule.CurrentScene is not GameScene scene) return false;
        scene.LossCondition = true;
        return true;
    }
}
