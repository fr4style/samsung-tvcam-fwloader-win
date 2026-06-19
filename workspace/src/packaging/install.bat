@echo off
setlocal EnableExtensions EnableDelayedExpansion

net session >nul 2>&1
if errorlevel 1 (
    echo Error: this installer must be run as Administrator.
    exit /b 1
)

set "SCRIPT_DIR=%~dp0"
set "TARGET_DIR=C:\Program Files\SamsungFWLoader"

pushd "%SCRIPT_DIR%" || (
    echo Error: unable to access installer directory.
    exit /b 1
)

if not exist "drivers\samsung-tvcam.inf" (
    echo Error: missing drivers\samsung-tvcam.inf
    popd
    exit /b 1
)

if not exist "samsung-fwloader.exe" (
    echo Error: missing samsung-fwloader.exe
    popd
    exit /b 1
)

if not exist "libusb-1.0.dll" (
    echo Error: missing libusb-1.0.dll
    popd
    exit /b 1
)

pnputil.exe /add-driver drivers\samsung-tvcam.inf /install
if errorlevel 1 (
    echo Error: driver installation failed.
    popd
    exit /b 1
)

if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"
if errorlevel 1 (
    echo Error: failed to create "%TARGET_DIR%".
    popd
    exit /b 1
)

copy /Y "samsung-fwloader.exe" "%TARGET_DIR%\" >nul
if errorlevel 1 (
    echo Error: failed to copy samsung-fwloader.exe.
    popd
    exit /b 1
)

copy /Y "libusb-1.0.dll" "%TARGET_DIR%\" >nul
if errorlevel 1 (
    echo Error: failed to copy libusb-1.0.dll.
    popd
    exit /b 1
)

set "CURRENT_PATH="
for /f "skip=2 tokens=2,*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul') do set "CURRENT_PATH=%%B"

echo(!CURRENT_PATH! | findstr /I /C:"%TARGET_DIR%" >nul
if errorlevel 1 (
    if defined CURRENT_PATH (
        set "NEW_PATH=!CURRENT_PATH!;%TARGET_DIR%"
    ) else (
        set "NEW_PATH=%TARGET_DIR%"
    )
    reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "!NEW_PATH!" /f >nul
    if errorlevel 1 (
        echo Error: failed to update PATH in the registry.
        popd
        exit /b 1
    )
)

popd

echo.
echo Samsung TV Camera Firmware Loader installed successfully.
echo.
echo Usage:
echo   1. Place FalconFW.bin in "%TARGET_DIR%"
echo   2. Connect the camera in firmware-update mode.
echo   3. Open a new Command Prompt and run:
echo      samsung-fwloader.exe
echo.
echo If the command is not found immediately, open a new terminal session.
exit /b 0
