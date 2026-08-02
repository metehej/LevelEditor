using Core;
using Core.Graphics;
using Core.Simulation;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;
using System;
using System.Linq;
using CrazedCaver.Modules.GameScenes;

namespace CrazedCaver.Modules.GameObjects;

/// <summary>
/// Represents the player
/// </summary>
public class Character : MovableEntity
{
    /// <summary>
    /// Types of ground the character can stand on
    /// </summary>
    private enum GroundTypes
    {
        None,
        Solid,
        Belt
    }

    // How far (in CharacterScene units) to look for walls and grounds
    private const int SurroundingsSearchDistance = 2;
    // Horizontal acceleration for walking and vertical for jumps
    private readonly Vector2 _acceleration = new(1.8F, 7.8F);
    // Maximal achievable walking velocity and fall velocity that kills the character
    private readonly Vector2 _maxVelocity = new(3.0F, 10.3F);

    public Character(Sprite characterSprite, Binding binding) : base(characterSprite, binding)
    {
        // Place the sprite above other entities
        OriginalSprite.DrawLayer = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Character);
    }

    public override void Update(GameTime gameTime, Scene scene)
    {
        // Check for movement
        CheckInput(scene);

        // Movement updates
        var standingOn = StandingOn(scene);
        if (standingOn == GroundTypes.None)
            Velocity += CoreModule.Gravitation;
        else if (Velocity.Y > _maxVelocity.Y)
        {
            // Fell to death before update
            if (scene is GameScene gameScene)
            {
                gameScene.LossCondition = true;
            }
            Velocity.Y = 0;
            return;
        }
        if (standingOn == GroundTypes.Belt)
        {
            // Belt overrides players horizontal speed
            var belt = FindBelt(scene);
            Velocity.X = belt.Movement.X;
        }

        var oldRectangle = Binding.BindingRectangle.GetCopy();
        // If next to a solid object, don't move horizontally
        if (IsNear(Math.Sign(Velocity.X), scene))
        {
            Binding.Move(Velocity * Vector2.UnitY);
        }
        else Binding.Move(Velocity);

        // Check each colliding entity
        foreach (var entity in scene.FindCollidingEntities(this))
        {
            var side = entity.Binding.GetCollisionSide(Binding.BindingRectangle, oldRectangle, Velocity);
            switch (entity)
            {
                case ISolid solid when solid.IsSolid(Binding.BindingRectangle, oldRectangle):
                {
                    var intersect = entity.Binding.GetIntersect(Binding);
                    switch (side)
                    {
                        case CollisionSide.None:
                            break;
                        case CollisionSide.Top:
                            RigidCollision(-intersect.Height, side);
                            break;
                        case CollisionSide.Bottom:
                            RigidCollision(intersect.Height, side);
                            break;
                        case CollisionSide.Left:
                        case CollisionSide.Right:
                        default:
                            RigidCollision(intersect.Width, side);
                            break;
                    }

                    break;
                }
                // True return value from interact signifies a change that stops update
                case IInteractable interactableEntity when interactableEntity.Interact(this, side):
                    return;
            }
        }

        // Check the character stays within bounds
        var sceneRectangle = scene.SceneBounds.BindingRectangle;
        if (Binding.BindingRectangle.TopLeft.Y < sceneRectangle.TopLeft.Y)
        {
            RigidCollision(sceneRectangle.TopLeft.Y - Binding.BindingRectangle.TopLeft.Y, CollisionSide.Bottom);
        }

        // Finalize update based on current ground type
        standingOn = StandingOn(scene);
        if (standingOn == GroundTypes.None) return;
        if (Velocity.Y > _maxVelocity.Y)
        {
            // Fell to death after update
            if (scene is GameScene gameScene)
            {
                gameScene.LossCondition = true;
            }
            Velocity.Y = 0;
            return;
        }
        Velocity.Y = 0;
        if (standingOn == GroundTypes.Belt) return;
        Velocity.X *= CoreModule.SpeedLoss;
        if (Math.Abs(Velocity.X) < 0.5F)
        {
            Velocity.X = 0;
        }
    }

    /// <summary>
    /// A collision into an unmovable entity.
    /// </summary>
    /// <param name="displacement">The distance to the side</param>
    /// <param name="side">Side of the collision</param>
    private void RigidCollision(float displacement, CollisionSide side)
    {
        switch (side)
        {
            case CollisionSide.None:
                break;
            case CollisionSide.Top:
                Velocity.X *= CoreModule.SpeedLoss;
                Move(new Vector2(0, displacement - 1));
                break;
            case CollisionSide.Bottom:
                Velocity.Y *= -1;
                Move(new Vector2(0, displacement + 1));
                break;
            case CollisionSide.Left:
            case CollisionSide.Right:
            default:
                Move(new Vector2(-(2 * Math.Sign(Velocity.X)) * displacement, 0));
                break;
        }
    }

    // Move controls
    private void CheckInput(Scene scene)
    {
        // Changing this order may change the behavior of the character
        if (Controls.Stop()) Stop(scene);
        if (Controls.MoveLeft()) MoveLeft(scene);
        if (Controls.MoveRight()) MoveRight(scene);
        if (Controls.Jump()) Jump(scene);
    }

    public void MoveLeft(Scene scene)
    {
        if (StandingOn(scene) != GroundTypes.Solid) return;
        Velocity.X = Velocity.X - _acceleration.X < -_maxVelocity.X
            ? -_maxVelocity.X
            : Velocity.X - _acceleration.X;
    }

    public void MoveRight(Scene scene)
    {
        if(StandingOn(scene) != GroundTypes.Solid) return;
        Velocity.X = Velocity.X + _acceleration.X > _maxVelocity.X
            ? _maxVelocity.X
            : Velocity.X + _acceleration.X;
    }

    public void Jump(Scene scene)
    {
        if (StandingOn(scene) == GroundTypes.None || Velocity.Y > _maxVelocity.Y) return;
        Velocity.Y = -_acceleration.Y;
        Velocity.X = Velocity.X switch
        {
            < 0 => Velocity.X - 0.4F,
            > 0 => Velocity.X + 0.4F,
            _ => 0
        };
    }

    public void Stop(Scene scene)
    {
        if (StandingOn(scene) == GroundTypes.Solid)
        {
            Velocity.X = 0;
        }
    }

    /// <summary>
    /// Check if the character is standing on a solid entity.
    /// Move atop the closest such entity if any is found
    /// </summary>
    private GroundTypes StandingOn(Scene scene)
    {
        if (Velocity.Y < 0) return GroundTypes.None;
        var loweredRectangle = Binding.BindingRectangle.GetCopy().Move(Vector2.UnitY * SurroundingsSearchDistance);
        loweredRectangle.TopLeft = new Vector2(loweredRectangle.TopLeft.X, loweredRectangle.BottomRight.Y - SurroundingsSearchDistance);

        // Check if the player is not out of bounds
        if (loweredRectangle.BottomRight.Y > scene.SceneBounds.BindingRectangle.BottomRight.Y)
        {
            Move(new Vector2(0,
                scene.SceneBounds.BindingRectangle.BottomRight.Y -
                Binding.BindingRectangle.BottomRight.Y));
            return GroundTypes.Solid;
        }

        bool standing = false;
        float closest = SurroundingsSearchDistance + 1;
        // Find the closest solid object below the character
        ISceneObject closestObject = null;
        foreach (var entity in scene.FindCollidingEntities(loweredRectangle)
                     .Where(collidingEntity => collidingEntity is ISolid))
        {
            // Check if it is a solid from above
            if (!((ISolid)entity).IsSolid(loweredRectangle, Binding.BindingRectangle)) continue;
            standing = true;
            var distance = entity.Binding.BindingRectangle.TopLeft.Y - Binding.BindingRectangle.BottomRight.Y;
            // Check if the solid is closer
            if (!(distance < closest)) continue;
            closest = distance;
            closestObject = entity;
        }
        if (standing)
        {
            // Clip to the solid
            Move(new Vector2(0, closest));
        }
        if (closestObject is Belt)
        {
            return GroundTypes.Belt;
        }
        return closestObject is not null ? GroundTypes.Solid : GroundTypes.None;
    }

    /// <summary>
    /// Returns any belt underneath the character
    /// Presumes such belt exists
    /// </summary>
    private Belt FindBelt(Scene scene)
    {
        // Search for belts right below the character
        foreach (var entity in scene.FindCollidingEntities(
                     new BindingRectangle(Binding.BindingRectangle.TopLeft + Vector2.UnitY * Binding.BindingRectangle.Height, 
                         Binding.BindingRectangle.BottomRight + Vector2.UnitY * SurroundingsSearchDistance)))
        {
            if (entity is Belt beltObject)
            {
                return beltObject;
            }
        }
        throw new Exception("No belt found under the character.");
    }

    /// <summary>
    /// Check if the character is near a solid object in horizontal directions
    /// </summary>
    private bool IsNear(int direction, Scene scene)
    {
        if (direction == 0) return false;
        var shiftedRectangle = Binding.BindingRectangle.GetCopy().Move(Vector2.UnitX * Math.Sign(direction) * SurroundingsSearchDistance);
        return shiftedRectangle.BottomRight.X > scene.SceneBounds.BindingRectangle.BottomRight.X || // right side out of bounds
               shiftedRectangle.TopLeft.X < scene.SceneBounds.BindingRectangle.TopLeft.X || // Left side out of bounds
               scene
                   .FindCollidingEntities(shiftedRectangle)
                   .Any(entity =>
                        entity is ISolid solidEntity &&
                        solidEntity.IsSolid(shiftedRectangle, Binding.BindingRectangle)); // There is a solid entity
    }

    public override void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        Sprite.SpritePosition = Binding.ObjectOrigin;
        base.Draw(gameTime, batch, offset, uiScale);
    }

    public override void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        base.DrawDebugInfo(batch, offset, uiScale);
        // Draw velocity information
        batch.DrawString(CoreModule.Font, $"vX: {Velocity.X:0.00} vY: {Velocity.Y:0.00}", new Vector2(12, 52), Color.DarkSlateGray, 0, Vector2.Zero, Vector2.One * 0.25F, SpriteEffects.None, 0.41F);
        batch.DrawString(CoreModule.Font, $"vX: {Velocity.X:0.00} vY: {Velocity.Y:0.00}", new Vector2(10, 50), Color.White, 0, Vector2.Zero, Vector2.One * 0.25F, SpriteEffects.None, 0.4F);
    }
}