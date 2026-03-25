@echo off
setlocal enabledelayedexpansion

set "FOLDER_A=%~dp0bin"
set "FOLDER_B=%~dp0output\bin"
set /a TOTAL_FILES=0
set /a SAME_FILES=0
set /a DIFF_FILES=0
set /a MISSING_FILES=0

cd /d "%FOLDER_A%"


for %%f in (*.*) do (
    set /a TOTAL_FILES+=1
    set "FILE_NAME=%%f"
    
    echo [!TOTAL_FILES!] checking: %%f
    if exist "%FOLDER_B%\%%f" (

        fc /b "%%f" "%FOLDER_B%\%%f" >nul
        
        if errorlevel 1 (
            echo     Mismatched!
            set /a DIFF_FILES+=1
        ) else (
            echo     Matched!
            set /a SAME_FILES+=1
        )
    ) else (
        echo     Missing!
        set /a MISSING_FILES+=1
    )
)

echo.
echo ========================================
echo Done!
echo Total: %TOTAL_FILES%
echo Match: %SAME_FILES%
echo Mismatch %DIFF_FILES%
echo Missing: %MISSING_FILES%
echo ========================================

pause