@echo off
rem Один проход синхронизации (вызывается планировщиком через autosync-hidden.vbs).
cd /d "%~dp0.."
if exist "%ProgramFiles%\Git\bin\bash.exe" (
  "%ProgramFiles%\Git\bin\bash.exe" tools/sync.sh >> "%~dp0..\.git\gc-sync.log" 2>&1
) else (
  bash tools/sync.sh >> "%~dp0..\.git\gc-sync.log" 2>&1
)
