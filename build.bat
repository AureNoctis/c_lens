@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"
set "WORKSPACE_DIR=%~dp0"
if "%WORKSPACE_DIR:~-1%"=="\" set "WORKSPACE_DIR=%WORKSPACE_DIR:~0,-1%"

set "BUILD_DIR=%WORKSPACE_DIR%\build"

set "MODE=debug"

for %%a in (%*) do set "%%~a=1"

if "%debug%"=="1"   set "MODE=debug"
if "%release%"=="1" set "MODE=release"

set "CC=clang"
set "INCLUDES="
set "RELEASE_FLAGS=-O3 -DNDEBUG=1"
set "DEBUG_FLAGS=-g -O0 -DDEBUG=1"

set "COMMON_FLAGS=-Wall -std=c17 -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -MJ compile_commands.raw.json"

if "%MODE%"=="debug" (
    set "OPTIONS=%COMMON_FLAGS% %DEBUG_FLAGS%"
) else (
    set "OPTIONS=%COMMON_FLAGS% %RELEASE_FLAGS%"
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

    %CC% %OPTIONS% %INCLUDES% "%WORKSPACE_DIR%\entrypoint.c" -o clens.exe

    if %errorlevel% neq 0 (
        popd
        exit /b %errorlevel%
    )

    if exist "compile_commands.raw.json" (
        powershell -Command "(Get-Content compile_commands.raw.json -Raw).Trim().TrimEnd(',') | ForEach-Object { '[' + $_ + ']' }" > "%WORKSPACE_DIR%\compile_commands.json"
        del compile_commands.raw.json
    )

    if "%run%"=="1"     clens.exe

popd

endlocal
