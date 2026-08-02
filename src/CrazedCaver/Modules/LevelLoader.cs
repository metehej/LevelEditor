using Core.Graphics;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using System.Xml;
using Core;
using Core.Simulation;
using CrazedCaver.Modules.GameObjects;

namespace CrazedCaver.Modules;

/// <summary>
/// A structure containing loaded data
/// </summary>
public readonly struct Level(string name, float airSupply, float airDecaySpeed, int number, int keyCount, Color BackgroundColor, ISceneObject[] objects)
{
    public string Name { get; } = name;
    public float AirSupply { get; } = airSupply;
    public float AirDecaySpeed { get; } = airDecaySpeed;
    public int Number { get; } = number;
    public Color BackgroundColor { get; } = BackgroundColor;
    public int KeyCount { get; } = keyCount;
    public ISceneObject[] Objects { get; } = objects;
}

public static class LevelLoader 
{
    /// <summary>
    /// Read the specified document and return the root element.
    /// </summary>
    /// <exception cref="Exception">No root was found in the specified file</exception>
    private static XElement LoadRoot(string filePath)
    {
        using Stream stream = TitleContainer.OpenStream(filePath);
        using XmlReader reader = XmlReader.Create(stream);
        XDocument document = XDocument.Load(reader);
        return document.Root ?? throw new Exception("Root element not found");
    }

    /// <summary>
    /// Get a list of names of levels found in the file, ordered by level number
    /// </summary>
    public static string[] ListLevels(string filePath)
    {
        var root = LoadRoot(filePath);
        return root?.Elements("Level")
            .OrderBy(level => level.Attribute("number"))
            .Select(level => level.Attribute("name")?.Value)
            .Where(name => !string.IsNullOrEmpty(name))
            .ToArray() ?? [];
    }

    /// <summary>
    /// Get the count of levels found in the file
    /// </summary>
    public static int CountLevels(string filePath)
    {
        var root = LoadRoot(filePath);
        return root?.Elements("Level").Count() ?? 0;
    }

    /// <summary>
    /// Fetch the level element by name
    /// </summary>
    private static XElement FetchLevel(string name, string path)
    {
        var root = LoadRoot(path);
        return root.Elements("Level").FirstOrDefault(l => l.Attribute("name")?.Value == name, null)!;
    }

    /// <summary>
    /// Fetch the level element by number
    /// </summary>
    private static XElement FetchLevel(int number, string path)
    {
        var levelRoot = LoadRoot(path);
        if (levelRoot is null) throw new Exception("Level not found.");
        return levelRoot.Elements("Level").FirstOrDefault(l => Convert.ToInt32(l.Attribute("number")?.Value) == number, null)!;
    }

    /// <summary>
    /// Load a level by its number 
    /// </summary>
    public static Level LoadLevel(int levelNumber, Scene scene, TextureAtlas textureAtlas, string filePath)
    {
        // Fetch the level
        var levelRoot = FetchLevel(levelNumber, filePath);
        if (levelRoot is null) throw new Exception("Level not found.");
        // Get the level description
        return LoadLevelData(levelRoot, scene, textureAtlas);
    }

    /// <summary>
    /// Load a level by its name 
    /// </summary>
    public static Level LoadLevel(string levelName, Scene scene, TextureAtlas textureAtlas, string filePath)
    {
        // Fetch the level
        var levelRoot = FetchLevel(levelName, filePath);
        // Get the level description
        return LoadLevelData(levelRoot, scene, textureAtlas);
    }

    /// <summary>
    /// Load a level by its number
    /// </summary>
    private static Level LoadLevelData(XElement levelRoot, Scene scene, TextureAtlas textureAtlas)
    {
        try
        {
            // Load level info
            var name = levelRoot.Attribute("name")?.Value ?? "";
            var airSupply = (float?)levelRoot.Attribute("airSupply") ?? 0F;
            var airDecaySpeed = (float?)levelRoot.Attribute("airDecaySpeed") ?? 0F;
            var number = (int?)levelRoot.Attribute("number") ?? -1;
            var keyCount = 0;
            var hex = levelRoot.Attribute("backgroundColor")?.Value ?? "#778899";
            int r, g, b;
            try
            {
                r = int.Parse(hex.Substring(1, 2), System.Globalization.NumberStyles.HexNumber);
                g = int.Parse(hex.Substring(3, 2), System.Globalization.NumberStyles.HexNumber);
                b = int.Parse(hex.Substring(5, 2), System.Globalization.NumberStyles.HexNumber);
            }
            catch
            {
                // Light Slate Gray fallback
                r = 47;
                g = 53;
                b = 60;
            }
            var backgroundColor = new Color(r, g, b);

            // Load sprites
            Dictionary<string, Sprite> sprites = [];
            foreach (var spriteDescription in levelRoot.Element("Sprites")?.Elements("Sprite") ?? [])
            {
                if (spriteDescription.Attribute("animated") is not null && (bool)spriteDescription.Attribute("animated"))
                {
                    var frames = new TextureRegion[spriteDescription.Elements("Frame").Count()];
                    foreach (var frame in spriteDescription.Elements("Frame"))
                    {
                        var frameRegionName = frame.Attribute("regionName")?.Value;
                        var frameNumber = (int?)frame.Attribute("frameNumber") ?? 0;
                        if (string.IsNullOrEmpty(frameRegionName)) continue;
                        frames[frameNumber] = textureAtlas.GetRegion(frameRegionName);
                    }
                    if (frames.Length == 0) continue;
                    var animatedSprite = new AnimatedSprite(frames)
                    {
                        AnimationLength = (float?)spriteDescription.Attribute("animationLength") ?? 1,
                        Loop = (bool?)spriteDescription.Attribute("loop") ?? true
                    };
                    sprites[spriteDescription.Attribute("spriteName")?.Value ?? ""] = animatedSprite;
                    continue;
                }
                var regionName = spriteDescription.Attribute("regionName")?.Value;
                if (string.IsNullOrEmpty(regionName)) continue;
                Sprite sprite = new Sprite(textureAtlas.GetRegion(regionName));
                sprites[spriteDescription.Attribute("spriteName")?.Value ?? ""] = sprite;
            }
            // Load objects
            var objectRoot = levelRoot!.Element("Objects");
            List<ISceneObject> objectList =
            [
                LoadCharacter(objectRoot, sprites, scene),
                LoadDoor(objectRoot, sprites, scene)
            ];
            
            objectList.AddRange(LoadKeys(ref keyCount, objectRoot, sprites, scene));
            objectList.AddRange(LoadStaticEnemies(objectRoot, sprites, scene));
            objectList.AddRange(LoadDynamicEnemies(objectRoot, sprites, scene));
            objectList.AddRange(LoadPlatforms(objectRoot, sprites, scene));
            objectList.AddRange(LoadBelts(objectRoot, sprites, scene));
            objectList.AddRange(LoadWalls(objectRoot, sprites, scene));
            objectList.AddRange(LoadLabels(objectRoot, sprites, scene));

            var objects = objectList.ToArray();
            return new Level(name, airSupply, airDecaySpeed, number, keyCount, backgroundColor, objects);
        }
        catch (NullReferenceException)
        {
            // Return an invalid level if the level data is not found or is incomplete
            return new Level("", 0, 0, -1, 0, Color.LightSlateGray, []);
        }
    }

    #region Loaders
    /// <summary>
    /// Load the character
    /// </summary>
    private static Character LoadCharacter(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        var description = objectRoot.Element("Character");
        // Get character's sprite
        var objectSprite = sprites[description!.Attribute("spriteName")!.Value].GetCopy();
        // Get character's starting position represented by the top-left tile
        var startingPosition = scene.GetTilePosition(
            new Point((int)description.Attribute("startTileX"), (int)description.Attribute("startTileY")));
        var playerBinding = Binding.FromSprite(objectSprite,
            objectSprite.GetOrigin(OriginLocation.MiddleCentre)).Move(startingPosition);
        // Create the character and move its origin to the bottom centre
        var character = new Character(objectSprite, playerBinding)
        {
            SpriteOrigin = OriginLocation.BottomCentre
        };
        character.BindingScale(new Vector2(0.6F, 0.95F));
        return character;
    }

    /// <summary>
    /// Load the exit door for the level
    /// </summary>
    private static Door LoadDoor(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        var description = objectRoot.Element("Door");
        // Load door sprites for both states
        var objectSprite = sprites[description!.Attribute("spriteNameClosed")!.Value];
        var spriteOpen = sprites[description.Attribute("spriteNameOpen")!.Value];
        // Load door position (top-left tile)
        var position = scene.GetTilePosition(
            new Point((int)description.Attribute("startTileX"), (int)description.Attribute("startTileY")));
        var binding = Binding.FromSprite(objectSprite, objectSprite.GetOrigin(OriginLocation.MiddleCentre));
        binding.MoveTo(position);
        return new Door(objectSprite, spriteOpen.Region, binding);
    }

    /// <summary>
    /// Load all collectibles/keys in the room
    /// </summary>
    private static List<Key> LoadKeys(ref int keyCount, XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        var description = objectRoot.Element("Keys");
        // Load key sprites and binding
        var objectSprite = sprites[description!.Attribute("spriteName")!.Value];
        var binding = Binding.FromSprite(objectSprite, objectSprite.GetOrigin(OriginLocation.MiddleCentre));
        // Load key value
        var value = (int)description.Attribute("value");
        // Create a key for each position
        List<Key> keys = [];
        foreach (var key in description.Elements("Position"))
        {
            var keyPosition = scene.GetTilePosition(new Point((int)key.Attribute("startTileX"),
                (int)key.Attribute("startTileY")));
            keys.Add(new Key(objectSprite, binding.GetCopy().MoveTo(keyPosition), value));
            keyCount++;
        }
        return keys;
    }

    /// <summary>
    /// Load all static enemies
    /// </summary>
    private static List<StaticEnemy> LoadStaticEnemies(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        List<StaticEnemy> objectList = [];
        foreach (var staticEnemy in objectRoot.Elements("StaticEnemy"))
        {
            // Load enemy's information
            var objectSprite = sprites[staticEnemy.Attribute("spriteName")!.Value];
            var binding = Binding.FromSprite(objectSprite, objectSprite.GetOrigin(OriginLocation.MiddleCentre));
            var scale = new Vector2((float)staticEnemy.Attribute("bindingScaleX"),
                (float)staticEnemy.Attribute("bindingScaleY"));
            // Load orientation
            // Values are the same as SpriteOrigin enum names
            var origin = (staticEnemy.Attribute("origin")?.Value ?? "BottomCentre") switch
            {
                "MiddleRight" => OriginLocation.MiddleRight,
                "TopCentre" => OriginLocation.TopCentre,
                "MiddleLeft" => OriginLocation.MiddleLeft,
                "MiddleCentre" => OriginLocation.MiddleCentre,
                _ => OriginLocation.BottomCentre
            };
            // Load an enemy at all specified positions
            foreach (var enemyPosition in staticEnemy.Elements("Position"))
            {
                var position = scene.GetTilePosition(
                    new Point((int)enemyPosition.Attribute("startTileX"), (int)enemyPosition.Attribute("startTileY")));
                var enemy = new StaticEnemy(objectSprite, binding.MoveTo(position))
                {
                    SpriteOrigin = origin
                };
                enemy.BindingScale(scale);
                objectList.Add(enemy);
            }
        }
        return objectList;
    }

    /// <summary>
    /// Load all dynamic enemies
    /// </summary>
    public static List<DynamicEnemy> LoadDynamicEnemies(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        List<DynamicEnemy> objectList = [];
        foreach (var dynamicEnemy in objectRoot.Elements("DynamicEnemy"))
        {
            // Load enemy's information
            var objectSprite = sprites[dynamicEnemy.Attribute("spriteName")!.Value];
            var binding = Binding.FromSprite(objectSprite, objectSprite.GetOrigin(OriginLocation.MiddleCentre));
            var scale = new Vector2((float)dynamicEnemy.Attribute("bindingScaleX"),
                (float)dynamicEnemy.Attribute("bindingScaleY"));
            // Load origin
            // Values are the same as SpriteOrigin enum names
            var origin = (dynamicEnemy.Attribute("origin")?.Value ?? "BottomCentre") switch
            {
                "MiddleRight" => OriginLocation.MiddleRight,
                "TopCentre" => OriginLocation.TopCentre,
                "MiddleLeft" => OriginLocation.MiddleLeft,
                "MiddleCentre" => OriginLocation.MiddleCentre,
                _ => OriginLocation.BottomCentre
            };

            var loop = (bool)dynamicEnemy.Attribute("loop");
            var delay = (float)dynamicEnemy.Attribute("delay");
            var speedX = (float)dynamicEnemy.Attribute("speedX");
            var speedY = (float)dynamicEnemy.Attribute("speedY");
            foreach (var dynamicEnemyPosition in dynamicEnemy.Elements("Position"))
            {
                var position = scene.GetTilePosition(
                    new Point((int)dynamicEnemyPosition.Attribute("startTileX"), (int)dynamicEnemyPosition.Attribute("startTileY")));
                var pathStart = scene.GetTilePosition(
                    new Point((int)dynamicEnemyPosition.Attribute("pathStartX"), (int)dynamicEnemyPosition.Attribute("pathStartY")));
                var pathEnd = scene.GetTilePosition(
                    new Point((int)dynamicEnemyPosition.Attribute("pathEndX"), (int)dynamicEnemyPosition.Attribute("pathEndY")));

                var enemy = new DynamicEnemy(objectSprite, binding.MoveTo(position), pathStart, pathEnd, loop, delay)
                {
                    SpriteOrigin = origin,
                    Velocity = new Vector2(speedX, speedY),
                };
                enemy.BindingScale(scale);
                objectList.Add(enemy);
            }
        }
        return objectList;
    }

    /// <summary>
    /// Load all platforms in the level, including sinking ones
    /// </summary>
    public static List<Platform> LoadPlatforms(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        List<Platform> objectList = [];
        foreach (var platform in objectRoot.Elements("Platform"))
        {
            // Load platform's information
            var middleSprite = sprites[platform.Attribute("middleSpriteName")!.Value];
            var edgeSprite = sprites[platform.Attribute("edgeSpriteName")!.Value];
            var binding = Binding.FromSprite(middleSprite, middleSprite.GetOrigin(OriginLocation.MiddleCentre));
            foreach (var platformPosition in platform.Elements("Position"))
            {
                var position = scene.GetTilePosition(
                    new Point((int)platformPosition.Attribute("startTileX"), (int)platformPosition.Attribute("startTileY")));
                binding.MoveTo(position);
                var length = (int)platformPosition.Attribute("length");
                // type is true for sinking platforms, any other value means solid
                var type = platformPosition.Attribute("type")?.Value == "sinking";
                if (length == 1)
                {
                    // Only a solo middle platform
                    objectList.Add(type
                        ? new SinkingPlatform(middleSprite, binding)
                        : new Platform(middleSprite, binding));
                }
                else
                {
                    // A platform with edges and optional middle
                    objectList.Add(type
                        ? new SinkingPlatform(edgeSprite, binding)
                        : new Platform(edgeSprite, binding));
                    var tileMove = new Vector2(scene.TileSize, 0);
                    for (var i = 1; i < length - 1; i++)
                    {
                        objectList.Add(type
                            ? new SinkingPlatform(middleSprite, binding.Move(tileMove))
                            : new Platform(middleSprite, binding.Move(tileMove)));
                    }
                    var secondEdgeSprite = edgeSprite.GetCopy();
                    secondEdgeSprite.Effect = SpriteEffects.FlipHorizontally;
                    objectList.Add(type
                        ? new SinkingPlatform(secondEdgeSprite, binding.Move(tileMove))
                        : new Platform(secondEdgeSprite, binding.Move(tileMove)));
                }
            }
        }
        return objectList;
    }

    /// <summary>
    /// Load all belts
    /// </summary>
    public static List<Belt> LoadBelts(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        List<Belt> objectList = [];
        foreach (var belt in objectRoot.Elements("Belt"))
        {
            var objectSprite = sprites[belt.Attribute("spriteName")!.Value];
            var binding = Binding.FromSprite(objectSprite, objectSprite.GetOrigin(OriginLocation.MiddleCentre));
            var speed = new Vector2(
                (float)belt.Attribute("speedX"), 0);
            var tileMove = new Vector2(scene.TileSize, 0);
            foreach (var beltPosition in belt.Elements("Position"))
            {
                var position = scene.GetTilePosition(
                    new Point((int)beltPosition.Attribute("startTileX"), (int)beltPosition.Attribute("startTileY")));
                binding.MoveTo(position);
                var length = (int)beltPosition.Attribute("length");
                binding.MoveTo(position);
                for (var i = 0; i < length; i++)
                {
                    objectList.Add(new Belt(objectSprite, binding, speed));
                    binding.Move(tileMove);
                } 
            }
        }
        return objectList;
    }

    /// <summary>
    /// Load all walls
    /// </summary>
    public static List<ImpassableEntity> LoadWalls(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        List<ImpassableEntity> objectList = [];
        // Load walls
        foreach (var wall in objectRoot.Elements("Wall"))
        {
            var objectSprite = sprites[wall.Attribute("spriteName")!.Value];
            var binding = Binding.FromSprite(objectSprite, objectSprite.GetOrigin(OriginLocation.MiddleCentre));
            foreach (var wallPosition in wall.Elements("Position"))
            {
                var position = scene.GetTilePosition(
                    new Point((int)wallPosition.Attribute("startTileX"), (int)wallPosition.Attribute("startTileY")));
                var length = (int)wallPosition.Attribute("length");
                binding.MoveTo(position);
                var tileMove = wallPosition.Attribute("direction")?.Value == "vertical"
                    ? new Vector2(0, scene.TileSize) 
                    : new Vector2(scene.TileSize, 0);
                for (var i = 0; i < length; i++)
                {
                    objectList.Add(new ImpassableEntity(objectSprite, binding));
                    binding.Move(tileMove);
                }
            }
        }

        return objectList;
    }

    public static List<Label> LoadLabels(XElement objectRoot, Dictionary<string, Sprite> sprites, Scene scene)
    {
        List<Label> objectList = [];
        foreach (var label in objectRoot.Elements("Label"))
        {
            var position = scene.GetTilePosition(
                new Point((int)label.Attribute("tileX"), (int)label.Attribute("tileY")));
            var text = label.Attribute("text")?.Value ?? "";
            var height = (int)label.Attribute("height");
            var labelObject = new Label(text, CoreModule.Font)
            {
                Position = position,
                Origin = OriginLocation.TopLeft
            };
            labelObject.SetHeight(height * scene.TileSize - 2);
            objectList.Add(labelObject);
        }
        return objectList;
    }
    #endregion
}