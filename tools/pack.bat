@echo off
chcp 65001 >nul
rem Сборка PBO через AddonBuilder из DayZ Tools (Windows).
rem Требуется: смонтированный диск P: и DayZ Tools в Steam.
setlocal
set "DAYZ_TOOLS=%ProgramFiles(x86)%\Steam\steamapps\common\DayZ Tools"
set "SRC=%~dp0..\GlobalChernarusRU"
set "OUT=%~dp0..\build\@GlobalChernarusRU\Addons"

if not exist "%OUT%" mkdir "%OUT%"
"%DAYZ_TOOLS%\Bin\AddonBuilder\AddonBuilder.exe" "%SRC%" "%OUT%" -clear -prefix=GlobalChernarusRU -include="%~dp0include.lst"
if errorlevel 1 (echo Сборка не удалась & exit /b 1)

copy /y "%SRC%\mod.cpp" "%~dp0..\build\@GlobalChernarusRU\" >nul
echo Готово: build\@GlobalChernarusRU
