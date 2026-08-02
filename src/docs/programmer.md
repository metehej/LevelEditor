# Programátorská dokumentace

## Rozcestník

Jelikož je projekt celkem komplexní, zde jsou odkazy na různé části dokumentace:

- [Zadání](#zadání)
- [Rozbor](#rozbor)
    - [Simulace prostředí](#simulace-prostředí)
    - [Rozbor prostředí na jednotlivé entity](#rozbor-prostředí-na-jednotlivé-entity)
    - [Simulace interakcí mezi entitami](#simulace-interakcí-mezi-entitami)
    - [Kreslení momentálního stavu prostředí na obrazovku](#kreslení-momentálního-stavu-prostředí-na-obrazovku)
    - [Uživatelské rozhraní](#uživatelské-rozhraní)
    - [Ovládání programu](#ovládání-programu)
    - [Práce se vstupními daty](#práce-se-vstupními-daty)
- [Algoritmy](#algoritmy)
    - [Detekce kolizí](#detekce-kolizí)
    - [Změna velikosti obrazovky](#změna-velikosti)
- [Koncepce](#koncepce)
- [Třídy](#třídy)
    - [Core](#core)
    - [CrazedCaver](#crazedcaver)
- [Alternativní řešení](#alternativní-řešení)
- [Vstupní data](#vstupní-data)
    - [Textury](#textures.png)
    - [Regiony](#textureRegions.xml)
    - [Levely](#levels.xml)
- [Ukázky prostředí](#ukázky)
- [Průběh práce](#průběh-práce)
- [Další rozšíření](#další-rozšíření)
- [Závěr](#závěr)
- [Odkazy](#odkazy)

## Zadání

Úkolem je vytvořit obdobu hry Manic Miner v jazyce C#. Pro tvorbu jsem zvolil framework MonoGame, který usnadňuje práci s IO. Zaměřil jsem se tedy na tvorbu samotného 2D engine.
Vytyčil jsem si pár základních potřeb takového engine. Patří mezi ně:
- Simulace prostředí
- Rozbor prostředí na jednotlivé entity
- Simulace interakcí mezi entitami
- Kreslení momentálního stavu prostředí na obrazovku
- Uživatelské rozhraní
- Ovládání programu
- Práce se vstupními daty (načítání textur a úrovní)

Kromě tohoto jsem ještě nakreslil jednotlivé textury použité ve hře.

## Rozbor

Jednotlivé části rozeberu podle finální podoby. Některé jsou detailněji rozebrané dále.

### Simulace prostředí

Herní engine má dva souřadnicové systémy a jeden z nich slouží pro simulaci entit. Tento je představen tzv. `Scene` objekty, které prostředí spravují. 
Veškeré entity a další objekty s rozhraním `ISceneObject` by se měly nacházet na ohraničeném území `SceneBounds`, ale toto není nijak vynuceno kódem. 
Samotná hra to omezuje pouze pro figurku hráče, ale ostatní entity a UI objekty jsou také umístěny do tohoto prostřední.

Tato plocha je rozdělena na pole. Pro účely hry bylo použito stejných rozměrů jako v původní hře, tedy 32 Horizontálních a 24 vertikálních polí. 
V duchu původní hry byl i zachován poměr stran 4:3.

### Rozbor prostředí na jednotlivé entity

Scéna obsahuje Entity. Každá entita má rozhraní `ISceneObject`, které obsahuje vlastnosti a metody pro vykreslení a obnovování. 
Některé entity místo toho využívají rozhraní `ISceneObjectSimulated`. To je rozšířené o vlastnost `Binding` a slouží pro fyzickou simulaci. 
Entity mohou také mít rozhraní `IResettable`, které obsahuje metoru pro navrácení do výchozího stavu.

Entity jsou odvozené od třídy `Entity`. Ta má rozhraní pro simulaci i resetování a slouží jako základní stavební blok simulovaného světa.
Obsahuje základní metody pro pohyb, škálování a kreslení. Má také proměnnou třídy `Sprite` pro vykreslování.

Pro entity jsou definovaná ještě rozhraní `ISolid` s metodou pro testování "průchodnosti" entity při kolizi a `IInteractable` pro entity, co mají nadefinované události při dotyku s jinými entitami.

`Core` knihovna definuje ještě odvozenou třídu `MovableEntity` pro entity, které se hýbají podle vnitřní rychlosti a polohy, a `ImpassableEntity` pro entity, které jsou brány za pevné při každé kolizi.

Různé herní entity používají různá rozhraní. Například:
- Character je odvozen od `Entity`, jelikož vyžaduje jak vykreslování, tak fyzickou simulaci a resetování
- Wall je instancí `ImpassableEntity`, jelikož není prostupná ze žádné strany
- Platform je Entitou a implementuje `ISolid`, pro které má vlastní pravidla
- Key je Entitou a implementuje `IInteractable`, kdy při interakci s hráčem provede kroky pro označení za sebraný

### Simulace interakcí mezi entitami

Knihovna Core interakce předdefinované nemá a entity samy o sobě navzájem nereagují. Její třída `Scene` ale nabízí metody pro získání přístupu k jejím entitám. 
Díky tomuto lze iterovat přes dané entity a provádět akce na základě jejich polohy a nastavení.

Příkladem tohoto je `Character`. Instance této třídy si najde blízké entity a podle toho, jestli jsou `ISolid` nebo `IInteractable` mění svou polohu, rychlost, nastavuje různé stavy hry atp.

Pro fyzickou simulaci je třeba vědět, kdy a z jaké strany spolu dvě entity kolidují. Obě proto mají svůj `Binding`, který je reprezentuje fyzicky. 
Třída Binding implementuje algoritmy pro hledání kolizí, získávání průniku dvou kolizních čtverců a hledání vzdálenosti.

### Kreslení momentálního stavu prostředí na obrazovku

Z požadavku mít možnost okénko škálovat a dávat na celou obrazovku vznikla potřeba překladu pozic vnitřního prostředí na to vnější. 
Samotné jádro knihovny `CoreModule` (odvozené od třídy `Game` frameworku MonoGame) tedy obsahuje několik metod pro převod těchto souřadnic. Toto je do větší hloubky vysvětleno dále. 
Souřadnice se překládají při kreslení. Slouží k tomu údaje `Offset` ukládající odsazení simulované plochy a `UIScale` obsahující konstantu pro přenásobení souřadnic.

Framework MonoGame při vykreslování umožňuje použít hloubku v rozmezí 0 - 1 (float). `CoreModule` má proto navíc i statickou metodu pro získání hloubky podle typu vykreslovaného objektu (Background, UI, Character,...).

### Uživatelské rozhraní

Při vzhledu rozhraní bylo dbáno na jednoduchost, proto se hlavní scény skládají převážně z textu. Prostředí herní scény je inspirováno rozhraním hry Manic Miner s drobnými změnami pro zlepšení čitelnosti.
Prostředí je vykreslováno objekty, které implementují `ISceneObject` rozhranní. Jejich vykreslování je tedy řízeno scénou. Jedná se o třídy `Image`, `Label` a `Rectangle`.

### Ovládání programu

Při návrhu ovládání jsem zůstal u původních zkratek, ale některé jsem pozměnil tak, aby reflektovaly současnou herní scénu. Horní řada písmen stále představuje pohyb do stran, ale klávesy 
**W|A|S|D** byly předefinovány na samotný pohyb. Tohle navíc umožnilo využít klávesu **Q** pro pozastavení.

Další zřejmou inspirací je binární kód na číslech pro změnu levelu. Funguje na stejném principu, jako v původní hře.

Funkce myši jsem se rozhodl neimplementovat.

### Práce se vstupními daty

Veškeré textury jsou v podobě regionů uložené na jednom souboru .png pro rychlejší práci při kreslení. Popis těchto regionů je v .xml souboru. Tento je načítán přímo třídou `TextureAtlas`.

Levely mají vlastní soubor .xml, který přesně popisuje jejich obsah a nastavení. Pro načítání slouží statická třída `LevelLoader`.

## Algoritmy

V této sekci popíšu hlavní algoritmy. V celém projektu by se jich našlo více (např. vyhledávání blízkých entit), ale ty jsou většínou zastoupeny LINQ dotazem. 

### Detekce kolizí

Veškerou detekci kolizí počítá třída `Binding`. Samotná detekce kolize není tolik složitá - metoda `Intersects()` v případě kruhů zjistí vzdálenost jejich středů s pomocí pythagorovy vvěty a v případě obdélníků porovná jejich vrcholy.

Zajímavější je detekce strany, ze které kolize nastala. Ta je ve finální části prováděna v několika krocích. Na vstupu jsou dva `BindingRectangle` obdélníky, které představují stav před a po pohybu druhého objektu. 

`BindingRectangle` je instance obdélníku, pro kterou hledáme stranu. `velocity` je rychlost druhého objektu a `oldBinding` a `secondBinding` představují starý a nový stav druhého objektu respektive.

### Výpočet důležitých údajů
```c#
 // Find which sides were breached
 var axisX = velocity.X > 0 ? BindingRectangle.TopLeft.X : BindingRectangle.BottomRight.X;
 var axisY = velocity.Y >= 0 ? BindingRectangle.TopLeft.Y : BindingRectangle.BottomRight.Y;
 // Choose testing vortex
 var vertexY = velocity.Y < 0 ? oldBinding.TopLeft.Y : oldBinding.BottomRight.Y;
 var vertexX = velocity.X > 0 ? oldBinding.BottomRight.X : oldBinding.TopLeft.X;
 // Calculate the distance from the vertex to the axes
 var vertexDistanceX = axisX - vertexX;
 var vertexDistanceY = axisY - vertexY;
```

"testing vortex" je vrchol `oldBinding`, který je nejblíže testované oblasti. Snažíme se vybírat ten, který je "co nejdál" za testovanými stranami nebo co nejblíže k nim. 

### Triviální případy
```c#
// Test trivial cases
if (velocity.X == 0)
{
    // If the velocity is vertical, the collision is from the top or the bottom
    return velocity.Y > 0 ? CollisionSide.Top : CollisionSide.Bottom;
}
if (velocity.Y == 0)
{
    // If the velocity is horizontal, the collision is from a side
    return velocity.X > 0 ? CollisionSide.Left : CollisionSide.Right;
}
if (Math.Sign(vertexDistanceX * velocity.X) == Math.Sign(vertexDistanceY * velocity.Y)
    && vertexDistanceX * vertexDistanceY != 0)
{
    // The vertex was already past the tested axes
    return CollisionSide.None;
}
```
Kdy první dva případy představují pohyb po jednotlivých osách a třetí nám říká, že ke kolizi již došlo před aktuálním pohybem. Ve třetím případě vracíme hodnotu, která dává najevo, že ke kolizi nedošlo nebo nelze určit její stranu.

### Případy, kdy již byla jedna strana proťata

```c#
// Check if the chosen vertex has crossed exactly one of the axes
if (vertexDistanceX * vertexDistanceY * (velocity.X) * (velocity.Y) < 0)
{
    if((axisX  - vertexX) * velocity.X < 0)
    {
        // Hit from the top or the bottom
        return velocity.Y > 0 ? CollisionSide.Top : CollisionSide.Bottom;
    }
    // Hit from a side
    return velocity.X > 0 ? CollisionSide.Left : CollisionSide.Right;
}
```

Pokud objekt ve starém stavu již překročil jednu ze zvolených stran, je jisté, že ke kolizi došlo na té druhé. Toto je ve schématu zobrazeno jako "First case".

To, zda je překročena pouze jedna strana, je získáno výpočtem znaménka vzdáleností bodu od jednotlivých stran a znaménka jednotlivých složek pohybu. 

### Ostatní případy

```c#
// Construct a vector from the vertex to the point where axes cross
var vertexAxisVector = new Vector2(vertexDistanceX, vertexDistanceY);

// Calculate the angle between velocity and the vector
var angle = GeometryFormulas.GetAngle(vertexAxisVector, velocity);
if (angle > 0)
{
    // Hit from the top or the bottom
    return velocity.Y > 0 ? CollisionSide.Top : CollisionSide.Bottom;
}
// Otherwise, the collision is from a side
return velocity.X > 0 ? CollisionSide.Left : CollisionSide.Right;
```

Pokud stará pozice neprotíná ani jednu ze stran, sestrojením vektoru ze zvoleného vrcholu do průsečíku zvolených stran a nalezením úhlu mezi ním a vektorem pohybu lze podle znaménka zjistit, která strana je proťata dříve a kde tedy došlo ke kolizi. Toto je "Second case".

[Zde](docs/images/Binding.jpg) je ilustrace zvolených vrcholů, hran a výpočtu ve dvou případech. Kód se nachází [zde](Core/Simulation/Bindings.cs).

Tento postup je celkem obtížné osvětlit v textu, ale, jak se lze při hraní přesvědčit, funguje přesně podle zadání.

Dřívější verze měly odlišné algoritmy. Používal se například algoritmus, který jednoduše porovnával výšku a šířku průniku, ale ten se ukázal nedostačující. Také se používal algoritmus, který jen hledal průsešíky os, ale měl problémy v krajních případech.

### Změna velikosti

Dalším důležitým výpočtem je konstanta uiScale, která zařizuje překlad poloh. Její výpočet má dvě části
```c#
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
```
Nejprve se nastaví velikost `canvas` a `Offset` tak, aby se dodržel poměr stran. Porovnání zajistí, že je alespoň jedna strana nejdelší, jaká může být.

```c#
// Set uiScale
var uiScale = (screenSize.X - Offset.X * 2) / SimulationCanvasWidth;
```
Poté se vypočítá samotná konstanta. Její velikost závisí na velikosti simulovaného prostoru, velikosti obrazovky a dříve vypočítaného offsetu.
Konstanta vpodstatě říká, jak velký je jeden "pixel" simulovaného prostředí na reálné obrazovce.

Kód je k dispozici v souboru [zde](Core/CoreModule.cs).

Program původně držel vnitní souřadnice stejné jako vnější. uiScale bylo tedy pouze pro momenty, kdy se aplikaci měnila velikost. Problém s tímto byla potřeba často násobit vnitřní vektory tímto číslem, jinak například hráč skákal výše, když bylo okno menší. 
Oddělením tyto problémy vymizely. Údaje se sice pronásobují každý draw call, operace násobení je ale dost rychlá na to, aby nebylo potřeba výsledek ukládat. 

## Koncepce

Program je již od začátku koncipován jako simulace v reálném čase. Program má jistou dualitu v podobně obecné části `Core` a herní části `CrazedCaver`.

## Třídy
### Core

Jedná se o knihovnu obsahující definice, které by mohly být znovupoužity u dalších projektů. Obsahuje třináct .cs souborů rozdělených do 4 kategorií.

1. Core
Tato část obsahuje dva nejdůležitější prvky.

`CoreModule.cs` je hlavní třída knihovny. Ukrývá v sobě statické proměnné a parametry pro nastavení celé simulace. 

`Scene.cs` je druhá nejdůležitější třída knihovny. Obstarává simulaci prvků, které do ní patří.

2. Simulation
Tato část ukládá simulované objekty. 

`Bindings.cs` je třída, která vypočítává kolize a polohy jednotlivých objektů simulace. Každý `ISceneObject` obsahuje svůj Binding.

`Entity.cs` má vlastní Binding, Sprite a metody pro kreslení a obnovování. Tvoří základ toho, s čím hráč interaguje.

`Formulas.cs` je malá statická třída pro výpočet průsečíků a úhlu mezi vektory. Slouží třídě Binding.

3. Input

Nejmenší část knihovny, zajišťuje správu vstupních zařízení

`KeyboardManager.cs` je správce klávesnice. Pamatuje si předchozí a současný stav, který se obnovuje každý Update(). Umožňuje nejen testování, zda je klávesa stisknutá, ale i testování změny stavu klávesy.

`MouseManager.cs` je správce myši, který ale pouze zjišťuje pozici kurzoru. Více nebylo k projektu potřeba.

4. Graphics

Grafická část knihovny.

`TextureRegion.cs` je jednoduchá třída, jejíž instance uchovávají informace o regionu mapy textur. 

`TextureAtlas.cs` obsahuje všechny regiony jedné instance MonoGame třídy Texture2D. Regiony navíc drží pod jménem a umožňuje jejich snadné vyhledávání.

`Sprite.cs` objekty mají svůj region, který vykreslují podle jejich vnitřního stavu nebo podle parametrů předaných funkci Draw(). `AnimatedSprite` třída je animovaná verze Sprite, která jednoduše prohazuje aktivní region.
Její aktualizace je volaná při kreslení, aby byl dodržen "L" princip koncepce SOLID. Pro příklad takového Sprite je ve hře naanimován klíč.

`Image.cs` je `ISceneObject` verze Sprite. Umožňuje Sprite obnovovat a vykreslovat ve scénách, aniž by byl součástí plnohodnotné entity.

`Label.cs` je `ISceneObject` textového charakteru. Kromě běžného kreslení textu umožňuje i jednoduché stínování pro zvýraznění.

`BackgroundRectangle.cs` je prázdný obdélník jedné barvy. Využívá se pro kreslení rozhraní. Je to také `ISceneObject`.

### CrazedCaver

Herní část projektů využívá jádra pro nadefinování hry. Nepřináší mnoho nových konceptů, spíš staví na konceptech `Core`.

1. Game1
Hlavní třída reprezentující hru frameworku MonoGame. Je to vstupní brána při inicializaci. Kromě toho čte některé společné vstupy pro ovládání programu.

2. Controls.cs

Tato třída sdružuje manažery vstupu a předdefinovává některé klávesy.

3. Modules/GameObjects

Zde se nachází třídy vycházející z `Entity.cs`

`Character.cs` obsahuje logiku postavy hráče. Obstarává jak vstup, tak zobrazení. Řídí pohyb hráče po mapě, při kolizích a při dotyku s objekty, které mají `IInteractable` rozhraní.

`Enemies.cs` definuje statické a dynamické nepřátele. Statičtí mají pernamentní pozici. Dynamičtí nepřátelé se pohybují po předdefinované trase a představují tak větší nebezpečí.

`Environment` definuje vše ostatní. Zde patří dveře a klíče, platformy a pásy. Pro dveře a klíče je definovaná logika při doteku, pro platformy a pásy logika pevnosti jen při kolizi shora a pohyb hráče.

4. Modules/GameScenes

Tyto .cs soubory definují vzhled a obsah jednotlivých scén. Celá hra se odehrává na čtyřech scénách

`TitleScene` je hlavní scéna. Odsud lze přejít pouze na herní scénu.

`GameScene` je samotná herní scéna. Ve spodní části obsahuje rozhraní s informacemi o skóre, vzduchu a životech. Nad ním je samotná hra, která se nahrává s pomocí `LevelLoader.cs`. 
Z herní scény lze přejít do TitleScene při stisku tlačítka ESCAPE, do LossScene při ztrátě všech životů a do WinScene při dokončení všech levelů.

`LossScene` oznamuje hráči, že prohrál. Obsahuje instanci `Image`. Z této scény lze přejít do TitleScene, nebo lze začít hru od začátku.

`WinScene` značí výhru. Kromě získaného skóre a počtu životů umožňuje hráči pokračovat ve hře, nebo ji ukončit.

5. LevelLoader.cs
Poslední část CrazedCaver slouží k nahrávání levelů. Třída obsahuje statické metody pro výčet levelů v souboru a pro získání dat levelu. Ta jsou ukládána do struktury `Level`. Mimo to zde leží ještě samostatné Loadery jednotlivých typů objektů.

LevelLoader přijímá soubory formátu XML. 

## Alternativní řešení

Původní hra pro kolize používala detekce na bázi pixelů. Toto řešení jsem nezvolil, jelikož by vyžadovalo jiný přístup k vykreslování.

Další z možností, kterou jsem původně zvažoval, bylo přesunutí UI objektů mimo simulovaný svět, ale časem jsem došel k závěru, že to není třeba. 

Jak jsem již dříve zmiňoval, původně jsem zabudoval jiné algoritmy pro hledání kolizí. Například se dříve nejprve hledaly možné kolize pomocí BindingCircle a až poté se blíže prověřovalo, jestli skutečně došlo ke kolizi na základě obélníku.
Toto řešení bylo sice několikrát rychlejší, ale testováním jsem přišel na to, že rozdíl je při počtu entit ve hře nepoznatelný.

Uvažoval jsem i nad volbou jiného frameworku, nicméně projekt jsem dopsal v MonoGame.

Poslední větší volbou byla volba reprezentace levelů. Volil jsem mezi JSON, XML a binárními soubory. Zvolil jsem XML, jelikož jsem jej použil již dříve a tvorba binárních souborů by vyžadovala napsat editor.

## Vstupní data

### textures.png
[Tento](CrazedCaver/Content/textures.png) PNG soubor obsahuje veškeré sprity hry. Rozlišení je značně velké oproti zabrané ploše, protože je soubor připravený pro dokreslení všech animovaných spritů. 

Původní hra sice pracovala s velice malými a nanejvýš dvoubarevnými texturami, zato jich ale vývojář vymyslel hodně. I proto je textures.png tak velký.

Pro přidání Sprite jej stačí umístit na volnou pozici. Jeho souřadnice je poté potřeba zapsat do následujícího souboru.

### textureRegions.xml

Tento XML soubor ukládá informace o tom, kde se nachází jaké Sprites na textures.png. S pomocí tohoto souboru sestaví TextureAtlas slovník textur.

Zápis v tomto souboru je jednoduchý. Za hlavičkou následuje kořen s názvem souboru textur. Název je bez přípony, protože je soubor nahráván přes MonoGame ContentManager, který má soubory předzpracované v binární podobě. 

Pod ním se pak nachází definice jednotlivých textur. Soubor může vypadat například takto:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Texture name="textures">
	<TextureRegion name="belt" x="0" y="0" w="32" h="32"/>
</Texture>
```
- `name` - název regionu
- `x|y` - souřadnice levého horního rohu
- `w|h` - výška a šířka regionu

### levels.xml

Tento soubor popisuje složení každého levelu. Jeho struktura je shodná se stylem čtení v souboru `LevelLoader.cs`.

```xml
<?xml version="1.0" encoding="utf-8" ?>
<Root>
    <Level name="string" number="int" airSupply="float" airDecaySpeed ="float">
        <Sprites>
            ...
        </Sprites>
        <Objects>
            .
            .
            .
        </Objects>
    </Level>
</Root>
```
Kořenem souboru je Element `Root`. Jeho součástí může být libovolný počet levelů, ale levely by měly mít vlastní čísla a řada těchto čísel by měla být kompletní (tedy nepřeskakovat hodnoty).

Pod kořenem je již samostatný `Level` s názvem, pořadovým číslem, množstvím vzduchu a rychlostí úbytku vzduchu.

Název se zobrazuje v liště hradí scény. Číslo slouží k řazení a vyhledávání levelů. 

Každá jednotka `airSupply` představuje 10 potenciálních bodů skóre pro hráče. Správnou kombinací tohoto a `airDecaySpeed` parametru lze pro level zvolit jeho délku a odpovídající ohodnocení.

*Tip:* `airDecaySpeed` hodnota 0.03F zaručí, že počet `airSupply` představuje délku levelu v sekundách.

```xml
<Sprites>
    <Sprite regionName="string" spriteName="string"/>
    <Sprite spriteName="string" animated="bool" loop="bool" animationLength="float">
        <Frame frameNumber="int" regionName="string"/>
    </Sprite>
</Sprites>
```
`Sprites` definuje vešekré regiony, které level používá, a dává jim jména. 
Pokud `Sprite` nemá definované `animated` jako hodnotu true, tak se jedná o obyčejný sprite. Ten má pouze jméno a region.
Pokud má `animated` jako true, pak je to Animovaný sprite. Ten má navíc definované, jestli loopuje ("jde tam a zpátky") a jak dlouho animace trvá.
Krom toho má uvnitř definované jednotlivé regiony animace tak, jak jdou za sebou. Mají tedy číslo `frameNumber` a region.

```xml
<Objects>
    .
    .
    .
</Objects>
```
Objekty jsou veškeré entity scény. Ty si rozebereme každou zvlášť, nejprve ale společné údaje
- `startTile.` : int - index Scene Tile, na kterém se entita nachází na začátku. Jedná se o levý horní roh entity
- `spriteName` : string - dříve definovaný název Sprite. Některá políčka mají speciální názvy pro různé stavy entity, třeba otevřené a zavřené dveře
- `pathStart.|pathEnd.` : int - označují Tiles, které jsou brané jako začátky a konce cesty dynamických nepřátel
- `origin` : string - speciální proměnná, která ukládá, kam se má po inicializaci entity přemístit její počátek. Možnosti jsou zvýrazněné přímo v kódu

```xml
<Character spriteName="string" startTileX ="int" startTileY ="int"/>
```
`Character` definuje startovací pozici hráče.

```xml
<Door spriteNameClosed="string" spriteNameOpen="string" startTileX="int" startTileY="int"/>
```
`Door` Definuje pozici dveří a jejich Sprite pro oba stavy

```xml
<Keys spriteName="string" value="int">
    <Position startTileX="int" startTileY="int"/>
</Keys>
```
`Keys` definuje Sprite klíčů a jejich hodnotu pro skóre. Každá `Position` říká pozici jednoho klíče. Jejich počet je spočítán při načítání.


Předchozí definice by měly existovat pouze jednou v Levelu. Následující takové omezení nemají. Stejně tak jejich vnitřních členů může být více.


```xml
<StaticEnemy spriteName="string" bindingScaleX="float" bindingScaleY="float" origin="see LevelLoader for details">
    <Position startTileX="int" startTileY="int"/>
</StaticEnemy>
```
`StaticEnemy` definuje Sprite, origin a bindingScale jednoho druhu statického nepřítele. `bindingScale.` je použito pro změnění velikosti hitboxu nepřítele. Škáluje se s ohledem na origin.
Každá `Position` je jedna instance tohoto typu.

```xml
<DynamicEnemy spriteName="string" bindingScaleX="float" bindingScaleY="float"
                loop="bool" delay="float" speedX="float" speedY="float">
    <Position startTileX="int" startTileY="int" pathStartX="int" pathStartY="int" pathEndX="int" pathEndY="int"/>
</DynamicEnemy>
```
`DynamicEnemy` definuje pohyblivé nepřítele. Každá taková definice má společný Sprite, velikost hitboxu. Dále také 

`loop`, což značí, jestli se nepřítel jen otočí, nebo rovnou teleportuje na první pozici

`delay`, počet sekund, než se znovu rozhýbe

`speed.` rychlost a směr pohybu v jednotlivých osách

Každá `Position` definuje nejen počáteční pozici, ale i pozice prvního a posledního bodu trasy. Body nemusí být uspořádané s ohledem na pozici, ale pokud se nepřítel teleportuje, teleportuje se vždy k první pozici.

```xml
<Platform edgeSpriteName="string" middleSpriteName="string">
    <Position startTileX="int" startTileY="int" length="int" type="solid|sinking"/>
</Platform>
```
`Platform` definuje platformy. Ty mají krajní a vnitřní segmenty. Každá `Position` definuje jednu platformu, která začíná na pozici X,Y a má délku `length` v horizontálním směru.
`type` definuje v současné době platformy buď pevné, nebo propadající se. Platí, že každá hodnota jiná než "sinking" je automaticky "solid".

```xml
<Belt spriteName="string" speedX="float">
    <Position startTileX="int" startTileY="int" length="int"/>
</Belt>
```
`Belt` je obdobné definici `Platform`, ale místo dvou Sprite definuje jeden Sprite a jednu rychlost. Rychlost je v ose x.


```xml
<Wall spriteName="string">
    <Position startTileX="int" startTileY="int" length="int" direction="vertical|horizontal"/>
</Wall>
```
`Wall` definuje stěny. Tvoří je jako instance `ImpassableEntity`, přičemž stačí definovat počáteční bod, délku a směr stěny.

Pro příklad definice je soubor [zde](CrazedCaver/Content/levels.xml).

## Ukázky
Odkazy na ukázky prostředí:
- [Title Scene](docs/images/TitleScene.png) - Ukázka hlavní stránky.
- [First Level](docs/images/FirstLevel.png) - Ukázka prvního levelu. Červené skóre značí použití Cheatů.
- [Loss Scene](docs/images/LossScene.png) - Ukázka prohry. Hráč nahrál nové nejvyšší skóre.
- [Win Scene](docs/images/WInScene.png) - Ukázka výhry. Hráč nejenže zahrál nové nejvyšší skóre, ale zvládl to s plným počtem životů.

## Průběh práce
Téma jsem si zvolil již během zkouškového, kvůli nemoci a přesunutí termínu jsem se ale k projektu dostal až týden před termínem. Abych maximalizoval produktivitu, rozplánoval jsem si práci na jednotlivé dny, ale i tak jsem musel pracovat celé dny. 

Během projektu jsem se seznámil s frameworkem MonoGame, ve kterém mimo jiné vzniklo několik her, které jsem sám hrál. Byla to dobrá zkušenost a něco úplně jiného, než mé předchozí projekty.

Také jsem si stáhl emulátor počítače ZX Spectrum, abych měl rychlý přístup ke hře. Bohužel jsem zjistil, že mi ta hra moc nejde.

## Další rozšíření

K dispozici je hned několik cest, kterými se dá vydat.

### Audio
Hra postrádá audio. Ačkoliv původní hra nemá nijak moc zvuků (dvě skladby a pár efektů), tak jsem měl původně v plánu s pomocí open-source studia složit krátké efekty. 

### Sjednocení metod
Finální verze ořezala třídám nevyužité funkce. Ty jsou sice dobré, ale kód je již tak dlouhý. Pokud by se knihovna měla dále rozvíjet, bylo by třeba metody sjednotit a nejlépe je přenést do rozhraní.

### Level loader, generátor, editor
Byť vypadá XML definice levelu celkem přímočaře, přepisovat úrovně do této podoby by při větším počtu úrovní byla úmorná práce. Mnohem jednodušší by bylo napsat generátor levelů a editor levelů. Editor by mohl být i in-game, jelikož `Scene` třída sama o sobě nemá určený účel. Nejlépe by se ještě dodefinovalo klikání myší a ve scéně by se objekty tahaly přímo na její TileMap.

### Více spritů
Původní hra je velice rozmanitá co se tvarů a barev týče. Bylo by pěkné naanimovat a překreslit více druhů nepřátel a klíčů do nové verze. 

### Podpora spritů založených na pohybu
Hráč má momentálně pouze jeden sprite, který se dívá buď nalevo nebo na pravo. Pomocí jednoduchých úprav kódu třídy `Character.cs` by se umožnilo Sprite měnit při chůzi a volit tak animace podle toho, jestli se character hýbe.

## Závěr
Jsou věci, které bych udělal jinak a ušetřil bych si nejen čas, ale i zdraví. I tak jsem ale rád za to vše, co jsem se o jazyku C# a o tvorbě plošinovek během tvorby projektu naučil. Navíc jsem měl možnost pohledět na fungování programů na starších počítačích.

## Odkazy
[MonoGame](https://monogame.net/) - Framework pro tvorbu her v C#
