# Compiler vlastního programovací jazyka
Autor: **Ondřej Suk**

(Pokud máte pro tento projekt dobré jméno, řekněte mi ho)

Tohle je můj projekt pro Maturitu z Informatiky. Jedná se compiler pro můj vlastní programovací jazyk.

## Co je hlavním cílem projektu
- Funkční kompilace programu pro Linux x86_64 v mém vlastním programovacím jazyce
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
    - `getchar` a `putchar`, které fungují jako ty z C standardní knihovny

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
  - Předem definované funkce kromě `getchar` a `putchar`

## Co není cílem
- Kompilace pro jiné operační systémy a architektury než Linux x86_64
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
- [X] Globální funkce
- [X] Vytváření spustitelných souborů z LLVM IR
- [X] Kostra kódu pro generaci LLVM IR
- [X] Aritmetické operátory
- [X] Funkce `putchar`
### Listopad / Prosinec
- [ ] Funkce `getchar`
- [ ] Funkce s vícero řádky
- [ ] klíčové slovo `if`
- [ ] Porovnávací operátory
- [ ] Logické operátory
- [ ] Lokální funkce
- [ ] Globální proměnné
- [ ] Lokální proměnné
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
- **make**
- **clang++**
- **llvm**
- **lld**
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
Kód je definován v `code` a výsledný program se jmenuje `program`.
### Windows (nedoporučeno)
Kompilace na operačním systému Windows je složitější.
#### Předpoklady
- **msys2**
- **make**
- **mingw-w64-x86_64-clang** (clang++)
- **mingw-w64-x86_64-llvm**
- **mingw-w64-x86_64-lld**
#### Kroky
Otevřete MSYS2 MINGW terminál v místě stažení:
```sh
cd cppcompiler/
make
```
Spuštení compileru:
```sh
./lang
```
Kód je definován v `code` a výsledný program se jmenuje `program`.
Výsledný soubor vyžaduje některá DLL z MSYS2 a MINGW, takže pokud soubor spouštíte mimo MSYS2, tak je nutné je přidat do souboru s `lang.exe`.
### MacOS a jiné operační systémy
Kompilace na MacOS a na jiných operačních systémech není a v dohledné době nebude podporována.
