@echo off
setlocal

REM === Get folder path from argument ===
if "%~1"=="" (
    echo Usage: %0 ^<uncompressed_clang_folder^>
    exit /b 1
)
set SRC=%~1

REM === Define target folders ===
set DEST_BIN=llvm_bin
set DEST_LIB=llvm_lib
set DEST_INC=llvm_include

REM === Create target folders if they don't exist ===
if not exist "%DEST_BIN%" mkdir "%DEST_BIN%"
if not exist "%DEST_LIB%" mkdir "%DEST_LIB%"
if not exist "%DEST_INC%" mkdir "%DEST_INC%"

REM === Copy binaries ===
echo Copying binaries...
copy /Y "%SRC%\bin\*" "%DEST_BIN%\" >nul
echo Done.

REM === Copy libraries ===
echo Copying libraries...
copy /Y "%SRC%\lib\*" "%DEST_LIB%\" >nul
echo Done.

REM === Copy include files ===
echo Copying include files...
xcopy /E /I /Y "%SRC%\include" "%DEST_INC%\" >nul
echo Done.

echo LLVM/Clang files copied successfully.
