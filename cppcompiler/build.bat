@echo off
setlocal enabledelayedexpansion

REM === Tools and paths ===
set CLANG=.\llvm_bin\clang-cl.exe
set SRCDIR=src
set BUILDDIR=build
set OUT=lang.exe
set INC=src
set EXTINC=llvm_include
set LIBPATH=llvm_lib

REM === Normal includes ===
set INCLUDES=
for %%d in (%INC%) do (
    set INCLUDES=!INCLUDES! /I %%d
)

REM === External includes (suppress warnings) ===
set INCLUDES=!INCLUDES! /external:I %EXTINC% /external:W0

REM === Collect all sources recursively ===
set SOURCES=
for /R %SRCDIR% %%f in (*.cpp) do (
    set SOURCES=!SOURCES! %%f
)

REM === Build list of objects and compile ===
set OBJECTS=
for %%f in (%SOURCES%) do (
    REM Strip "src\" prefix
    set rel=%%f
    set rel=!rel:%CD%\%SRCDIR%\=!

    REM Replace .cpp with .obj under build\
    set obj=%BUILDDIR%\!rel:.cpp=.obj!

    REM Ensure directory exists
    for %%d in ("!obj!") do if not exist "%%~dpd" mkdir "%%~dpd"

    set OBJECTS=!OBJECTS! !obj!

    echo Compiling %%f -> !obj!
	%CLANG% /std:c++17 /EHsc /O2 /Wall -Wno-c++98-compat -Wno-c++98-compat-pedantic -Xclang -Wno-unsafe-buffer-usage ^
		%INCLUDES% -c "%%f" /Fo"!obj!"
)

REM === Collect all .lib files ===
set LIBS=
for %%f in (%LIBPATH%\*.lib) do (
    set LIBS=!LIBS! %%f
)

REM === Link all objects ===
echo Linking %OUT% ...
%CLANG% %OBJECTS% ^
  /Fe:%OUT% ^
  /link /LIBPATH:%LIBPATH% %LIBS% ntdll.lib kernel32.lib advapi32.lib

endlocal
