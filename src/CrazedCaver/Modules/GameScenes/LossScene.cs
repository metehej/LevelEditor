using System.Collections.Generic;
using Core;
using Core.Graphics;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace CrazedCaver.Modules.GameScenes;

public class LossScene(CoreModule manager) : Scene(manager)
{
    // UI elements
    private Label _infoLabel;
    private Image _characterImage;
    private int _colorTint = 255;
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
    // Game information for further use
    private string _levelsFileName;
    private bool _cheated;

    public override void Initialize(Dictionary<string, object> kwargs = null)
    {
        // Fetch passed data
        var score = kwargs?.GetValueOrDefault("score", 0) as int? ?? 0;
        var highScore = kwargs?.GetValueOrDefault("highScore", false) as bool? ?? false;
        _cheated = kwargs?.GetValueOrDefault("cheated", false) as bool? ?? false;
        _levelsFileName = kwargs?.GetValueOrDefault("levelsFileName") as string ?? "";

        // Header
        var label = new Label("You lose :(", CoreModule.Font)
        {
            Origin = OriginLocation.TopCentre,
            Position = GetTilePosition(new Point(TilemapWidth / 2, 1)),
            Color = new Color(178, 50, 82),
            Shaded = true
        };
        label.SetWidth(SceneBounds.Size.X * 0.8F);
        AddObject(label, true);

        // Shaded character
        var characterSprite = new Sprite(Manager.TextureAtlas.GetRegion("character"), Vector2.One * 5)
        {
            SpriteOrigin = OriginLocation.BottomCentre,
            SpritePosition = GetTilePosition(new Point(
                TilemapWidth / 2,
                TilemapHeight * 7 / 10 - 2
            ), OriginLocation.BottomLeft)
        };
        var image = new Image(characterSprite);
        AddObject(image);
        _characterImage = image;

        // Score label
        label = new Label($"Total score: {score:D6}", CoreModule.Font)
        {
            Color = _cheated ? Color.Red : Color.Yellow,
            Shaded = _cheated,
            Origin = OriginLocation.BottomCentre,
            Position = GetTilePosition(new Point(TilemapWidth / 2, TilemapHeight * 7 / 10), OriginLocation.BottomLeft)
        };
        label.SetWidth(TileSize * TilemapWidth * 0.4F);
        var height = label.Height;
        AddObject(label, true);
        // Further information
        if (highScore || _cheated)
        {
            // Nudge the score label
            label.Origin = OriginLocation.BottomRight;
            label.Position = GetTilePosition(new Point(TilemapWidth * 3 / 5 - 1, TilemapHeight * 7 / 10),
                OriginLocation.BottomRight);
            // Add high score information
            label = new Label(_cheated ? "Cheated :|" : "New high!", CoreModule.Font)
            {
                Origin = OriginLocation.BottomLeft,
                Color = _cheated ? Color.Red : _winScreenColors[0],
                Shaded = true,
                Position = GetTilePosition(new Point(TilemapWidth * 3 / 5 + 1, TilemapHeight * 7 / 10),
                    OriginLocation.BottomLeft)
            };
            label.SetHeight(height);
            AddObject(label, true);
            _infoLabel = label;
        }

        // Key binds info
        label = new Label("Press ENTER to start over", CoreModule.Font)
        {
            Origin = OriginLocation.BottomCentre,
            Position = GetTilePosition(new Point(16, TilemapHeight * 7 / 10 + 2), OriginLocation.BottomLeft),
            Shaded = true
        };
        label.SetHeight(height);
        AddObject(label, true);

        label = new Label("Press ESC to exit to menu", CoreModule.Font)
        {
            Origin = OriginLocation.TopCentre,
            Position = GetTilePosition(new Point(16, TilemapHeight * 7 / 10 + 4), OriginLocation.TopLeft),
            Shaded = true
        };
        label.SetHeight(TileSize * 0.8F);
        AddObject(label, true);
    }

    public override void Update(GameTime gameTime)
    {
        if (Controls.Keyboard.WasKeyPressed(Keys.Enter))
        {
            // Continue game
            Manager.LoadScene("game", new Dictionary<string, object>()
            {
                { "levelsFileName", _levelsFileName }
            });
            return;
        }
        // ESC is handled by Game1 (ensures that the game can always be closed)
        base.Update(gameTime);
        // Update the character image tint if it isn't fully shaded yet
        if (_colorTint > 100 && _updateCounter % 2 == 0)
        {
            _colorTint -= 10;
            _characterImage.Tint = new Color(_colorTint, _colorTint, _colorTint);
        }
        // Update the info label color if it exists and enough time has passed
        if (_cheated || _infoLabel is null || ++_updateCounter < 45) return;
        _colorIndex = ++_colorIndex % _winScreenColors.Length;
        _updateCounter = 0;
        _infoLabel.Color = _winScreenColors[_colorIndex];
    }
}
