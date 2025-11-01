:: !!! IMPORTANT: RUN windows.bat also !!!
@echo off
cd /d "%~dp0"

:: Check if the script is running as administrator
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo This script needs administrator privileges...
    :: Relaunch the script with administrator privileges
    powershell -Command "Start-Process cmd -ArgumentList '/c \"%~f0\"' -Verb RunAs"
    exit /b
)

:: Your actual batch commands go below
echo Running with administrator privileges!

:: Set variables
set MINGW_INSTALL_DIR=C:\MSYS2
set MINGW_INSTALL_DIR_SAVE=%MINGW_INSTALL_DIR:\=/%/ucrt64
set MINGW_INSTALL_DIR_SAVE_PATH=%MINGW_INSTALL_DIR_SAVE:/=\%\bin
set ENV_FILE=%~dp0..\..\..\.env.cmake

echo Installing MinGW-w64...
winget install --id=MSYS2.MSYS2 -e --location "%MINGW_INSTALL_DIR%"

echo Running MSYS2 shell...
cmd.exe /C "%MINGW_INSTALL_DIR%\usr\bin\bash.exe -lc 'pacman -Syu --noconfirm'"
cmd.exe /C "%MINGW_INSTALL_DIR%\usr\bin\bash.exe -lc 'pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain --noconfirm'"

echo Setting MINGW environment variable...
if not exist "%ENV_FILE%" (
    echo set(MINGW_PATH %%MINGW_INSTALL_DIR_SAVE%%^)) >> "%ENV_FILE%"
) else (
    findstr /C:"set(MINGW_PATH" "%ENV_FILE%" >nul
    if errorlevel 1 (
        echo set(MINGW_PATH %%MINGW_INSTALL_DIR_SAVE%%^)) >> "%ENV_FILE%"
    ) else (
        powershell -Command "(Get-Content -path '%ENV_FILE%') -replace 'set\(MINGW_PATH.*\)', 'set(MINGW_PATH \"%MINGW_INSTALL_DIR_SAVE%\")' | Set-Content -Path '%ENV_FILE%'"
    )
)

echo Setting MINGW in user PATH environmental variable...
set currentPath=
for /f "tokens=1,2,*" %%A in ('reg query HKCU\Environment /v PATH') do (
    if "%%A"=="PATH" (
        set currentPath=%%C
    )
)
echo %currentPath% | findstr /i /c:"%MINGW_INSTALL_DIR_SAVE_PATH%" >nul
if %errorlevel%==0 (
    echo MINGW_INSTALL_DIR_SAVE_PATH is already in the currentPath.
    goto :skipAddPath
) else (
    :: Append %MINGW_INSTALL_DIR_SAVE_PATH% to the current user PATH
    if not "%currentPath:~-1%"==";" set "currentPath=%currentPath%;"
    set "currentPath=%currentPath%%MINGW_INSTALL_DIR_SAVE_PATH%"
)
:: Update the user-level PATH environment variable persistently
powershell -Command "[System.Environment]::SetEnvironmentVariable('PATH', '%currentPath%', [System.EnvironmentVariableTarget]::User)"
echo MINGW_INSTALL_DIR_SAVE_PATH has been added to the PATH.

:: Skip to avoid adding the same path multiple times
:skipAddPath

echo Setting MINGW in system PATH environmental variable...
:: Save the current system PATH
set currentSystemPath=
for /f "tokens=1,2,*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH') do (
    if "%%A"=="PATH" (
        set currentSystemPath=%%C
    )
)

:: Check if the custom path is already in the system PATH
echo %currentSystemPath% | findstr /i /c:"%MINGW_INSTALL_DIR_SAVE_PATH%" >nul
if %errorlevel%==0 (
    echo MINGW_INSTALL_DIR_SAVE_PATH is already in the system PATH.
    goto :skipSystemAddPath
) else (
    :: Append %MINGW_INSTALL_DIR_SAVE_PATH% to the current system PATH
    set "currentSystemPath=%MINGW_INSTALL_DIR_SAVE_PATH%;%currentSystemPath%"
)

:: Update the system-level PATH environment variable persistently (admin privileges required)
echo MINGW_INSTALL_DIR_SAVE_PATH has been added to the system PATH.
powershell -Command "[System.Environment]::SetEnvironmentVariable('PATH', '%currentSystemPath%', [System.EnvironmentVariableTarget]::Machine)"

:skipSystemAddPath

echo Installation complete.
pause