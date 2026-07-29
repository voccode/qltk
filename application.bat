@echo off
setlocal
cd /d "%~dp0"
set "APP_DIR=%~dp0data"
set "TCL_LIBRARY=%APP_DIR%\tcl_runtime\tcl8.6"
set "TK_LIBRARY=%APP_DIR%\tcl_runtime\tk8.6"

if exist "%APP_DIR%\runtime\pythonw.exe" (
    start "" /normal "%APP_DIR%\runtime\pythonw.exe" "%APP_DIR%\app_gui.py"
    exit /b 0
)
if exist "%APP_DIR%\.venv\Scripts\pythonw.exe" (
    start "" /normal "%APP_DIR%\.venv\Scripts\pythonw.exe" "%APP_DIR%\app_gui.py"
    exit /b 0
)
for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do (
    if exist "%%~fD\pythonw.exe" (
        start "" /normal "%%~fD\pythonw.exe" "%APP_DIR%\app_gui.py"
        exit /b 0
    )
)
where pythonw.exe >nul 2>nul
if not errorlevel 1 (
    start "" /normal pythonw.exe "%APP_DIR%\app_gui.py"
    exit /b 0
)
where pyw.exe >nul 2>nul
if not errorlevel 1 (
    start "" /normal pyw.exe -3 "%APP_DIR%\app_gui.py"
    exit /b 0
)

echo Khong tim thay Python. Hay dat Python portable trong thu muc data\runtime.
pause
exit /b 0