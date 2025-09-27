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
    - Znaky, 8- 16- 32- bitová celá čísla, celá čísla bez znaménka, čísla s pobyblivou desetinnou čárkou
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
Otevřít terminál v místě stažení:
```sh
cd cppcompiler/
make
```
Spuštení kompileru:
```sh
./lang
```
### Windows
Kompilace na operačním systému Windows je (velmi) experimentální a není podporována.
### MacOS a jiné operační systémy
Kompilace na MacOS a jiných operačních systémech není a v dohledné době nebude podporována.
