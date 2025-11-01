@echo off
setlocal
cd /d "%~dp0"

:: Load environment variables from config file
for /F "tokens=* delims=" %%i in (..\..\configuration_files\versions_config.env) do set %%i

:: Set download URLs
set GLFW_URL=https://github.com/glfw/glfw/releases/download/%GLFW_VERSION%/glfw-%GLFW_VERSION%.bin.WIN64.zip
set GLM_URL=https://github.com/g-truc/glm/releases/download/%GLM_VERSION%/glm-%GLM_VERSION%-light.zip
set TINYOBJLOADER_URL=https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/master/tiny_obj_loader.h
set STB_IMAGE_URL=https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h

:: Set installation paths
set INSTALL_DIR=%~dp0..\..\..\libraries
set "INSTALL_DIR_SAVE=%INSTALL_DIR:\=/%"

set VULKAN_EXT_CHOICE_DIR=%~dp0..\vulkan_scripts\vulkan_installer_windows.bat

set GLFW_DIR=%INSTALL_DIR%\GLFW
set GLFW_DIR_SAVE=%GLFW_DIR:\=/%

set GLM_DIR=%INSTALL_DIR%\GLM
set GLM_DIR_SAVE=%GLM_DIR:\=/%

set TINYOBJLOADER_DIR=%INSTALL_DIR%\TinyObjLoader
set TINYOBJLOADER_DIR_SAVE=%TINYOBJLOADER_DIR:\=/%

set STB_DIR=%INSTALL_DIR%\STB
set STB_dIR_SAVE=%STB_DIR:\=/%

set GRAPHVIZ_INSTALL_DIR=C:\Graphviz
set GRAPHVIZ_INSTALL_DIR_SAVE=%GRAPHVIZ_INSTALL_DIR:\=/%

set IFW_INSTALL_DIR=%~dp0..\..\..\libraries\IFW
set IFW_INSTALL_DIR_SAVE=%IFW_INSTALL_DIR:\=/%

set ENV_FILE=%~dp0..\..\..\.env.cmake
set MARKER_FILE=%~dp0..\..\..\.marker_file.txt

:: Create installation directory
mkdir "%INSTALL_DIR%"
del /f "%ENV_FILE%"
del /f "%MARKER_FILE%"

echo =====================================
echo Do you want to install IFW as installer tool? (Y or N)
echo =====================================
set /p choice_ifw=Your choice: 

if /i "%choice_ifw%"=="Y" goto install_ifw
if /i "%choice_ifw%"=="N" goto skip_ifw
echo Invalid choice! Please enter Y or N.
goto end

:install_ifw

mkdir "%IFW_INSTALL_DIR%"

if exist "%IFW_INSTALL_DIR%\" (
    for /f %%i in ('dir /b /a-d "%IFW_INSTALL_DIR%"') do set IFW_INSTALL_DIR_NOT_EMPTY=1
)
if not defined IFW_INSTALL_DIR_NOT_EMPTY (
    echo Installing aqtinstall...
    :: Install aqtinstall
    pip3 install --upgrade pip
    pip3 install aqtinstall

    :: Install IFW
    echo Installing IFW...
    aqt install-tool --outputdir "%IFW_INSTALL_DIR%" windows desktop tools_ifw %IFW_VERSION%
) else (
    echo IFW directory already exists and is not empty, skipping installation.
    goto skip_ifw
)

echo Setting IFW_PATH environment variable...
if not exist "%ENV_FILE%" (
    echo set(IFW_PATH %%IFW_INSTALL_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(IFW_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(IFW_PATH %%IFW_INSTALL_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(IFW_PATH.*\)', 'set(IFW_PATH \"%IFW_INSTALL_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

:skip_ifw

echo =====================================
echo Do you want to install Doxygen and Graphviz to create the documentation? (Y or N)
echo =====================================
set /p choice_dox=Your choice: 

if /i "%choice_dox%"=="Y" goto install_dox
if /i "%choice_dox%"=="N" goto skip_dox
echo Invalid choice! Please enter Y or N.
goto end

:install_dox
:: Install doxygen and graphviz
echo Installing Doxygen...
winget install -e --id=DimitriVanHeesch.Doxygen
echo Installing Graphviz...
winget install -e --id Graphviz.Graphviz --location "%GRAPHVIZ_INSTALL_DIR%"

echo Setting GRAPHVIZ_PATH environment variable...
if not exist "%ENV_FILE%" (
    echo set(GRAPHVIZ_PATH %%GRAPHVIZ_INSTALL_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(GRAPHVIZ_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(GRAPHVIZ_PATH %%GRAPHVIZ_INSTALL_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(GRAPHVIZ_PATH.*\)', 'set(GRAPHVIZ_PATH \"%GRAPHVIZ_INSTALL_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

:skip_dox

echo =====================================
echo Do you want to install Vulkan version %VULKAN_VERSION%? (Y or N)
echo =====================================
set /p choice_vulkan=Your choice: 

if /i "%choice_vulkan%"=="Y" goto install_vulkan
if /i "%choice_vulkan%"=="N" goto skip_vulkan
echo Invalid choice! Please enter Y or N.
goto end

:install_vulkan
:: Install Vulkan SDK
call "%VULKAN_EXT_CHOICE_DIR%" dev

:skip_vulkan

:: Install CMake
echo Installing CMake...
winget install CMake

mkdir "%GLFW_DIR%"
:: Check if GLFW directory exists and is not empty
if exist "%GLFW_DIR%\" (
    for /f %%i in ('dir /b /a-d "%GLFW_DIR%"') do set GLFW_DIR_NOT_EMPTY=1
)
if not defined GLFW_DIR_NOT_EMPTY (
    echo Downloading GLFW...
    curl -L -o "%INSTALL_DIR%\glfw.zip" %GLFW_URL%
    echo Extracting GLFW...
    mkdir "%INSTALL_DIR%\glfw_temp"
    ::Extract the archive
    powershell -Command "Expand-Archive -Path '%INSTALL_DIR%\glfw.zip' -DestinationPath '%INSTALL_DIR%\glfw_temp'"
    ::Move the contents to the destination folder
    for /d %%D in ("%INSTALL_DIR%\glfw_temp\*") do (
        xcopy "%%D\*" "%GLFW_DIR%\" /E /H /K
    )
    ::Remove the temporary folder
    rmdir /s /q "%INSTALL_DIR%\glfw_temp"
    echo Deleting GLFW zip...
    del "%INSTALL_DIR%\glfw.zip"
) else (
    echo GLFW directory already exists and is not empty, skipping download and extraction.
)

echo Setting GLFW_PATH environment variable...
if not exist "%ENV_FILE%" (
    echo set(GLFW_PATH %%GLFW_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(GLFW_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(GLFW_PATH %%GLFW_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(GLFW_PATH.*\)', 'set(GLFW_PATH \"%GLFW_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

mkdir "%GLM_DIR%"
:: Check if GLM directory exists and is not empty
if exist "%GLM_DIR%\" (
    for /f %%i in ('dir /b /a-d "%GLM_DIR%"') do set GLM_DIR_NOT_EMPTY=1
)
if not defined GLM_DIR_NOT_EMPTY (
    echo Downloading GLM...
    curl -L -o "%INSTALL_DIR%\glm.zip" %GLM_URL%
    echo Extracting GLM...
    ::Extract the archive
    powershell -Command "Expand-Archive -Path '%INSTALL_DIR%\glm.zip' -DestinationPath '%INSTALL_DIR%\glm_temp'"
    ::Move the contents to the destination folder
    mkdir "%GLM_DIR%\glm"
    for /d %%D in ("%INSTALL_DIR%\glm_temp\*") do (
        xcopy "%%D\*" "%GLM_DIR%\glm\" /E /H /K
    )
    ::Remove the temporary folder
    rmdir /s /q "%INSTALL_DIR%\glm_temp"
    echo Deleting GLM zip...
    del "%INSTALL_DIR%\glm.zip"
) else (
    echo GLM directory already exists and is not empty, skipping download and extraction.
)

echo Setting GLM_PATH environment variable...
if not exist "%ENV_FILE%" (
    echo set(GLM_PATH %%GLM_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(GLM_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(GLM_PATH %%GLM_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(GLM_PATH.*\)', 'set(GLM_PATH \"%GLM_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

mkdir "%TINYOBJLOADER_DIR%"
:: Check if TinyObjLoader directory exists and is not empty
if exist "%TINYOBJLOADER_DIR%\" (
    for /f %%i in ('dir /b /a-d "%TINYOBJLOADER_DIR%"') do set TINYOBJLOADER_DIR_NOT_EMPTY=1
)
if not defined TINYOBJLOADER_DIR_NOT_EMPTY (
    echo Downloading tiny_obj_loader.h...
    curl -L -o "%INSTALL_DIR%\tiny_obj_loader.h" %TINYOBJLOADER_URL%
    echo Moving tiny_obj_loader.h to TinyObjLoader directory...
    move "%INSTALL_DIR%\tiny_obj_loader.h" "%TINYOBJLOADER_DIR%"
) else (
    echo TinyObjLoader directory already exists and is not empty, skipping download.
)

echo Setting TINYOBJLOADER_PATH environment variable...
if not exist "%ENV_FILE%" (
    echo set(TINYOBJLOADER_PATH %%TINYOBJLOADER_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(TINYOBJLOADER_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(TINYOBJLOADER_PATH %%TINYOBJLOADER_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(TINYOBJLOADER_PATH.*\)', 'set(TINYOBJLOADER_PATH \"%TINYOBJLOADER_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

mkdir "%STB_DIR%"
:: Check if STB directory exists and is not empty
if exist "%STB_DIR%\" (
    for /f %%i in ('dir /b /a-d "%STB_DIR%"') do set STB_DIR_NOT_EMPTY=1
)
if not defined STB_DIR_NOT_EMPTY (
    echo Downloading stb_image.h...
    curl -L -o "%INSTALL_DIR%\stb_image.h" %STB_IMAGE_URL%
    echo Moving stb_image.h to STB directory...
    move "%INSTALL_DIR%\stb_image.h" "%STB_DIR%"
) else (
    echo STB directory already exists and is not empty, skipping download.
)

echo Setting STB_PATH environment variable...
if not exist "%ENV_FILE%" (
    echo set(STB_PATH %%STB_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(STB_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(STB_PATH %%STB_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(STB_PATH.*\)', 'set(STB_PATH \"%STB_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

echo Prerequisite script run at %date% %time% > "%MARKER_FILE%"
echo Installation complete.
:end
endlocal
pause