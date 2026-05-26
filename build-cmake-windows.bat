@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Messenger Build Script
echo ========================================
echo.

set BUILD_DIR=build
set CONFIG=Release

if not "%1"=="" (
    set CONFIG=%1
)

echo Build configuration: %CONFIG%
echo.

if exist %BUILD_DIR% (
    echo Removing existing build directory...
    rmdir /s /q %BUILD_DIR%
)

if exist bin (
    echo Removing bin directory...
    rmdir /s /q bin
)

echo Creating build directory...
mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo Detecting available generator...

set GENERATOR=
set PLATFORM=

where ninja >nul 2>nul
if %errorlevel% equ 0 (
    echo Found Ninja
    set GENERATOR=Ninja
    set PLATFORM=
) else (
    where msbuild >nul 2>nul
    if %errorlevel% equ 0 (
        echo Found Visual Studio
        set GENERATOR=Visual Studio 17 2022
        set PLATFORM=-A x64
    ) else (
        echo Found MinGW
        set GENERATOR=MinGW Makefiles
        set PLATFORM=
    )
)

if "%GENERATOR%"=="" (
    echo No suitable generator found!
    cd ..
    exit /b 1
)

echo Using generator: %GENERATOR% %PLATFORM%
echo.

echo Configuring CMake...
cmake .. -G "%GENERATOR%" %PLATFORM% -DCMAKE_BUILD_TYPE=%CONFIG%

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

echo.
echo Building...
cmake --build . --config %CONFIG% --parallel

if errorlevel 1 (
    echo.
    echo Build failed!
    cd ..
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Output location:
if "%GENERATOR%"=="Ninja" (
    echo   bin\%CONFIG%\x64\Client\Client.exe
    echo   bin\%CONFIG%\x64\Server\Server.exe
    echo   bin\%CONFIG%\x64\lib\Common.lib
) else (
    echo   bin\%CONFIG%\x64\Client\Client.exe
    echo   bin\%CONFIG%\x64\Server\Server.exe
    echo   bin\%CONFIG%\x64\lib\Common.lib
)
echo.

pause
cd ..