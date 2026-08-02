// PROJECT: CrazedCaver (based on ManicMiner)
// AUTHOR: Matěj Kretek
// SEMESTER: 2024/2025 Summer
// CLASS: NPRG031




using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Core.Graphics;
using CrazedCaver.Modules;
using CrazedCaver.Modules.GameScenes;

namespace CrazedCaver;

public class Game1 : Core.CoreModule
{
    // File names for textures and levels
    private const string AtlasRegionsFile = "textureRegions.xml";
    public readonly string LevelsFile = "levels.xml";

    private readonly Color[] _backgroundColors =
    [
        Color.LightSlateGray,
        Color.DarkSlateGray,
        Color.Black,
        Color.Azure, 
        Color.Red, 
        Color.Olive, 
        Color.Indigo, 
        Color.Yellow, 
        Color.AntiqueWhite,
        Color.LightGray
    ];
    private int _backgroundColorIndex;

    // Current high score
    public int HighScore { get; set; }

    public Game1() :
        base("Crazed Caver", 1080, 720, true, false)
    {
        BackgroundColor = _backgroundColors[0];
    }

    protected override void LoadContent()
    {
        _textureAtlas = TextureAtlas.FromFile(Content, AtlasRegionsFile);
        Font = Content.Load<SpriteFont>("PixelFont/PublicPixel");
        // Create loaders for all scenes
        // In this case, all are trivial
        SceneLoaders["title"] = () => new TitleScene(this);
        SceneLoaders["game"] = () => new GameScene(this);
        SceneLoaders["win"] = () => new WinScene(this);
        SceneLoaders["loss"] = () => new LossScene(this);
        LoadFirstScene();
        base.LoadContent();
    }

    protected override void Update(GameTime gameTime)
    {
        Controls.Update();
        // Check for common input
        if (Controls.Exit())
        {
            if (CurrentScene is not TitleScene)
            {
                LoadFirstScene();
            }
            else
            {
                Exit();
            }
        }
        if (Controls.Debug()) DebugMode ^= true;
        if (Controls.FullScreen())
        {
            Graphics.ToggleFullScreen();
        }
        if (Controls.BackgroundSwitch())
        {
            // Switch background color
            _backgroundColorIndex = ++_backgroundColorIndex % _backgroundColors.Length;
            BackgroundColor = _backgroundColors[_backgroundColorIndex];
            if (CurrentScene is GameScene) return;
            CurrentScene.BackgroundColor = BackgroundColor;
        }
        if (Controls.Pause()) CurrentScene.ToggleUpdate();

        // Update Scene
        CurrentScene.Update(gameTime);

        // Call base update to MonoGame
        base.Update(gameTime);
    }

    protected override void Draw(GameTime gameTime)
    {
        base.Draw(gameTime);
        GraphicsDevice.Clear(BackgroundColor);
        // Start sprite batch drawing
        // PointClamp sampler state is used to avoid blurring of pixel art
        // FrontToBack sort mode is used in tandem with GetDrawLayer to ensure correct draw order
        SpriteBatch.Begin(samplerState:SamplerState.PointClamp, sortMode: SpriteSortMode.FrontToBack);
        // Draw current Scene
        CurrentScene.Draw(gameTime, SpriteBatch, Offset, UiScale);
        // Draw debug info
        if (DebugMode)
        {
            DrawDebugInfo(gameTime);
        }
        // Draw mouse cursor
        DrawSquare((Controls.Mouse.Position.ToVector2() - Offset) / UiScale, Color.Black, size: 7, drawLayer:GetDrawLayer(DrawLayers.Cursor) - 0.01F);
        DrawSquare((Controls.Mouse.Position.ToVector2() - Offset) / UiScale, Color.Yellow, size: 5, drawLayer: GetDrawLayer(DrawLayers.Cursor));
        SpriteBatch.End();
    }

    protected void DrawDebugInfo(GameTime gameTime)
    {
        // Draw information about mouse position
        SpriteBatch.DrawString(Font, $"Mouse: {Controls.Mouse.X - Offset.X:N0}, " +
                                     $"{Controls.Mouse.Y - Offset.Y:N0}",
            new Vector2(10, 10), Color.DarkSlateGray, 0, Vector2.Zero, Vector2.One * 0.25F, SpriteEffects.None, 0.41F);
        SpriteBatch.DrawString(Font, $"Mouse: {Controls.Mouse.X - Offset.X:N0}, " +
                                     $"{Controls.Mouse.Y - Offset.Y:N0}",
            new Vector2(12, 12), Color.White, 0, Vector2.Zero, Vector2.One * 0.25F, SpriteEffects.None, 0.4F);

        // Draw information about current Scene
        CurrentScene.DrawDebugInfo(SpriteBatch, Offset, UiScale);
    }
}