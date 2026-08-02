using System;
using System.Collections.Generic;
using System.Threading.Tasks.Sources;
using Core;
using Core.Graphics;
using Core.Simulation;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace CrazedCaver.Modules.GameScenes;

public class GameScene(CoreModule manager) : Scene(manager)
{
    // Constant values
    private const int MaxLives = 10, InitialLives = 3;
    private string _levelsFileName;
    private bool _infiniteLives;

    private int _lives, _totalKeys;
    public int RemainingKeys { get; set; } = 0;
    // _score represents progress towards the next life while _totalScore represents the total score
    private int _score, _totalScore;
    // Total air capacity for the current level
    private float _airCapacity;
    private float _remainingAir, _airBarWidth;
    // Speed at which air decays
    // _remainingAir is reduced by this value every update (~60 times/second)
    private float _airDecayValue;
    // UI elements
    private Label _levelName, _highScoreLabel, _scoreLabel;
    private BackgroundRectangle _airBar, _shadowRectangle;
    private Image _livesIcon;
    // Levels control variables
    private int _levelsCount;
    private int _currentLevel;

    public int Score
    {
        get => _score;
        set
        {
            // Add the score to the total and check if another life should be given
            _totalScore += value - _score;
            _lives += value / 10000;
            if (_lives > MaxLives) _lives = MaxLives;
            _score = value % 10000; 
            _scoreLabel.Text = _totalScore.ToString("D6");
        }
    }
    // WinCondition means the player has reached the current exit door
    // LossCondition means the player should die this update
    private bool _winCondition;
    public bool WinCondition
    {
        get => _winCondition;
        set
        {
            if (!(WinCondition ^ value)) return;
            _winCondition = value;
            _shadowRectangle.Visible = value;
            UpdateSceneObjects = !value;
        }
    }
    private bool _lossCondition;

    public bool LossCondition
    {
        private get => _lossCondition;
        set => _lossCondition = value && !_winCondition;
    }

    // If player uses any cheats this run, Cheated is set to true
    private bool _cheated;
    private bool Cheated
    {
        get => _cheated;
        set
        {
            if (_cheated || _cheated == value) return;
            _cheated = value;
            _scoreLabel.Color = Color.Red;
        }
    }

    public override void Initialize(Dictionary<string, object> kwargs = null)
    {
        _levelsFileName = (string)(kwargs?.GetValueOrDefault("levelsFileName") ?? "");
        if (_levelsFileName == "") throw new ArgumentException("Levels fileName is empty", nameof(kwargs));
        #region UI elements
        // All these are added as not simulated, therefore aren't reloaded each new level
        #region Rectangles
        // Draw UI background rectangles
        var tileSize = SceneBounds.Size.X / CoreModule.WidthTileCount;
        var position = new Vector2(0, 16 * tileSize);
        var rectangle = new BackgroundRectangle(position, new Vector2(SceneBounds.Size.X, tileSize * 9))
        {
            Color = new Color(190, 190, 63)
        };
        AddObject(rectangle, true);

        position.Y += tileSize;
        rectangle = new BackgroundRectangle(position, new Vector2(SceneBounds.Size.X, tileSize * 8))
        {
            Color = new Color(233, 54, 38)
        };
        rectangle.DrawLayer += 0.01F;
        AddObject(rectangle, true);

        position += new Vector2(tileSize * 10, 0);
        rectangle = new BackgroundRectangle(position, new Vector2(tileSize * 22, tileSize * 8))
        {
            Color = new Color(116, 251, 77)
        };
        rectangle.DrawLayer += 0.02F;
        AddObject(rectangle, true);

        position = new Vector2(0, 18 * tileSize); ;
        rectangle = new BackgroundRectangle(position, new Vector2(SceneBounds.Size.X, tileSize * 7))
        {
            Color = Color.Black
        };
        rectangle.DrawLayer += 0.03F;
        AddObject(rectangle, true);

        position = new Vector2(3 * tileSize, 17 * tileSize) + new Vector2(20, 6);
        rectangle = new BackgroundRectangle(position, new Vector2(tileSize * 29 - 30, tileSize - 10))
        {
            Color = Color.Black
        };
        rectangle.DrawLayer += 0.04F;
        AddObject(rectangle, true);

        // Air bar
        position += new Vector2(5, 5);
        rectangle = new BackgroundRectangle(position, new Vector2(tileSize * 29 - 40, tileSize - 20))
        {
            Color = Color.White
        };
        rectangle.DrawLayer += 0.05F;
        AddObject(rectangle, true);
        _airBar = rectangle;
        _airBarWidth = rectangle.Size.X;

        // A shadow when player wins
        position = new Vector2(0, 0);
        rectangle = new BackgroundRectangle(position, new Vector2(tileSize*32, tileSize*16))
        {
            Color = new Color(Color.DarkSlateBlue, 0.5F),
            DrawLayer = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Character) + 0.01F,
            Visible = false
        };
        AddObject(rectangle, true);
        _shadowRectangle = rectangle;
        #endregion

        #region Text
        // Add static text
        position = new Vector2(0, 17.5F * tileSize + 2);;
        var label = new Label("AIR", CoreModule.Font)
        {
            Origin = OriginLocation.MiddleLeft,
            Position = position,
            Color = Color.White
        };
        rectangle.DrawLayer += 0.004F;
        label.SetHeight(tileSize - 4);
        AddObject(label, true);

        position = new Vector2(10 * tileSize, 19.5F * tileSize + 2);
        label = new Label("High Score", CoreModule.Font)
        {
            Origin = OriginLocation.MiddleRight,
            Position = position,
            Color = Color.Yellow
        };

        label.SetHeight(tileSize - 4);
        AddObject(label, true);
        position = new Vector2(23 * tileSize, 19.5F * tileSize + 2);
        label = new Label("Score", CoreModule.Font)
        {
            Origin = OriginLocation.MiddleRight,
            Position = position,
            Color = Color.Yellow
        };
        label.SetHeight(tileSize - 4);
        AddObject(label, true);

        // Add dynamic labels
        // Level name
        position = new Vector2(16 * tileSize, 16 * tileSize + 2);
        label = new Label("Placeholder lorem ipsum name", CoreModule.Font)
        {
            Origin = OriginLocation.TopCentre,
            Position = position,
            Color = Color.Black
        };
        label.SetHeight(tileSize - 4);
        AddObject(label, true);
        _levelName = label;

        // High score and score labels
        position = new Vector2(11 * tileSize, 19.5F * tileSize + 2);
        label = new Label("000000", CoreModule.Font)
        {
            Origin = OriginLocation.MiddleLeft,
            Position = position,
            Color = Color.Yellow
        };
        label.SetHeight(tileSize - 4);
        AddObject(label, true);
        _highScoreLabel = label;

        position = new Vector2(26 * tileSize, 19.5F * tileSize + 2);
        label = new Label("000000", CoreModule.Font)
        {
            Origin = OriginLocation.MiddleLeft,
            Position = position,
            Color = Color.Yellow
        };
        label.SetHeight(tileSize - 4);
        AddObject(label, true);
        _scoreLabel = label;
        if (Manager is Game1 manager) _highScoreLabel.Text = manager.HighScore.ToString("D6");
        #endregion

        // Images showing the lives
        _livesIcon = new Image(new Sprite(Manager.GetRegion("character"))
        {
            SpritePosition = new Vector2(tileSize * 1.5F, tileSize * 21),
            SpriteOrigin = OriginLocation.BottomCentre,
            Tint = Color.Cyan,
            DrawLayer = CoreModule.GetDrawLayer(CoreModule.DrawLayers.Debug) + 0.01F
        });
        #endregion

        _lives = (int)(kwargs?.GetValueOrDefault("initialLives", null) ?? InitialLives);
        Score = (int)(kwargs?.GetValueOrDefault("initialScore", null) ?? 0);
        Cheated = kwargs?.GetValueOrDefault("cheated", false) as bool? ?? false;
        // Fetch levels
        _levelsCount = LevelLoader.CountLevels(CoreModule.Content.RootDirectory + '/' + _levelsFileName);
        if (_levelsCount == 0) throw new Exception("No levels found in the specified file.");
        // Load first level
        LoadLevel(0);
        Reset();
    }

    public override void Update(GameTime gameTime)
    {
        // Cheats
        if (Controls.CheatKey())
        {
            Cheated = true;
            // Reload current level
            if (Controls.Keyboard.IsKeyDown(Keys.M))
            {
                LoadLevel(_currentLevel);
                Reset();
                return;
            }
            // Toggle infinite lives
            if (Controls.Keyboard.IsKeyDown(Keys.L))
            {
                _infiniteLives = !_infiniteLives;
                return;
            }
            // Control gravitation
            if (Controls.Keyboard.IsKeyDown(Keys.G))
            {
                CoreModule.Gravitation = CoreModule.Gravitation == Vector2.Zero
                    ? Vector2.UnitY * CoreModule.DefaultGravitation
                    : Vector2.Zero;
                return;
            }
            // Load a different level
            var number = 0;
            // Get binary input from number keys
            Keys[] keys = [Keys.D1, Keys.D2, Keys.D3, Keys.D4, Keys.D5];
            for (var i = 0; i < keys.Length; i++)
            {
                if (Controls.Keyboard.IsKeyDown(keys[i])) number += (int)Math.Pow(2, i);
            }

            if (number < _levelsCount)
            {
                LoadLevel(number);
                Reset();
                return;
            }
        }

        // If won, animate air bar transfer to score
        // Done before update to prevent further updates 
        if (WinCondition)
        {
            // Gives 10 points a units
            Score += (int)(_airCapacity * 0.01 > _remainingAir ? _remainingAir * 10 : _airCapacity * 0.1);
            _remainingAir -= _airCapacity * 0.01F;
            _scoreLabel.Text = _totalScore.ToString("D6");
            if (_remainingAir > 0) return;
            // Load next level or set scene to Win scene
            if (++_currentLevel < _levelsCount)
            {
                LoadLevel(_currentLevel);
                Reset();
            }
            else
            {
                if (Manager is not Game1 manager) return;
                var data = new Dictionary<string, object>()
                {
                    { "score", _totalScore },
                    { "lives", _lives },
                    { "highScore", false},
                    { "levelsFileName", _levelsFileName},
                    { "cheated", Cheated }
                };
                if (!Cheated && _totalScore > manager.HighScore)
                {
                    manager.HighScore = _totalScore;
                    data["highScore"] = true;
                }
                Manager.LoadScene("win", data);
            }
            return;
        }

        // Entity updates
        base.Update(gameTime);
        if (!UpdateSceneObjects) return;

        _remainingAir -= _airDecayValue;
        if (_remainingAir <= 0)
        {
            _lossCondition = true;
            _remainingAir = 0;
        }
        // Check if the player has lost
        if (!_lossCondition) return;
        if (!_infiniteLives) _lives -= 1;
        if (_lives <= 0)
        {
            // Show the loss scene
            if (Manager is not Game1 manager) return;
            var data = new Dictionary<string, object>()
            {
                { "score", _totalScore },
                { "lives", _lives },
                { "highScore", false},
                { "levelsFileName", _levelsFileName},
                { "cheated", Cheated}
            };
            // If the player has cheated, don't set high score
            if (!Cheated && _totalScore > manager.HighScore)
            {
                manager.HighScore = _totalScore;
                data["highScore"] = true;
            }
            Manager.LoadScene("loss", data);
        }
        else
        {
            Reset();
        }
    }

    public override void Draw(GameTime gameTime, SpriteBatch batch, Vector2 offset, Vector2 uiScale)
    {
        // Flicker the air bar if the player is low on air
        _airBar.Width = _remainingAir / _airCapacity * _airBarWidth;
        if (_airBar.Width <= _airBarWidth * 0.2 && gameTime.TotalGameTime.Milliseconds <= 500)
        {
            _airBar.Color = Color.Red;
        }
        else
        {
            _airBar.Color = Color.White;
        }
        // Lives are drawn manually for their dynamic nature
        var tileSize = SceneBounds.Size.X / CoreModule.WidthTileCount * uiScale.X;
        for (var i = 0; i < _lives - 1; i++)
        {
            _livesIcon.DrawExtended(gameTime, batch, offset + new Vector2(tileSize * 2 * i, 0), uiScale, _infiniteLives ? Color.Gold : null);
        }
        base.Draw(gameTime, batch, offset, uiScale);
    }

    /// <summary>
    /// Resets current level
    /// </summary>
    public override void Reset()
    {
        base.Reset();
        _remainingAir = _airCapacity;
        RemainingKeys = _totalKeys;
        _scoreLabel.Text = _totalScore.ToString("D6");
        _shadowRectangle.Visible = false;
        UpdateSceneObjects = true;
        _lossCondition = false;
        WinCondition = false;
    }

    /// <summary>
    /// Load a level with passed index
    /// </summary>
    private void LoadLevel(int levelIndex)
    {
        // clear current level objects
        SimulatedSceneObjects.Clear();

        // Load the level from the file
        var loadedLevel = LevelLoader.LoadLevel(levelIndex, this, Manager.TextureAtlas, CoreModule.Content.RootDirectory + "/" + _levelsFileName);
        if (loadedLevel.Number == -1) return;
        // Set the level properties
        _airCapacity = loadedLevel.AirSupply;
        RemainingKeys = loadedLevel.KeyCount;
        _totalKeys = loadedLevel.KeyCount;
        _levelName.Text = loadedLevel.Name;
        _airDecayValue = loadedLevel.AirDecaySpeed;
        BackgroundColor = loadedLevel.BackgroundColor;
        foreach (var obj in loadedLevel.Objects)
        {
            AddObject(obj);
        }
        _currentLevel = levelIndex;
    }
}
