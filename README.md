# CrazedCaver 2.0

## Specifikace

Manic Miner je hra původně vydaná na počítače ZX Spectrum. Je to jedna z nejznámějších plošinovek nejen pro ZX Spectrum, ale obecně. Tento projekt obsahuje hru napsanou v jazyce C#, jež z Manic Miner silně čerpá. Mezi hlavní rozdíly mezi hrami, kterých si uživatel všimne, patří nová grafika, jiný styl pohybu a také pozměněné ovládání. Program se mimo to liší i tím, jak počítá simulaci, kolize atp.

Verze 2.0 obsahuje navíc i LevelEditor v jazyce C#. Tento self-contained program umožňuje jednodušší správu levelů. S hrou přímo nekomunikuje, pouze pro ni staví soubory, z kterých nahrává svůj obsah.

## Struktura projektu

Projekt je rozložen do tří podsložek.

### Core

Tato složka obsahuje část představující knihovnu herního enginu.

### CrazedCaver

Zde jsou definované entity, scény a běh hry CrazedCaver. Složka obsahuje i vstupní soubory s texturami a popisem úrovní.

### LevelEditor

V této složce se nachází zdrojový kód a data LevelEditor podprojektu.

### docs

Zde se nachází herní dokumentace. Dokumentace LevelEditor podprojektu se nachází ve stejnojmenné složce o úroveň výše v repozitáři.

## Instalace a spuštění

### CrazedCaver

Pro sestavení přejděte do složky `scripts` a spusťte `buildCrazedCaver.sh`. Po dokončení lze program spustit jako `CrazedCaver/build/CrazedCaver`.

```sh
# in src folder
cd scripts

# if not executable, add executable rights
chmod +x ./buildCrazedCaver
./buildCrazedCaver

# launching
../CrazedCaver/build/CrazedCaver
```

Pro spuštění bez sestavení přejděte do složky `CrazedCaver` a spusťte příkaz `dotnet run`.

```sh
# in src folder
cd CrazedCaver
dotnet run
```

Ovládání hry je popsáno v [uživatelské](docs/user.md) části dokumentace.

Podrobnější rozbor vstupních souborů je v [programátorské](docs/programmer.md) části dokumentace.

### LevelEditor

Pro uživatele Windows, všechny dependencies jsou již součástí systému samotného.

Pro linux uživatele, Level editor vyžaduje systémové balíčky, které uživatel nemusí mít na svém PC nainstalované. Tyto balíčky nelze zahrnout automaticky, proto je třeba je nejdříve manuálně nainstalovat.

Tyto balíčky slouží pro kompilaci a vykreslování okna.

###### Fedora
```bash
sudo dnf install gcc-c++ cmake make libX11-devel libXrandr-devel libXcursor-devel libXi-devel mesa-libGL-devel freetype-devel systemd-devel
```

###### Debian-based
```bash
sudo apt update
sudo apt install build-essential cmake libx11-dev libxrandr-dev libxcursor-dev libxi-dev libgl1-mesa-dev libfreetype-dev libudev-dev
```

###### Arch/Manjaro
```bash
sudo pacman -S base-devel cmake libx11 libxrandr libxcursor libxi mesa freetype2 systemd
```


Pro sestavení přejděte do složky `scripts` a spusťte `buildLevelEditor.sh`. Po dokončení lze editor spustit jako `LevelEditor/build/LevelEditor`

```sh
# in src folder
cd scripts

# if not executable, add executable rights
chmod +x ./buildLevelEditor
./buildLevelEditor

# launching
../LevelEditor/build/LevelEditor/LevelEditor
```

## Dokumentace

* [Uživatelská dokumentace](docs/user.md)
* [Programátorská dokumentace](docs/programmer.md)