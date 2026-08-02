using System;
using Core.Graphics;
using Microsoft.Xna.Framework;

namespace Core.Simulation;

/// <summary>
/// Enum representing the side of collision.
/// </summary>
public enum CollisionSide
{
    Top,
    Left,
    Bottom,
    Right,
    None
}

/// <summary>
/// A circle used for simple collision detection
/// </summary>
public class BindingCircle(Vector2 centre, float radius)
{
    public Vector2 CentrePoint { get; set; } = centre;
    public float Radius { get; set; } = radius;

    /// <summary>
    /// Move the BindingCircle by a vector
    /// </summary>
    public BindingCircle Move(Vector2 vector)
    {
        CentrePoint += vector;
        return this;
    }

    /// <summary>
    /// Return true if the BindingCircles overlap
    /// </summary>
    public static bool Overlaps(BindingCircle first, BindingCircle second)
    {
        var radiiDistance = first.Radius + second.Radius;
        var xDistance = first.CentrePoint.X - second.CentrePoint.X;
        var yDistance = first.CentrePoint.Y - second.CentrePoint.Y;
        return radiiDistance * radiiDistance >= xDistance * xDistance + yDistance * yDistance;
    }

    /// <summary>
    /// Return the distance between BindingCircles edges
    /// </summary>
    public static float Distance(BindingCircle first, BindingCircle second)
    {
        var radiiDistance = first.Radius + second.Radius;
        var xDistance = first.CentrePoint.X - second.CentrePoint.X;
        var yDistance = first.CentrePoint.Y - second.CentrePoint.Y;
        return (float)(Math.Sqrt(xDistance * xDistance + yDistance * yDistance) - radiiDistance);
    }
    /// <summary>
    /// Return the distance between a point and a circle
    /// </summary>
    public static float Distance(BindingCircle circle, Vector2 point)
    {
        return Distance(circle, new BindingCircle(point, 0));
    }

    /// <summary>
    /// Create a deep copy of BindingCircle
    /// </summary>
    public BindingCircle Copy()
    {
        return new BindingCircle(CentrePoint, Radius);
    }
}

/// <summary>
/// A rectangle for more complex collision detection
/// </summary>
public class BindingRectangle(Vector2 topLeft, Vector2 bottomRight)
{
    /// <summary>
    /// TopLeft position of Rectangle, adjusted to UI scale
    /// </summary>
    public Vector2 TopLeft { get; set; } = topLeft;

    /// <summary>
    /// BottomRight position of Rectangle, adjusted to UI scale
    /// </summary>
    public Vector2 BottomRight { get; set; } = bottomRight;
    
    public float Width => BottomRight.X - TopLeft.X;
    public float Height => BottomRight.Y - TopLeft.Y;


    /// <summary>
    /// Move rectangle by vector
    /// </summary>
    public BindingRectangle Move(Vector2 vector)
    {
        TopLeft += vector;
        BottomRight += vector;
        return this;
    }

    /// <summary>
    /// Create a deep copy of BindingRectangle
    /// </summary>
    public BindingRectangle GetCopy()
    {
        return new BindingRectangle(TopLeft, BottomRight);
    }
}

/// <summary>
/// A bundle of BindingRectangle and BindingCircle for collision detection system
/// </summary>
public class Binding
{
    public BindingRectangle BindingRectangle { get; } = new(Vector2.One, Vector2.One);

    public BindingCircle BindingCircle { get; } = new (Vector2.Zero,0);

    private Vector2 _origin;

    /// <summary>
    /// Last set position of the object that this Binding is bound to
    /// </summary>
    public Vector2 ObjectOrigin { get; private set; }

    public Vector2 Size { get; set; }


    public Binding(Vector2 objectOrigin, Vector2 relativeOrigin, Vector2 size)
    {
        _origin = relativeOrigin;
        Size = size;
        Update(objectOrigin);
    }

    /// <summary>
    /// Return true if Bindings intersect
    /// </summary>
    public bool Intersects(Binding secondBinding, bool circleMode = false)
    {
        return circleMode 
            ? BindingCircle.Overlaps(BindingCircle, secondBinding.BindingCircle) 
            : !(
                secondBinding.BindingRectangle.TopLeft.X >= BindingRectangle.BottomRight.X ||
                secondBinding.BindingRectangle.BottomRight.X <= BindingRectangle.TopLeft.X ||
                secondBinding.BindingRectangle.TopLeft.Y >= BindingRectangle.BottomRight.Y ||
                secondBinding.BindingRectangle.BottomRight.Y <= BindingRectangle.TopLeft.Y);
    }

    /// <summary>
    /// Return true if Binding intersects with a rectangle
    /// </summary>
    public bool Intersects(BindingRectangle rectangle)
    {
        return !(
            rectangle.TopLeft.X >= BindingRectangle.BottomRight.X ||
            rectangle.BottomRight.X <= BindingRectangle.TopLeft.X ||
            rectangle.TopLeft.Y >= BindingRectangle.BottomRight.Y ||
            rectangle.BottomRight.Y <= BindingRectangle.TopLeft.Y);
    }

    /// <summary>
    /// Return true if Binding intersects with a circle
    /// </summary>
    public bool Intersects(BindingCircle circle)
    {
        return BindingCircle.Overlaps(BindingCircle, circle);
    }

    /// <summary>
    /// Get a Rectangle of the intersection
    /// </summary>
    public BindingRectangle GetIntersect(BindingRectangle rectangle)
    {
        Vector2 tl = BindingRectangle.TopLeft, br = BindingRectangle.BottomRight;
        float lx = tl.X, ly = tl.Y, mx = br.X, my = br.Y;
        tl = rectangle.TopLeft;
        br = rectangle.BottomRight;
        if (lx < tl.X) lx = tl.X;
        if (mx > br.X) mx = br.X;
        if (ly < tl.Y) ly = tl.Y;
        if (my > br.Y) my = br.Y;
        return 
            new BindingRectangle(new Vector2(lx, ly), new Vector2(mx, my));
    }

    /// <summary>
    /// Get a Rectangle of the intersection with another Binding
    /// </summary>
    public BindingRectangle GetIntersect(Binding secondBinding)
    {
        return GetIntersect(secondBinding.BindingRectangle);
    }

    /// <summary>
    /// Return which side collided first or CollisionSide.None if no valid collision found
    /// </summary>
    public CollisionSide GetCollisionSide(BindingRectangle secondBinding, BindingRectangle oldBinding, Vector2 velocity)
    {
        if (!Intersects(secondBinding) || Intersects(oldBinding)) 
            return CollisionSide.None;

        // Find which sides were breached
        var axisX = velocity.X > 0 ? BindingRectangle.TopLeft.X : BindingRectangle.BottomRight.X;
        var axisY = velocity.Y >= 0 ? BindingRectangle.TopLeft.Y : BindingRectangle.BottomRight.Y;
        // Choose testing vortex
        var vertexY = velocity.Y < 0 ? oldBinding.TopLeft.Y : oldBinding.BottomRight.Y;
        var vertexX = velocity.X > 0 ? oldBinding.BottomRight.X : oldBinding.TopLeft.X;
        // Calculate the distance from the vertex to the axes
        var vertexDistanceX = axisX - vertexX;
        var vertexDistanceY = axisY - vertexY;

        // Test trivial cases
        if (velocity.X == 0)
        {
            // If the velocity is vertical, the collision is from the top or the bottom
            return velocity.Y > 0 ? CollisionSide.Top : CollisionSide.Bottom;
        }
        if (velocity.Y == 0)
        {
            // If the velocity is horizontal, the collision is from a side
            return velocity.X > 0 ? CollisionSide.Left : CollisionSide.Right;
        }
        if (Math.Sign(vertexDistanceX * velocity.X) == Math.Sign(vertexDistanceY * velocity.Y)
            && vertexDistanceX * vertexDistanceY != 0)
        {
            // The vertex was already past the tested axes
            return CollisionSide.None;
        }

        // Check if the chosen vertex has crossed exactly one of the axes
        if (vertexDistanceX * vertexDistanceY * (velocity.X) * (velocity.Y) < 0)
        {
            if((axisX  - vertexX) * velocity.X < 0)
            {
                // Hit from the top or the bottom
                return velocity.Y > 0 ? CollisionSide.Top : CollisionSide.Bottom;
            }
            // Hit from a side
            return velocity.X > 0 ? CollisionSide.Left : CollisionSide.Right;
        }
        
        // Construct a vector from the vertex to the point where axes cross
        var vertexAxisVector = new Vector2(vertexDistanceX, vertexDistanceY);

        // Calculate the angle between velocity and the vector
        var angle = GeometryFormulas.GetAngle(vertexAxisVector, velocity);
        if (angle > 0)
        {
            // Hit from the top or the bottom
            return velocity.Y > 0 ? CollisionSide.Top : CollisionSide.Bottom;
        }
        // Otherwise, the collision is from a side
        return velocity.X > 0 ? CollisionSide.Left : CollisionSide.Right;
    }

    /// <summary>
    /// Create a deep copy of Binding
    /// </summary>
    public Binding GetCopy()
    {
        return new Binding(ObjectOrigin, _origin, Size);
    }

    /// <summary>
    /// Move Binding by a vector
    /// </summary>
    public Binding Move(Vector2 vector)
    {
        ObjectOrigin += vector;
        BindingRectangle.Move(vector);
        BindingCircle.CentrePoint += vector;
        return this;
    }

    /// <summary>
    /// Move Binding to a position
    /// </summary>
    public Binding MoveTo(Vector2 position)
    {
        Vector2 moveVector = position - ObjectOrigin;
        return Move(moveVector);
    }

    /// <summary>
    /// Set binding scale to vector
    /// Scales towards the origin
    /// </summary>
    public void Scale(Vector2 vector)
    {
        // Change the size
        Size *= vector;
        // Calculate the change of the relative origin
        var originChange = _origin * (vector - Vector2.One);
        _origin *= vector;
        // Refresh all binding
        Update(ObjectOrigin);
    }

    /// <summary>
    /// Move relative origin by vector
    /// Keeps the rectangle in the same place
    /// </summary>
    public void MoveRelativeOrigin(Vector2 vector)
    {
        _origin += vector;
        Update(ObjectOrigin);
        Move(-vector);
    }

    /// <summary>
    /// Update Binding to a new ObjectOrigin
    /// </summary>
    public void Update(Vector2 objectOrigin)
    {
        ObjectOrigin = objectOrigin;
        Vector2 centre = (objectOrigin + _origin);
        BindingRectangle.TopLeft = centre - Size * 0.5F;
        BindingRectangle.BottomRight = BindingRectangle.TopLeft + Size;
        BindingCircle.CentrePoint = centre;
        BindingCircle.Radius = (float)Math.Sqrt((Size.X * Size.X + Size.Y * Size.Y) * 0.25);
    }

    /// <summary>
    /// Create a Binding that has the size and relative origin of a Sprite
    /// </summary>
    public static Binding FromSprite(Sprite sprite, Vector2 relativeOrigin)
    {
        var binding = new Binding(Vector2.Zero, relativeOrigin, new Vector2(sprite.Width, sprite.Height));
        sprite.SpritePosition = binding.ObjectOrigin;
        return binding;
    }
}