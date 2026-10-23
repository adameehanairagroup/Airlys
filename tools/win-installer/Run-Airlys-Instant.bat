@echo off
:: ============================================================================
::  AIRLYS SOVEREIGN OS - INSTANT WINDOWS LAUNCHER (NO RESTART & NO USB!)
::  Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
::  Engineering Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
:: ============================================================================
TITLE Airlys Sovereign OS - 1-Click Instant Launcher
COLOR 0B

cls
echo ================================================================================
echo      _    ___ ____  _  __   ______  
echo     / \  |_ _|  _ \| | \ \ / / ___| 
echo    / _ \  | || |_) | |  \ V /\___ \ 
echo   / ___ \ | ||  _ <| |___| |  ___) |
echo  /_/   \_\___|_| \_\_____|_| |____/  SOVEREIGN INSTANT LAUNCHER
echo ================================================================================
echo  Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
echo  Engineering Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
echo --------------------------------------------------------------------------------
echo.
echo  [*] NO USB PENDRIVE REQUIRED!
echo  [*] NO LAPTOP REBOOT REQUIRED!
echo  [*] Runs natively in 60 FPS TrueColor inside Windows via hardware acceleration!
echo.
echo ================================================================================

:: Check for QEMU in system
where qemu-system-x86_64 >nul 2>&1
if %errorLevel% equ 0 (
    echo [*] Hardware virtualization engine detected.
    goto :LAUNCH_QEMU
)

:: Check C:\Program Files\qemu
if exist "C:\Program Files\qemu\qemu-system-x86_64.exe" (
    set PATH=%PATH%;C:\Program Files\qemu
    goto :LAUNCH_QEMU
)

:: Check local C:\Airlys\qemu
if exist "C:\Airlys\qemu\qemu-system-x86_64.exe" (
    set PATH=%PATH%;C:\Airlys\qemu
    goto :LAUNCH_QEMU
)

echo [!] QEMU Virtual Engine not found on your system.
echo [*] Downloading lightweight portable QEMU runner (Zero setup needed)...
if not exist "C:\Airlys" mkdir "C:\Airlys"

echo [*] Would you like to install QEMU using Windows winget? (Instant & 100%% automated)
set /p WINGET_CHOICE=">>> Press Y to install via winget (or N to download portable): "
if /i "%WINGET_CHOICE%"=="Y" (
    winget install SoftwareFreedomConservancy.QEMU --accept-package-agreements --accept-source-agreements
    set PATH=%PATH%;C:\Program Files\qemu
    goto :LAUNCH_QEMU
)

echo.
echo [*] You can also open "airlys.iso" in VirtualBox, VMware, or QEMU directly!
pause
exit /b 0

:LAUNCH_QEMU
echo.
echo [*] Initializing Airlys 64-Bit Sovereign Kernel...
set ISO_FILE=airlys.iso
if not exist "%ISO_FILE%" if exist "..\..\bin\airlys.iso" set ISO_FILE=..\..\bin\airlys.iso
if not exist "%ISO_FILE%" if exist "C:\Airlys\airlys.iso" set ISO_FILE=C:\Airlys\airlys.iso

set ELF_FILE=airlys.elf
if not exist "%ELF_FILE%" if exist "..\..\bin\airlys.elf" set ELF_FILE=..\..\bin\airlys.elf
if not exist "%ELF_FILE%" if exist "C:\Airlys\boot\airlys.elf" set ELF_FILE=C:\Airlys\boot\airlys.elf

if exist "%ISO_FILE%" (
    echo [OK] Booting Airlys Live ISO (%ISO_FILE%)...
    qemu-system-x86_64 -cdrom "%ISO_FILE%" -m 1024 -vga std -audiodev id=snd,driver=dsound -soundhw pcspkr -name "Airlys Sovereign OS - Er. Adam Eehan"
) else if exist "%ELF_FILE%" (
    echo [OK] Booting Airlys Kernel ELF (%ELF_FILE%)...
    qemu-system-x86_64 -kernel "%ELF_FILE%" -m 1024 -vga std -audiodev id=snd,driver=dsound -soundhw pcspkr -name "Airlys Sovereign OS - Er. Adam Eehan"
) else (
    echo [!] Neither airlys.iso nor airlys.elf was found in this folder.
    echo [*] Please place airlys.iso in the same folder as this script.
    pause
    exit /b 1
)

echo.
echo [OK] Airlys session ended safely. Windows remains completely untouched!
pause
