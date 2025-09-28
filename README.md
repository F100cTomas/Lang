# Maturitní projekt 2026
Autor: **Ondřej Suk**

(Pokud máte pro tento projekt dobré jméno, řekněte mi ho)

Tohle je můj projekt pro Maturitu z Informatiky. Jedná se compiler pro můj vlastní programovací jazyk.

## Co je cílem projektu
- Funkční kompilace programu pro Linux v mém vlastním programovacím jazyce
  - Funkce (klíčové slovo `fn`)
  - Konstantní globální a lokální proměnné (klíčové slovo `let`)
  - Testování podmínek pomocí klíčových slov `if` a `else`
  - Smyčka `while`
  - Shopnost použít funkce standardní knihovny jazyka C `getchar` a `putchar`

## Co není cílem, ale může se objevit ve finálním výsledku
- Kompilace pro jiné operační systémy, než Linux
- Shopnost práce s vícero soubory
- Importování knihoven napsaných v jazyce C
- Vytváření a importování knihoven napsaných v mém vlastním jazyce
- Vlastnosti jazyka:
  - Datové typy kromě 64 - bitových celých čísel se znaménkem
    - Znaky, 8- 16- 32- bitová celá čísla, celá čísla bez znaménka, čísla s pohyblivou desetinnou čárkou
    - Seznamy, Struktury, Třídy, Řetězce
  - Měnitelné proměnné
  - Smyčka `for`
  - Klíčová slova `return`, `break` a `continue`
  - Funkce s vícero parametry
  - Předem definované C funkce kromě `getchar` a `putchar`
  - Definice vlastních klíčových slov
- LSP, či jiné nástroje pro snažší práci s jazykem
- Optimalizace

## Předběžný časový plán
### Již hotovo
- Tokenizace
- Třídění tokenů dle druhu
- Analýza klíčových slov
- Stavba Abstract Syntax Tree
### Září / Říjen
- Globalní / lokální proměnné a funkce
### Listopad / Prosinec
- Generace LLVM IR a vznik spustitelných souborů
### Leden, Únor...
**TODO**

## Dokumentace
Dokumentace je psána v LaTeXu a nachází se v `doc/dokumentace.tex`.

## Kompilace
### Linux
#### Předpoklady
- **clang++**
- **llvm**
- **make**
#### Kroky
Otevřete terminál v místě stažení:
```sh
cd cppcompiler/
make
```
Spuštení compileru:
```sh
./lang
```
### Windows (nedoporučeno)
Kompilace na operačním systému Windows je (velmi) experimentální.
#### Předpoklady
- **msvc** (Visual Studio C++)
#### Kroky
- Jděte na `https://github.com/llvm/llvm-project/releases` a stáhněte si nejnovější verzi.
(`clang+llvm-X.X.X-x86_64-pc-windows-msvc.tar.xz`)
- Extrahujte archiv.
- Jděte do místa stažení a otevřete terminál ve složce `cppcompiler`:
```cmd
.\setup.bat <jméno složky s extrahovaným obsahem>
.\build.bat
```
Spuštění compileru:
```cmd
.\lang.exe
```
### MacOS a jiné operační systémy
Kompilace na MacOS a na jiných operačních systémech není a v dohledné době nebude podporována.
