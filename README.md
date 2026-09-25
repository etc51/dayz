# Глобальная Черноруссия (RU-мод) для DayZ

Мод с русификацией и расширением карты Черноруссия (`chernarusplus`):
локализация (stringtable), скрипты на Enforce Script, серверные настройки.

## Структура

```
GlobalChernarusRU/           исходники мода (монтируются в P:\GlobalChernarusRU)
  config.cpp                 CfgPatches / CfgMods, подключение скриптовых модулей
  mod.cpp                    метаданные для лаунчера
  stringtable.csv            переводы (ключи STR_GCRU_*)
  Scripts/3_Game/GCRU        константы, утилиты
  Scripts/4_World/GCRU       modded-классы мира (PlayerBase и т.д.)
  Scripts/5_Mission/GCRU     MissionGameplay / MissionServer
server/                      пример serverDZ.cfg и запуск тестового сервера
tools/
  validate.py                проверки без DayZ Tools (облако/CI)
  pack.bat                   сборка PBO через AddonBuilder (Windows)
  link_p_drive.bat           junction P:\GlobalChernarusRU -> репозиторий
```

## Работа локально (Windows)

1. `git clone https://github.com/etc51/dayz.git`
2. Установите DayZ Tools (Steam), смонтируйте диск P: (Workdrive).
3. От администратора: `tools\link_p_drive.bat`.
4. Сборка: `tools\pack.bat` → `build\@GlobalChernarusRU`.
5. Сервер: скопируйте `server\serverDZ.cfg.example` в `server\serverDZ.cfg`, затем `server\start_server.bat`.
6. Клиент: запуск DayZ с `-mod=<путь>\build\@GlobalChernarusRU`.
7. Перед коммитом: `python tools\validate.py`.

## Работа в облаке (Claude Code)

В облаке нет DayZ/Workbench, поэтому там правим скрипты, конфиги, stringtable,
XML-файлы экономики и проверяем их `python3 tools/validate.py` (эти же проверки
запускает GitHub Actions). Бинарную часть (модели, текстуры, сборка PBO,
тесты в игре) делаем локально.

## Ветки и синхронизация

- `main` — стабильная версия.
- Облачные сессии работают в ветках `claude/*` и открывают PR в `main`.
- Локально: `git pull` перед работой, `git push` после — так облако и ПК
  видят одни и те же изменения.

## Соглашения

- Все свои классы/ключи — с префиксом `GCRU` / `STR_GCRU_`.
- Ванильные классы расширяем только через `modded class` и `super.*`.
- Файлы — UTF-8, `stringtable.csv` — CRLF (см. `.gitattributes`).
