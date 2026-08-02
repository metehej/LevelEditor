using System.Collections.Generic;
using System.Linq;
using Core.Simulation;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Core;

#region Interfaces
/// <summary>
/// An interface for scene objects that can be updated and drawn
/// </summary>
public interface ISceneObject
{
    /// <summary>
    /// Indicates whether the object should be updated
    /// </summary>
    bool Enabled { get; set; }
    /// <summary>
    /// Indicates whether the object should be drawn
    /// </summary>
    bool Visible { get; set; }
    /// <summary>
    /// Carry out any updates for the scene object
    /// </summary>
    void Update(GameTime gameTime, Scene parentScene);
    /// <summary>
    /// Draw the scene object using the provided sprite batch, offset, and UI scale
    /// </summary>
    void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale);
    /// <summary>
    /// Draw debug information for the scene object
    /// </summary>
    void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale);
}

/// <summary>
/// Extended SceneObject interface that has a Binding property.
/// </summary>
public interface ISceneObjectSimulated : ISceneObject
{
    Binding Binding { get; }
}

/// <summary>
/// Interface for objects that have an initial state and may be reset to it
/// </summary>
public interface IResettable
{
    /// <summary>
    /// Reset the object to its initial state.
    /// </summary>
    void Reset();
}
#endregion

#region Classes
public class Scene(Binding sceneBounds, CoreModule manager)
{
    public bool UpdateSceneObjects { get; protected set; }
    protected readonly CoreModule Manager = manager;
    protected List<ISceneObject> SceneObjects = []; // Always update
    protected List<ISceneObject> SimulatedSceneObjects = []; // Update only when UpdateSceneObjects is true
    // The width and height of the scene in tiles
    public int TilemapWidth => (int)SceneBounds.Size.X / CoreModule.WidthTileCount;
    public int TilemapHeight => (int)SceneBounds.Size.Y / CoreModule.WidthTileCount;
    // The size of a single tile
    public float TileSize => SceneBounds.Size.X / CoreModule.WidthTileCount;
    public Color BackgroundColor { get; set; } = CoreModule.BackgroundColor;

    /// <summary>
    /// A Binding wrapping the entire displayed scene
    /// </summary>
    public Binding SceneBounds { get; private set; } = sceneBounds;

    /// <summary>
    /// Create a new Scene, the size of which is the entire Canvas.
    /// </summary>
    public Scene(CoreModule manager) : this(
        new Binding(Vector2.Zero, 
            CoreModule.Canvas.Size.ToVector2() * 0.5F, 
            CoreModule.Canvas.Size.ToVector2()),
        manager) { }

    #region Scene control

    /// <summary>
    /// Initializes all scene objects
    /// </summary>
    public virtual void Initialize(Dictionary<string, object> kwargs = null)
    {
        // Empty by default, can be overridden by derived classes
    }

    /// <summary>
    /// Reset resettable objects in the scene
    /// </summary>
    public virtual void Reset()
    {
        foreach (var sceneObject in SceneObjects.Union(SimulatedSceneObjects))
        {
            if (sceneObject is IResettable resettable)
            {
                resettable.Reset();
            }
        }
    }
    #endregion

    #region Object management
    /// <summary>
    /// Toggle scene updates for simulated objects
    /// </summary>
    public void ToggleUpdate()
    {
        UpdateSceneObjects ^= true;
    }

    /// <summary>
    /// Add object to scene's set
    /// </summary>
    /// <param name="newObject">object to add</param>
    /// <param name="updateAlways">true if object should ignore UpdateSceneObjects</param>
    public void AddObject(ISceneObject newObject, bool updateAlways = false)
    {
        if (updateAlways) SceneObjects.Add(newObject);
        else SimulatedSceneObjects.Add(newObject);
    }

    /// <summary>
    /// Return all scene objects that intersect with the sender object's binding rectangle
    /// </summary>
    public ISceneObjectSimulated[] FindCollidingEntities(ISceneObjectSimulated sender)
    {
        var collidingEntities = SceneObjects.Where(
            sceneObject =>
                sceneObject.Enabled &&
                sceneObject != sender &&
                sceneObject is ISceneObjectSimulated simulatedEntity &&
                simulatedEntity.Binding.Intersects(sender.Binding)
        ).Cast<ISceneObjectSimulated>();
        collidingEntities = collidingEntities.Union(SimulatedSceneObjects.Where(
            sceneObject =>
                sceneObject.Enabled &&
                sceneObject != sender &&
                sceneObject is ISceneObjectSimulated simulatedEntity &&
                simulatedEntity.Binding.Intersects(sender.Binding)
        ).Cast<ISceneObjectSimulated>());
        return collidingEntities.ToArray();
    }

    /// <summary>
    /// Return all scene objects that intersect with the specified rectangle
    /// </summary>
    public ISceneObjectSimulated[] FindCollidingEntities(BindingRectangle rectangle)
    {
        var collidingEntities = SceneObjects.Where(
            sceneObject =>
                sceneObject.Enabled &&
                sceneObject is ISceneObjectSimulated simulatedEntity &&
                simulatedEntity.Binding.Intersects(rectangle)
        ).Cast<ISceneObjectSimulated>();
        collidingEntities = collidingEntities.Union(SimulatedSceneObjects.Where(
            sceneObject =>
                sceneObject.Enabled &&
                sceneObject is ISceneObjectSimulated simulatedEntity &&
                simulatedEntity.Binding.Intersects(rectangle)
        ).Cast<ISceneObjectSimulated>());
        return collidingEntities.ToArray();
    }

    /// <summary>
    /// Return Canvas position of the specified part of the tile
    /// </summary>
    public Vector2 GetTilePosition(Point tilePosition, OriginLocation origin = OriginLocation.TopLeft)
    {
        var tileSize = SceneBounds.Size.X / CoreModule.WidthTileCount;
        var position = new Vector2(tilePosition.X * tileSize, tilePosition.Y * tileSize);
        // Adjust position based on origin
        position.X += ((int)origin % 3) switch
        {
            // Set origin X
            2 => tileSize,
            1 => tileSize * 0.5F,
            _ => 0
        };

        position.Y += (int)origin switch
        {
            // Set origin Y
            < 3 => 0,
            < 6 => tileSize * 0.5F,
            _ => tileSize
        };
        return position;
    }
    #endregion

    #region Updates
    /// <summary>
    /// Updates all SceneObjects that are active
    /// </summary>
    public virtual void Update(GameTime gameTime)
    {
        foreach (var sceneObject in SceneObjects)
        {
            sceneObject.Update(gameTime, this);
        }
        if (!UpdateSceneObjects) return;
        foreach (var sceneObject in SimulatedSceneObjects)
        {
            sceneObject.Update(gameTime, this);
        }
    }

    /// <summary>
    /// Draw the Scene and all of its objects
    /// </summary>
    public virtual void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // Draw the scene bounds (Background)
        CoreModule.Instance.DrawRectangle(SceneBounds.BindingRectangle.TopLeft,SceneBounds.BindingRectangle.BottomRight,
            color: BackgroundColor, drawLayer: CoreModule.GetDrawLayer(CoreModule.DrawLayers.Background));
        foreach (var sceneObject in SceneObjects)
        {
            sceneObject.Draw(gameTime, batch, offset, uiScale);
        }
        foreach (var sceneObject in SimulatedSceneObjects)
        {
            sceneObject.Draw(gameTime, batch, offset, uiScale);
        }
    }

    /// <summary>
    /// Draw debug information for the Scene and it's objects
    /// </summary>
    public void DrawDebugInfo(SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // Draw all object's information
        foreach (var sceneObject in SceneObjects)
        {
            sceneObject.DrawDebugInfo(batch, offset, uiScale);
        }
        foreach (var sceneObject in SimulatedSceneObjects)
        {
            sceneObject.DrawDebugInfo(batch, offset, uiScale);
        }
        // Draw Scene Tilemap grid
        var layer = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Debug);
        var tileSize = (int)SceneBounds.Size.X / CoreModule.WidthTileCount;
        for (var x = 0; x <= SceneBounds.BindingRectangle.BottomRight.X; x += tileSize)
        {
            CoreModule.Instance.DrawRectangle(new Vector2(x, 0), new Vector2(x, SceneBounds.Size.Y), Color.Black, layer);
        }
        for (var y = 0; y <= SceneBounds.BindingRectangle.BottomRight.Y; y += tileSize)
        {
            CoreModule.Instance.DrawRectangle(new Vector2(0, y), new Vector2(SceneBounds.Size.X, y), Color.Black, layer);
        }
    }
    #endregion

}
#endregion