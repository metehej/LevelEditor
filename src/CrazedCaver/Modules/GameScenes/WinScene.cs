using System.Collections.Generic;
using Core;
using Core.Graphics;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace CrazedCaver.Modules.GameScenes;

public class WinScene(CoreModule manager) : Scene(manager)
{
    // UI elements
    private Label _title, _infoLabel;
    private readonly Color[] _winScreenColors =
    [
        Color.White,
        Color.Yellow,
        Color.Indigo,
        Color.Coral,
        Color.Purple
    ];
    private int _colorIndex;
    private int _updateCounter;
    // Data to display and pass if the player continues the game
    private int _score, _lives;
    private string _levelsFileName;
    private bool _cheated;
    public override void Initialize(Dictionary<string, object> kwargs = null)
    {
        // Fetch info from the dictionary
        _score = kwargs?.GetValueOrDefault("score", 0) as int? ?? 0;
        _lives = kwargs?.GetValueOrDefault("lives", 0) as int? ?? 0;
        _levelsFileName = kwargs?.GetValueOrDefault("levelsFileName") as string ?? "";
        var highScore = kwargs?.GetValueOrDefault("highScore", false) as bool? ?? false;
        _cheated = kwargs?.GetValueOrDefault("cheated", false) as bool? ?? false;

        #region UI
        // Header
        _title = new Label("You win!", CoreModule.Font)
        {
            Origin = OriginLocation.BottomCentre,
            Position = GetTilePosition(new Point(17, 8)),
            Color = _winScreenColors[_colorIndex],
            Shaded = true
        };
        _title.SetWidth(SceneBounds.Size.X * 0.8F);
        AddObject(_title, true);

        // Lives label
        var label = new Label("Lives  left: ", CoreModule.Font)
        {
            Color = Color.Yellow,
            Origin = OriginLocation.MiddleLeft,
            Position = GetTilePosition(new Point(4, 13), OriginLocation.MiddleLeft)
        };
        label.SetWidth(TileSize * 8);
        var height = label.Height;
        AddObject(label, true);

        label = new Label($"{_lives}", CoreModule.Font)
        {
            Color = Color.Yellow,
            Origin = OriginLocation.MiddleRight,
            Position = GetTilePosition(new Point(15, 13), OriginLocation.MiddleRight)
        };
        label.SetHeight(height);
        AddObject(label, true);

        // Score label
        label = new Label($"Total score: ", CoreModule.Font)
        {
            Color = _cheated ? Color.Red : Color.Yellow,
            Shaded = _cheated,
            Origin = OriginLocation.MiddleLeft,
            Position = GetTilePosition(new Point(4, 11), OriginLocation.MiddleLeft)
        };
        label.SetHeight(height);
        AddObject(label, true);

        label = new Label($"{_score:D6}", CoreModule.Font)
        {
            Color = _cheated ? Color.Red : Color.Yellow,
            Shaded = _cheated,
            Origin = OriginLocation.MiddleRight,
            Position = GetTilePosition(new Point(15, 11), OriginLocation.MiddleRight)
        };
        label.SetHeight(height);
        AddObject(label, true);

        // If more info exists, display it
        if (highScore || _cheated)
        {
            // Add high score information
            label = new Label(_cheated ? "Cheated :|" : "New high!", CoreModule.Font)
            {
                Origin = OriginLocation.MiddleLeft,
                Color = _cheated ? Color.Red : _winScreenColors[0],
                Shaded = true,
                Position = GetTilePosition(new Point(17, 11), OriginLocation.MiddleLeft)
            };
            label.SetHeight(height);
            AddObject(label, true);
            _infoLabel = label;
        }

        // Key instructions
        label = new Label("Press ENTER to continue", CoreModule.Font)
        {
            Origin = OriginLocation.BottomCentre,
            Position = GetTilePosition(new Point(16, 16), OriginLocation.BottomCentre)
        };
        label.SetHeight(TileSize * 0.8F);
        AddObject(label, true);

        label = new Label("Press ESC to exit to menu", CoreModule.Font)
        {
            Origin = OriginLocation.TopCentre,
            Position = GetTilePosition(new Point(16, 18), OriginLocation.TopCentre)
        };
        label.SetHeight(TileSize * 0.8F);
        AddObject(label, true);
        #endregion
    }

    public override void Update(GameTime gameTime)
    {
        if (Controls.Keyboard.WasKeyPressed(Keys.Enter))
        {
            // Continue game
            Manager.LoadScene("game", new Dictionary<string, object>()
            {
                { "initialLives", _lives },
                { "initialScore", _score },
                { "levelsFileName", _levelsFileName },
                { "cheated", _cheated}
            });
            return;
        }
        // ESC is handled by Game1.cs
        base.Update(gameTime);
        if (++_updateCounter <= 45) return;
        // Change the color of the title
        _colorIndex = ++_colorIndex % _winScreenColors.Length;
        _title.Color = _winScreenColors[_colorIndex];
        _updateCounter=0;
        if (_cheated || _infoLabel is null) return;
        // Change the color of the info label if it isn't "cheated"
        _infoLabel.Color = _winScreenColors[_colorIndex];
    }
}