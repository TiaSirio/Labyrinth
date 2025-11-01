@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

:: Check if the script is running as administrator
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo This script needs administrator privileges...
    :: Relaunch the script with administrator privileges, passing all arguments
    powershell -Command "Start-Process cmd -ArgumentList '/c \"%~f0\" %*' -Verb RunAs"
    exit /b
)

:: Your actual batch commands go below
echo Running with administrator privileges!
echo Arguments passed: %*

for /F "tokens=* delims=" %%i in (..\..\configuration_files\versions_config.env) do set %%i

set VULKAN_SDK_URL=https://sdk.lunarg.com/sdk/download/%VULKAN_VERSION%/windows/vulkan-sdk.exe
set VULKAN_SDK_DIR=C:\VulkanSDK\%VULKAN_VERSION%
set TEMP_DOWNLOAD_DIR=%USERPROFILE%\Desktop

:: Default configuration file
set "inifile=..\..\configuration_files\vulkan_installer_config_install.ini"

:: Check the input parameter
if "%~1"=="dev" (
    set "inifile=..\..\configuration_files\vulkan_installer_config_dev.ini"
) else if "%~1"=="install" (
    set "inifile=..\..\configuration_files\vulkan_installer_config_install.ini"
) else (
    echo Invalid input provided. Defaulting to "install".
    set "inifile=..\..\configuration_files\vulkan_installer_config_install.ini"
)

set inifile=%~dp0%inifile%
echo Config file: %inifile%

:: Initialize the arguments string
set "arguments="

:: Loop through each line in the file
for /f "usebackq tokens=1,2 delims==" %%A in (`findstr "=" "%inifile%"`) do (
    set "key=%%A"
    set "value=%%B"

    :: Remove any leading/trailing whitespace
    for /f "delims= " %%C in ("!value!") do set "value=%%C"

    :: Check if the value is true
    if /i "!value!"=="true" (
        :: Append the key to the arguments string
        if defined arguments (
            set "arguments=!arguments! %%A"
        ) else (
            set "arguments=%%A"
        )
    )
)

:: Output the constructed arguments
echo Arguments: !arguments!

:: Check if Vulkan SDK directory exists and is not empty
if exist "%VULKAN_SDK_DIR%\" (
    for /f %%i in ('dir /b "%VULKAN_SDK_DIR%"') do set VULKAN_SDK_DIR_NOT_EMPTY=1
)
if not defined VULKAN_SDK_DIR_NOT_EMPTY (
    echo Downloading Vulkan SDK...
    curl --output "%TEMP_DOWNLOAD_DIR%\vulkan-sdk.exe" %VULKAN_SDK_URL%

    echo Installing Vulkan SDK...
    start /wait "" "%TEMP_DOWNLOAD_DIR%\vulkan-sdk.exe" --accept-licenses --default-answer --confirm-command install !arguments!
    echo Deleting Vulkan SDK installer...
    del "%TEMP_DOWNLOAD_DIR%\vulkan-sdk.exe"
) else (
    echo Vulkan SDK directory already exists and is not empty, skipping download and extraction.
)

net user Administrator /active:no