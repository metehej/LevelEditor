using System;
using Microsoft.Xna.Framework;

namespace Core.Simulation;

public static class GeometryFormulas
{
    /// <summary>
    /// Returns the X coordinate of the point [x, 0]
    /// </summary>
    public static float FindXIntersect(Vector2 point, Vector2 velocity, Vector2 axes)
    {
        var growth = velocity.Y / velocity.X;
        return growth == 0 ? float.MaxValue : -(point.Y - axes.Y) / growth + point.X;
    }

    /// <summary>
    /// Returns the y coordinate of the point [0, y]
    /// </summary>
    public static float FindYIntersect(Vector2 point, Vector2 velocity, Vector2 axes)
    {
        (point.X, point.Y) = (point.Y, -point.X);
        (velocity.X, velocity.Y) = (velocity.Y, -velocity.X);
        (axes.X, axes.Y) = (axes.Y, -axes.X);
        return FindXIntersect(point, velocity, axes);
    }

    /// <summary>
    /// Returns the angle between two vectors in radians.
    /// </summary>
    public static double GetAngle(Vector2 vectorA, Vector2 vectorB)
    {
        return Math.Atan2(vectorB.Y - vectorA.Y, vectorB.X - vectorA.X);
    }
}