using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using Core.Graphics;

namespace Core;

#region  Enums
/// <summary>
/// Origin location for sprite and binding positions
/// </summary>
public enum OriginLocation
{
    TopLeft,
    TopCentre,
    TopRight,
    MiddleLeft,
    MiddleCentre,
    MiddleRight,
    BottomLeft,
    BottomCentre,
    BottomRight
}
#endregion
public class CoreModule : Game
{
    #region Variables

    #region Game instance variables
    public static CoreModule Instance { get; private set; }
    public static GraphicsDeviceManager Graphics { get; private set; }
    public new static GraphicsDevice GraphicsDevice { get; private set; }
    public static SpriteBatch SpriteBatch { get; private set; }
    public new static ContentManager Content { get; private set; }
    #endregion

    #region Simulation variables
    // Gravitation variables
    public static readonly float DefaultGravitation = 0.35F;
    public static Vector2 Gravitation { get; set; } = new(0, DefaultGravitation);

    // Passive decay of speed
    public static float SpeedLoss { get; protected set; } = 0.750F;
    // Physics and graphics simulation canvas
    protected static Rectangle _canvas;
    public static Rectangle Canvas => _canvas;
    #endregion

    #region Graphics variables
    // Vector for translation between Simulation and UI coordinates
    private static Vector2 _uiScale = Vector2.One;
    private static Vector2 _uiScaleInverse = Vector2.One;
    public static Vector2 UiScale => _uiScale;

    // How much are images upscaled compared to 8x8 image tiles
    public static readonly float ImageUpscaleLevel = 4;
    public static readonly float OriginalScreenWidth = 256;
    public static readonly float ScreenRatioWidth = 4, ScreenRatioHeight = 3;
    public static readonly int WidthTileCount = 32;
    public static readonly float SimulationCanvasWidth = OriginalScreenWidth * ImageUpscaleLevel;

    // Default font
    public static SpriteFont Font { get; protected set; }
    // Empty texture for solid color rendering
    public static Texture2D EmptyTexture { get; protected set; }

    // Offset for UI (keeping Canvas side ratio)
    protected Vector2 Offset = Vector2.Zero;
    private bool _sizeHasChanged = false;

    public static Color BackgroundColor { get; protected set; } = Color.White;

    /// <summary>
    /// Use GetDrawLayer to convert to floats
    /// </summary>
    public enum DrawLayers
    {
        Background,
        ImageBackground,
        Sprites,
        Entities,
        Character, 
        Interface,
        Debug,
        HigherDebug,
        Cursor
    }
    /// <summary>
    /// Turn DrawLayer into a usable float
    /// </summary>
    public static float GetDrawLayer(DrawLayers layer)
    {
        return (float)layer / 10;
    }
        
    protected TextureAtlas _textureAtlas;
    public TextureAtlas TextureAtlas => _textureAtlas;
    #endregion

    #region Miscellaneous variables
    public static bool DebugMode { get; protected set; } = false;
    #endregion

    #region Scene management

    protected Dictionary<string, Func<Scene>> SceneLoaders = [];
    protected string FirstScene = "title";
    public static Scene CurrentScene { get; protected set; }
    #endregion

    #endregion

    /// <summary>
    /// Creates a new CoreModule instance.
    /// </summary>
    public CoreModule(string title, int width, int height, bool resizeable, bool mouse)
    {
        // Ensure that multiple cores are not created.
        if (Instance != null)
        {
            throw new InvalidOperationException($"Only a single CoreModule instance can be created");
        }

        // Store reference to engine for global member access.
        Instance = this;

        // Initialize the game graphics
        Graphics = new GraphicsDeviceManager(this)
        {
            PreferredBackBufferWidth = width,
            PreferredBackBufferHeight = height,
            HardwareModeSwitch = false
        };
        Window.AllowUserResizing = resizeable;
        IsMouseVisible = mouse;
        Graphics.ApplyChanges();

        // Set the window title.
        Window.Title = title;

        // Initialize the content manager.
        Content = base.Content;
        Content.RootDirectory = "Content";
        
        // Create a canvas
        _canvas = new Rectangle(new Point(0, 0),
            new Point((int)Math.Ceiling(SimulationCanvasWidth),
                (int)Math.Ceiling(SimulationCanvasWidth * ScreenRatioHeight / ScreenRatioWidth)));
    }

    protected override void Initialize()
    {
        base.Initialize();

        // Set the core's graphics device to a reference of the base Game's graphics device.
        GraphicsDevice = base.GraphicsDevice;

        // Create the sprite batch instance.
        SpriteBatch = new SpriteBatch(GraphicsDevice);

        Window.ClientSizeChanged += (sender, args) =>
        {
            // Resize UI when the window size changes
            _sizeHasChanged = true;
        };

        // Initialize empty Texture as a placeholder

        EmptyTexture = new(GraphicsDevice, 1, 1);
        EmptyTexture.SetData([Color.White]);

        // Initialize the UI scale and offset
        ResizeUi();
    }

    #region Graphics
    /// <summary>
    /// Resize the UI if needed
    /// </summary>
    protected override void Draw(GameTime gameTime)
    {
        if(_sizeHasChanged)
        {
            ResizeUi();
            _sizeHasChanged = false;
        }
        base.Draw(gameTime);
    }

    /// <summary>
    /// Resize the UI based on the current window size and predefined constants
    /// </summary>
    protected void ResizeUi()
    {
        // Calculate the maximum size of the canvas to keep the ratio
        Vector2 screenSize = new(GraphicsDevice.Viewport.Width, GraphicsDevice.Viewport.Height);
        if (screenSize.Y * ScreenRatioWidth / ScreenRatioHeight > screenSize.X)
        {
            Offset.X = 0;
            Offset.Y = (float)((screenSize.Y - screenSize.X * ScreenRatioHeight / ScreenRatioWidth) * 0.5);
        }
        else
        {
            Offset.Y = 0;
            Offset.X = (float)((screenSize.X - screenSize.Y * ScreenRatioWidth / ScreenRatioHeight) * 0.5);
        }
        // Set uiScale
        var uiScale = (screenSize.X - Offset.X * 2) / SimulationCanvasWidth;

        // Add Scene offset to the offset
        _uiScale.X = uiScale;
        _uiScale.Y = uiScale;
        uiScale = 1 / uiScale;
        _uiScaleInverse.X = uiScale;
        _uiScaleInverse.Y = uiScale;
    }

    /// <summary>
    /// Draw a square at the given position.
    /// </summary>
    public void DrawSquare(Vector2 centre, Color? color = null, int size = 3, float? drawLayer = null)
    {
        var tl = (centre * UiScale + Offset).ToPoint() - new Point(size / 2, size / 2);
        SpriteBatch.Draw(EmptyTexture, new Rectangle(tl, new Point(size, size)), 
            null, color ?? Color.White, 0, Vector2.Zero, SpriteEffects.None, drawLayer ?? GetDrawLayer(DrawLayers.Background) + 0.01F);
    }

    /// <summary>
    /// Draw a rectangle between two points.
    /// </summary>
    public void DrawRectangle(Vector2 topLeft, Vector2 bottomRight, Color? color = null, float? drawLayer = null)
    {
        var tl = (topLeft * UiScale + Offset).ToPoint();
        var size = (bottomRight * UiScale + Offset).ToPoint() - tl;
        size.X  = size.X < 1 ? 1 : size.X;
        size.Y = size.Y < 1 ? 1 : size.Y;
        SpriteBatch.Draw(EmptyTexture, new Rectangle(tl, size),
            null, color ?? Color.White, 0, Vector2.Zero, SpriteEffects.None, drawLayer ?? GetDrawLayer(DrawLayers.Background) + 0.01F);
    }
    #endregion

    #region Scene management
    /// <summary>
    /// Load a scene by its name
    /// Needs a registered scene loader for the scene name
    /// </summary>
    public void LoadScene(string sceneName, Dictionary<string, object> kwargs = null)
    {
        if (SceneLoaders.Count == 0)
        {
            // Load a clean scene
            CurrentScene = new Scene(this);
            CurrentScene.Initialize();
            return;
        }
        var loader = SceneLoaders.GetValueOrDefault(sceneName);
        if (loader == null)
        {
            throw new ArgumentException($"Scene '{sceneName}' is not registered.");
        }
        CurrentScene = loader() ?? new Scene(this);
        // Initialize scene using kwargs 
        CurrentScene.Initialize(kwargs);
    }

    /// <summary>
    /// Load the first scene defined or specified in the variable
    /// </summary>
    public void LoadFirstScene()
    {
        if(SceneLoaders.Count == 0)
        {
            CurrentScene = new Scene(this);
            CurrentScene.Initialize();
            return;
        }

        CurrentScene = SceneLoaders.GetValueOrDefault(FirstScene, null)?.Invoke() ?? SceneLoaders.First().Value.Invoke();
        CurrentScene.Initialize();
    }
    #endregion

    /// <summary>
    /// Get a texture region by its name
    /// </summary>
    public TextureRegion GetRegion(string name)
    {
        return _textureAtlas?.GetRegion(name);
    }
}