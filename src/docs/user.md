# Uživatelská dokumentace

## Jak hru spustit

Pro spuštění hry stačí přejít do složky **CrazedCaver** a spustit příkaz `dotnet run`.

Hra se spustí ve vlastním okénku. Okénku lze měnit velikost a i jej zvětšit na celou obrazovku. Pro přechod do režimu celé obrazovky stiskni klávesu **F11**.

## Jak se hra ovládá

Byť hra zobrazuje pozici kurzoru žlutým čtverečkem, tohle je pouze pro jednodušší nalezení kurzoru uživatelem. Hra vstup z myši nijak nepoužívá.

Hra má předdefinované následující klávesové zkratky:

### Ovládání chodu programu

- **ENTER** - Spusť hru / Pokračuj
- **ESCAPE** - Vrať se do menu / Ukonči program
- **F1** - Změň barvu pozadí
- **F11** - Přejdi mezi okénkem a režimem celé obrazovky

### Ovládání hry

- **E|T|U|O|A|Left arrow** - Pohyb doleva
- **R|Y|I|P|D|Right arrow** - Pohyb doprava
- **W|Space|Up arrow** - Skok
- **S|Left Shift|DownArrow** - Zpomalení a zastavení pohybu
- **Q|?** - Pozastavení a opětovné spuštění hry *pozn.: otazník funguje v závislosti na zvolené klávesnici*

### Cheat kódy

- **Right Alt** - Potvrzovací tlačítko

Následující klávesy je potřeba již držet v moment, kdy je stisknuté potvrzovací tlačítko.

**Použití kterékoliv zkratky způsobí označení aktuální hry za neplatnou.**

- **1|2|3** - Tlačítka pro zadání kódu levelu, který se má načíst *pozn.: více v kapitole [Jaké úrovně hra obsahuje](#jaké-úrovně-hra-obsahuje).*
- **M** - Reset aktuální úrovně bez ztráty života a score
- **G** - Vypnutí a zapnutí gravitace
- **L** - Vypnutí a zapnutí neomezených životů

### Vývojářské nástroje

- **F3** - Spuštění módu pro debuggování

## Cíl hry

Hra je klasickou plošinovkou. Hráč ovládá postavu pohybující se v 2D prostoru. Úkolem je sesbírat veškeré klíče v aktuální úrovni a dostat se ke dveřím.

Po spuštění první úrovně je na obrazovce vidět hned několik věcí.

### Hrací plocha

Hrací plocha se nachází v horní části obrazovky. Zkládá se z mapy čtvercových polí. Na hrací ploše se nachází několik různých entit.

Nejdůležitější je hráč. Toho lze najít jednodušše, při stisku kterékoliv klávesy pro pohyb jej vykoná.

Dále se tu nachází nepřátelé. Ti jsou buď statičtí, nebo se pohybují po mapě na předdefinované trase. 

Na mapě jsou i klíče. Ty blikají a je třeba se jich dotknout figurkou hráče. Jakmile hráč posbírá všechny klíče, otevřou se dveře do další úrovně.

Kromě zdí z cihel se může hráč pohybovat i po platformách. Ty jsou buď pevné, nebo se pod hráčem propadají. Jakmile platforma zmizí, už se nevrátí.

### Vzduch

Na každou úroveň má hráč pouze omezené množství času. Ten je reprezentovaný vzduchem, jehož množství znázorňuje ukazatel `AIR`. Jakmile hráči dojde vzduch, ztrácí život a musí začít od začátku.
Pokud ale stihne úroveň dokončit dříve, zbylý vzduch se mu připočítá ke score.

### Score

High Score je zatím nejvyšší dosažené score. Score je současná hodnota získaná hráčem. Pokud svítí červeně, hráč použil cheat kód a jeho score se nezapočítává do celkového žebříčku.

Score se získává sbíráním klíčů a dokončením úrovně dřív, než vyprší vzduch.

### Životy

V nejnižší části obrazovky jsou záložní životy, tedy ty, které má hráč navíc. Každých 10_000 bodů score tento počet navíc zvyšuje. Pokud je zapnutý cheat nekonečných životů, figurky svítí zlatě a představuji stav bez cheatu.

### Konec hry

Existuje pouze jeden skutečný konec hry, a to prohra. Při ní se zapíše celkové získané skóre jako nejvyšší, pokud překročilo dozatimní nejvyšší skóre a nebyl použitý žádný cheat.

Hra má i "dobrý" konec v podobě vítězné obrazovky, ale v duchu arkádovek vítězství není konec, pouze další začátek. Po této obrazovce lze ve hře pokračovat. Skóre i životy se přenáší do dalšího kola.

Hru lze kdykoliv opustit stisknutím klávesy ESCAPE, v takovém případě je ale skóre zapomenuto.

## Jaké úrovně hra obsahuje

Současná verze hry obsahuje 8 úrovní převzatých z původní hry Manic Miner. Mají svá jména a čísla. 

Tabulka obsahuje i kód pro nahrání úrovně s použitím cheatu.
Klávesy představují čísla v řadě nad písmeny a je třeba je držet již před stisknutím klávesy pro potvrzení.

|Název|Číslo|Kód|
|-|-|-|
|Central Cavern|0|xxx|
|The Cold Room|1|1xx|
|The Menagerie|2|x2x|
|Abandoned Uranium Workings|3|12x|
|Eugene's Lair|4|xx3|
|Processing Plant|5|1x3|
|The Vat|6|x23|
|Wacky Amoebatrons|7|123|

