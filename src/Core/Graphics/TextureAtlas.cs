using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;
using System.Xml;
using System;
using System.Linq;
using System.Reflection.Metadata.Ecma335;

namespace Core.Graphics;
public class TextureAtlas(Texture2D texture)
{
    private Dictionary<string, TextureRegion> _regions = new Dictionary<string, TextureRegion>();

    /// <summary>
    /// Set new region for this name if it isn't set already
    /// </summary>
    public void SetRegion(string regionName, Vector2 position, Vector2 size)
    {
        if (_regions.ContainsKey(regionName)) return;
        _regions.Add(regionName, new TextureRegion(texture, position, size));
    }

    /// <summary>
    /// Unset region for this name
    /// </summary>
    public void UnsetRegion(string regionName)
    {
        _regions.Remove(regionName);
    }

    /// <summary>
    /// Get a specific region by name
    /// </summary>
    public TextureRegion GetRegion(string regionName)
    {
        return _regions[regionName];
    }

    /// <summary>
    /// Get all regions
    /// </summary>
    public TextureRegion[] GetRegions()
    {
        return _regions.Values.ToArray();
    }

    /// <summary>
    /// Unset all named regions
    /// </summary>
    public void Clear()
    {
        _regions = new Dictionary<string, TextureRegion>();
    }


    /// <summary>
    /// Get regions from an XML file
    /// </summary>
    /// <param name="content">Used ContentManager</param>
    /// <param name="fileName">File path relative to content root</param>
    public static TextureAtlas FromFile(ContentManager content, string fileName)
    {
        var filePath = content.RootDirectory + '/' + fileName;
        using var stream = TitleContainer.OpenStream(filePath);
        using var reader = XmlReader.Create(stream);
        var document = XDocument.Load(reader);
        var root = document.Root;

        // Get Texture2D for atlas
        if (root == null)
            throw new InvalidOperationException("Invalid XML format for TextureAtlas.");
        var atlas = new TextureAtlas(content.Load<Texture2D>(root.Attribute("name")?.Value ?? "unknown_name"));

        // Populate atlas with regions
        var regions = root.Elements("TextureRegion");
        foreach (var region in regions)
        {
            if (string.IsNullOrEmpty(region.Attribute("name")?.Value)) continue;
            var origin = new Vector2(
                Convert.ToInt16(region.Attribute("x")?.Value ?? "0"),
                Convert.ToInt16(region.Attribute("y")?.Value ?? "0"));
            var size = new Vector2(
                Convert.ToInt16(region.Attribute("w")?.Value ?? "0"),
                Convert.ToInt16(region.Attribute("h")?.Value ?? "0"));
            atlas.SetRegion(region.Attribute("name")?.Value ?? "unknown_name", origin, size);
        }

        return atlas;
    }
}
