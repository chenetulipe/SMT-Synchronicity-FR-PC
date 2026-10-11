@echo off
chcp 65001 >nul
title SMT Synchronicity FR - Patcher

cd /d "%~dp0"

echo.
echo  ============================================
echo     SMT Synchronicity FR - Web Patcher
echo  ============================================
echo.

:: Detection de Python ou py
set "PY_CMD=python"
python --version >nul 2>&1
if not errorlevel 1 goto :found_python

py --version >nul 2>&1
if not errorlevel 1 (
    set "PY_CMD=py"
    goto :found_python
)

echo  [ERREUR] Python 3 est introuvable sur votre systeme.
echo  Veuillez telecharger et installer Python depuis :
echo  https://www.python.org/downloads/
echo.
echo  IMPORTANT : Cochez bien la case "Add Python to PATH" lors de l'installation.
echo.
pause
exit /b 1

:found_python
:: Verification des dependances requises
%PY_CMD% -c "from PIL import Image" >nul 2>&1
if errorlevel 1 (
    echo  [INFO] Installation automatique de Pillow...
    %PY_CMD% -m pip install pillow --quiet
)

%PY_CMD% -c "import bsdiff4" >nul 2>&1
if errorlevel 1 (
    echo  [INFO] Installation automatique de bsdiff4...
    %PY_CMD% -m pip install bsdiff4 --quiet
)

:: Demarrage du patcher
echo.
echo  Lancement du patcher web...
echo  Une page s'ouvre automatiquement dans votre navigateur.
echo  (Laissez cette fenetre ouverte pendant l'utilisation)
echo.
%PY_CMD% server.py
if errorlevel 1 (
    echo.
    echo  [ERREUR] Une erreur est survenue lors de l'execution du patcher.
    pause
)
