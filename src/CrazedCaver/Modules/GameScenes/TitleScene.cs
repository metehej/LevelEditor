using System.Collections.Generic;
using Core;
using Core.Graphics;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;
using static System.Formats.Asn1.AsnWriter;

namespace CrazedCaver.Modules.GameScenes;

public class TitleScene(CoreModule manager) : Scene (manager)
{
    private readonly Color[] _titleScreenColors =
    [
        Color.White,
        Color.Yellow,
        Color.Indigo,
        Color.Coral,
        Color.Purple
    ];
    private int _colorIndex;        
    private int _updateCounter;
    private Label _title;
    public override void Initialize(Dictionary<string, object> kwargs = null)
    {
        // Title
        var position = GetTilePosition(new Point(TilemapWidth / 2, TilemapHeight / 2 - 2));
        var sceneObject = new Label("CrazedCaver", CoreModule.Font)
        {
            Origin = OriginLocation.BottomCentre,
            Position = position,
            Shaded = true
        };
        sceneObject.SetWidth(SceneBounds.Size.X * 0.9F);
        AddObject(sceneObject);
        _title = sceneObject;

        // High score stats
        position += new Vector2(0,TileSize * 3);
        sceneObject = new Label($"High score: {((Game1)Manager).HighScore:D6}", CoreModule.Font)
        {
            Color = Color.Yellow,
            Origin = OriginLocation.TopCentre,
            Position = position
        };
        sceneObject.SetHeight(TileSize * 0.8F);
        AddObject(sceneObject, true);
        position += new Vector2(0, TileSize * 2);

        //Keybind info
        sceneObject = new Label("Press ENTER to start", CoreModule.Font)
        {
            Origin = OriginLocation.TopCentre,
            Position = position,
            Shaded = true
        };
        sceneObject.SetHeight(TileSize * 0.8F);
        AddObject(sceneObject, true);

        position += new Vector2(0, sceneObject.Height + 15);
        sceneObject = new Label("Press ESC to exit", CoreModule.Font)
        {
            Origin = OriginLocation.TopCentre,
            Position = position,
            Color = Color.LightGray,
            Shaded = true
        };
        sceneObject.SetHeight(TileSize * 0.65F);
        AddObject(sceneObject, true);
    }
    public override void Update(GameTime gameTime)
    {
        // Check for input
        if (Controls.Keyboard.WasKeyPressed(Keys.Enter))
        {
            Manager.LoadScene("game", new Dictionary<string, object>
            {
                {"levelsFileName", ((Game1)Manager).LevelsFile}
            });
            return;
        }

        // Update Scene
        base.Update(gameTime);

        // Switch Title color after 45 updates
        if (++_updateCounter < 45) return;
        _updateCounter = 0;
        _colorIndex = (_colorIndex + 1) % _titleScreenColors.Length;
        _title.Color = _titleScreenColors[_colorIndex];
    }
}