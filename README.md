# Compiler vlastního programovací jazyka
Autor: **Ondřej Suk**

(Pokud máte pro tento projekt dobré jméno, řekněte mi ho)

Tohle je můj projekt pro Maturitu z Informatiky. Jedná se compiler pro můj vlastní programovací jazyk.

## Co je cílem projektu
- Funkční kompilace programu pro Linux v mém vlastním programovacím jazyce
- Vlastnosti jazyka:
  - Klíčová slova
    - Funkce o jednom parametru (`fn`)
    - Konstantní lokální a globální proměnné (`let`)
    - Testování podmínek (`if` a `else`)
    - Smyčky (`while`)
  - Operátory
    - Aritmetické (`+`, `-`, `*`, `/`, a `%`)
    - Porovnávací (`==`, `!=`, `>`, `<`, `>=`, `<=`)
    - Logické (`!`, `||`, `&&`)
  - Předem definované funkce
    - `getchar` a `putchar`z C standardní knihovny

## Co je vedlejším cílem (seřazeno dle priority)
- Vlastnosti jazyka:
  - Funkce s vícero parametry
  - Smyčka `for`
  - Datové typy kromě 64 - bitových celých čísel se znaménkem
    - Znaky, 8- 16- 32- bitová celá čísla, celá čísla bez znaménka, čísla s pohyblivou desetinnou čárkou
    - Seznamy, Struktury, Třídy, Řetězce
  - Měnitelné proměnné
    - Přiřazovací operátor `=`
  - Klíčová slova `return`, `break` a `continue`
  - Definice vlastních klíčových slov
  - Předem definované C funkce kromě `getchar` a `putchar`

## Co není cílem
- Kompilace pro jiné operační systémy než Linux
- Schopnost práce s vícero soubory
- Importování knihoven napsaných v jazyce C
- Vytváření a importování knihoven napsaných v mém vlastním jazyce
- LSP, či jiné nástroje pro snazší práci s jazykem
- Optimalizace

## Časový plán
### Dokončeno před začátkem školního roku
- [X] Tokenizace
- [X] Třídění tokenů dle druhu
- [X] Analýza klíčových slov
- [X] Stavba Abstract Syntax Tree
### Září / Říjen
- [X] Ukládání jmen symbolů
- [ ] Globální a lokální funkce
- [ ] Globální a lokální proměnné
- [ ] Kostra kódu pro generaci LLVM IR
### Listopad / Prosinec
- [ ] Generace LLVM IR
- [ ] Vytváření spustitelných souborů z LLVM IR
### Leden, Únor...
Implementace Vedlejších cílů
- [ ] Funkce o vícero parametrech
- [ ] Smyčka `for`
- [ ] Datové typy
- [ ] Měnitelné proměnné
- [ ] `return`, `break` a `continue`
- [ ] Custom klíčová slova
- [ ] Standardní knihovna

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
