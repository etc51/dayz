@echo off
chcp 65001 >nul
rem Включает фоновую синхронизацию с GitHub каждые 5 минут (Планировщик заданий Windows).
rem Отключить: schtasks /Delete /TN "GC AutoSync" /F
schtasks /Create /SC MINUTE /MO 5 /TN "GC AutoSync" /TR "wscript.exe \"%~dp0autosync-hidden.vbs\"" /F
if errorlevel 1 (echo Не удалось создать задание. & pause & exit /b 1)
echo Готово: синхронизация каждые 5 минут. Журнал: .git\gc-sync.log
call "%~dp0autosync-run.cmd"
pause
