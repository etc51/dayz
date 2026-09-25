@echo off
chcp 65001 >nul
rem Создаёт ссылку P:\GlobalChernarusRU -> папка мода в репозитории,
rem чтобы Workbench и AddonBuilder видели актуальные исходники. Запускать от администратора.
mklink /J "P:\GlobalChernarusRU" "%~dp0..\GlobalChernarusRU"
