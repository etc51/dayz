@echo off
chcp 65001 >nul
rem Загрузка локальной папки проекта в GitHub (ветка local-import).
rem Положите этот файл в папку проекта и запустите двойным щелчком.
rem Файлы в папке не меняются и не удаляются: только git init + коммит + push.
rem Нужен Git for Windows: https://git-scm.com/download/win
setlocal
cd /d "%~dp0"

where git >nul 2>&1 || (echo Git не установлен: https://git-scm.com/download/win & pause & exit /b 1)

if not exist ".git" git init -b local-import
git remote get-url origin >nul 2>&1 || git remote add origin https://github.com/etc51/dayz.git

if not exist ".gitignore" (
  (
    echo *.pbo
    echo *.bisign
    echo *.biprivatekey
    echo *.pem
    echo .env
    echo *cookie*
    echo *.zip
    echo *.7z
    echo *.rar
    echo *.log
    echo *.rpt
    echo *.mdmp
    echo *.ADM
    echo node_modules/
    echo payload/
    echo world-signed/
    echo profiles/
    echo storage_*/
  ) > .gitignore
)

git checkout -B local-import
git add -A
echo.
echo Что будет загружено (проверьте, что нет паролей, ключей и cookies):
git status --short
echo.
pause
git -c user.name=etc51 -c user.email=etc51@users.noreply.github.com commit -m "Импорт локального проекта Глобальная Черноруссия [RU]"
git push -u origin local-import
echo.
echo Готово. Напишите в облачный тред: готово, ветка local-import.
pause
