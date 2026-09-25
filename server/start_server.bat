@echo off
chcp 65001 >nul
rem Локальный тестовый сервер с модом. Скопируйте serverDZ.cfg.example -> serverDZ.cfg.
set "DAYZ_SERVER=%ProgramFiles(x86)%\Steam\steamapps\common\DayZServer"
set "MOD=%~dp0..\build\@GlobalChernarusRU"
cd /d "%DAYZ_SERVER%"
start "" DayZServer_x64.exe -config="%~dp0serverDZ.cfg" -port=2302 -profiles="%~dp0profiles" -mod="%MOD%" -dologs -adminlog -netlog -freezecheck
