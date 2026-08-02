using Core.Graphics;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;

namespace Core.Simulation;


#region Interfaces
/// <summary>
/// An interface for Entities that can collide
/// </summary>
public interface ISolid
{
    /// <summary>
    /// Check if the entity is solid. Parameters are the states of the other entity.
    /// </summary>
    /// <param name="newRectangle">New state</param>
    /// <param name="oldRectangle">State before movement to newRectangle</param>
    bool IsSolid(BindingRectangle newRectangle, BindingRectangle oldRectangle);
}

/// <summary>
/// An interface for Entities that can be interacted with
/// </summary>
public interface IInteractable
{
    /// <summary>
    /// Interact with this entity if applicable
    /// </summary>
    /// <returns>true if current UpdateLoop should stop.</returns>
    bool Interact(Entity sender, CollisionSide side);
}
#endregion

#region Classes
public class Entity : ISceneObjectSimulated, IResettable
{
    protected Sprite Sprite, OriginalSprite;
    protected Binding OriginalBinding;
    /// <summary>
    /// Change the sprite origin while keeping the entity in the same position
    /// </summary>
    public virtual OriginLocation SpriteOrigin
    {
        get => Sprite.SpriteOrigin;
        set
        {
            // Calculate the movement based on the new origin and the current origin
            var movement = (Sprite.GetOrigin(value) - Sprite.GetOrigin(Sprite.SpriteOrigin)) * Sprite.SpriteScale;
            Binding.MoveRelativeOrigin(-movement);
            // Set sprite origin for current and original sprite
            Sprite.SpriteOrigin = value;
            // Calculate the movement for the original sprite
            movement = (OriginalSprite.GetOrigin(value) - OriginalSprite.GetOrigin(OriginalSprite.SpriteOrigin)) * OriginalSprite.SpriteScale;
            OriginalBinding.MoveRelativeOrigin(-movement);
            OriginalSprite.SpriteOrigin = value;
        }
    }

    public bool Visible { get; set; } = true; // Draw the entity
    public bool Enabled { get; set; } = true; // Include it in collisions and interactions
    public Binding Binding { get; protected set; }

    /// <summary>
    /// Create an entity
    /// Passed objects are copied by default (to avoid shared references)
    /// </summary>
    public Entity(Sprite sprite, Binding binding, bool copyObjects = true)
    {
        Sprite = copyObjects ? sprite.GetCopy() : sprite;
        Sprite.DrawLayer = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Entities);
        Binding = copyObjects ? binding.GetCopy() : binding;
        Sprite.SpritePosition = binding.ObjectOrigin;
        OriginalSprite = Sprite.GetCopy();
        OriginalBinding = Binding.GetCopy();
    }

    /// <summary>
    /// Update Entity
    /// </summary>
    public virtual void Update(GameTime time, Scene parentScene)
    {
        // Entity has no default update logic
    }

    /// <summary>
    /// Draw Entity using batch
    /// </summary>
    public virtual void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        if (!Visible) return;
        Sprite.Draw(gameTime, batch, offset, uiScale);
    }

    /// <summary>
    /// Draw Entity diagnostics (Binding, Origin)
    /// </summary>
    public virtual void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        if (!Enabled) return;
        // Draw BindingRectangle
        CoreModule.Instance.DrawRectangle(
            Binding.BindingRectangle.TopLeft,
            Binding.BindingRectangle.BottomRight,
            Color.White, drawLayer: CoreModule.GetDrawLayer(CoreModule.DrawLayers.ImageBackground));

        // Draw origin
        CoreModule.Instance.DrawSquare(
            Binding.ObjectOrigin,
            Color.Black, size: 5, drawLayer: CoreModule.GetDrawLayer(CoreModule.DrawLayers.HigherDebug));
        CoreModule.Instance.DrawSquare(
            Binding.ObjectOrigin,
            Color.Yellow, drawLayer: CoreModule.GetDrawLayer(CoreModule.DrawLayers.HigherDebug) + 0.01F);
    }

    /// <summary>
    /// Move Entity to specified position
    /// </summary>
    public virtual void MoveTo(Vector2 position)
    {
        Binding.Move(position - Binding.ObjectOrigin);
        Sprite.SpritePosition = Binding.ObjectOrigin;
    }

    /// <summary>
    /// Move Entity by vector
    /// </summary>
    public virtual void Move(Vector2 vector)
    {
        Binding.Move(vector);
        Sprite.SpritePosition = Binding.ObjectOrigin;
    }

    /// <summary>
    /// Reset the Entity to its original state
    /// </summary>
    public virtual void Reset()
    {
        // Reset the binding to the initial position
        Binding = OriginalBinding.GetCopy();
        // Reset the sprite position to the binding origin
        Sprite = OriginalSprite.GetCopy();
    }

    /// <summary>
    /// Scale binding of the entity
    /// This applies to both current and original scale
    /// </summary>
    public virtual void BindingScale(Vector2 scale)
    {
        Binding.Scale(scale);
        OriginalBinding.Scale(scale);
    }
}

/// <summary>
/// An Entity that moves on update and has a velocity.
/// </summary>
public class MovableEntity : Entity
{
    protected Vector2 Velocity, OriginalVelocity;
    protected MovableEntity(Sprite sprite, Binding binding, bool copyObjects = true) : base(sprite, binding, copyObjects)
    {
        Velocity = Vector2.Zero;
        OriginalVelocity = Vector2.Zero;
    }

    public override void Reset()
    {
        Velocity = OriginalVelocity;
        base.Reset();
    }

    public override void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // Choose the side entity is facing based on the velocity
        Sprite.Effect = Velocity.X switch
        {
            > 0 => SpriteEffects.None,
            < 0 => SpriteEffects.FlipHorizontally,
            _ => Sprite.Effect
        };
        base.Draw(gameTime, batch, offset, uiScale);
    }
}

/// <summary>
/// A simple Entity that cannot be passed (e.g. a wall)
/// </summary>
public class ImpassableEntity(Sprite sprite, Binding binding, bool copyObjects = true) : Entity(sprite, binding, copyObjects), ISolid
{
    public virtual bool IsSolid(BindingRectangle newRectangle, BindingRectangle oldRectangle) => 
        Enabled;
}
#endregion
