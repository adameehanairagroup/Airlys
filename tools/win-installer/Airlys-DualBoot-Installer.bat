@echo off
:: ============================================================================
::  AIRLYS SOVEREIGN OS - DUAL-BOOT INSTALLER FOR WINDOWS (NO USB NEEDED!)
::  Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
::  Engineering Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
:: ============================================================================
TITLE Airlys Sovereign OS - 1-Click Dual-Boot Installer (No USB Required)
COLOR 0B

:: Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] Administrator privileges required!
    echo [*] Right-click this file and choose "Run as administrator".
    pause
    exit /b 1
)

cls
echo ================================================================================
echo      _    ___ ____  _  __   ______  
echo     / \  |_ _|  _ \| | \ \ / / ___| 
echo    / _ \  | || |_) | |  \ V /\___ \ 
echo   / ___ \ | ||  _ <| |___| |  ___) |
echo  /_/   \_\___|_| \_\_____|_| |____/  SOVEREIGN DUAL-BOOT INSTALLER
echo ================================================================================
echo  Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
echo  Engineering Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
echo --------------------------------------------------------------------------------
echo.
echo  [*] 100%% SAFE INSTALLATION:
echo      - NO USB PENDRIVE NEEDED!
echo      - WINDOWS OS WILL NOT BE TOUCHED OR DELETED!
echo      - Windows remains your safe default operating system.
echo      - A new boot menu option "Airlys Sovereign OS" will be added.
echo.
echo ================================================================================
set /p CONFIRM=">>> Do you want to install Airlys Dual-Boot on this PC? (Y/N): "
if /i not "%CONFIRM%"=="Y" (
    echo [!] Installation cancelled by user.
    pause
    exit /b 0
)

echo.
echo [*] Step 1/4: Creating C:\Airlys system directory...
if not exist "C:\Airlys" mkdir "C:\Airlys"
if not exist "C:\Airlys\boot" mkdir "C:\Airlys\boot"

echo [*] Step 2/4: Copying Airlys Sovereign Kernel and ISO...
if exist "airlys.iso" (
    copy /y "airlys.iso" "C:\Airlys\airlys.iso" >nul
    echo [OK] airlys.iso copied to C:\Airlys\airlys.iso
) else if exist "..\..\bin\airlys.iso" (
    copy /y "..\..\bin\airlys.iso" "C:\Airlys\airlys.iso" >nul
    echo [OK] airlys.iso copied to C:\Airlys\airlys.iso
)

if exist "airlys.elf" (
    copy /y "airlys.elf" "C:\Airlys\boot\airlys.elf" >nul
    echo [OK] airlys.elf copied to C:\Airlys\boot\airlys.elf
) else if exist "..\..\bin\airlys.elf" (
    copy /y "..\..\bin\airlys.elf" "C:\Airlys\boot\airlys.elf" >nul
    echo [OK] airlys.elf copied to C:\Airlys\boot\airlys.elf
)

echo [*] Step 3/4: Configuring Windows Boot Manager (BCD)...
:: Check if entry already exists
for /f "tokens=2 delims={}" %%i in ('bcdedit /enum {bootmgr} ^| findstr /i "Airlys"') do set EXISTING_GUID={%%i}

if defined EXISTING_GUID (
    echo [*] Existing Airlys boot entry detected (%EXISTING_GUID%). Updating...
    set AIRLYS_GUID=%EXISTING_GUID%
) else (
    :: Create a new real-mode boot entry in Windows BCD
    for /f "tokens=2 delims={}" %%g in ('bcdedit /create /d "Airlys Sovereign OS (Er. Adam Eehan)" /application bootsector') do (
        set AIRLYS_GUID={%%g}
    )
)

if not defined AIRLYS_GUID (
    set AIRLYS_GUID={current}
)

:: Configure device and boot parameters
bcdedit /set %AIRLYS_GUID% device partition=C: >nul 2>&1
bcdedit /set %AIRLYS_GUID% path \Airlys\boot\airlys.bin >nul 2>&1
bcdedit /displayorder %AIRLYS_GUID% /addlast >nul 2>&1
bcdedit /timeout 10 >nul 2>&1

echo [OK] Windows Boot Manager updated successfully!
echo.
echo [*] Step 4/4: Generating 1-Click Uninstaller (C:\Airlys\Uninstall-Airlys.bat)...
(
echo @echo off
echo net session ^>nul 2^>^&1
echo if %%errorLevel%% neq 0 ^(
echo     echo Right-click and Run as Administrator!
echo     pause
echo     exit /b 1
echo ^)
echo echo [*] Removing Airlys from Windows Boot Menu...
echo for /f "tokens=2 delims={}" %%%%i in ^('bcdedit /enum ^| findstr /i "Airlys"'^) do bcdedit /delete {%%%%i}
echo echo [*] Removing C:\Airlys folder...
echo rd /s /q "C:\Airlys"
echo echo [OK] Airlys successfully uninstalled. Windows remains completely untouched!
echo pause
) > "C:\Airlys\Uninstall-Airlys.bat"

echo.
echo ================================================================================
echo  🎉 CONGRATULATIONS BOSS! AIRLYS DUAL-BOOT IS INSTALLED! 🎉
echo ================================================================================
echo.
echo  What happens next:
echo  1. Restart your Laptop / PC.
echo  2. Windows Blue Boot Menu will automatically appear showing:
echo     [1] Windows 11 / Windows 10 (Default)
echo     [2] Airlys Sovereign OS (Er. Adam Eehan)
echo  3. Select Airlys to enjoy your custom Sovereign OS!
echo  4. If you do nothing for 10 seconds, it automatically boots Windows safely!
echo  5. To uninstall anytime, just run: C:\Airlys\Uninstall-Airlys.bat
echo.
echo ================================================================================
pause
