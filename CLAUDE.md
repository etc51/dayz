# CLAUDE.md

DayZ-мод «Глобальная Черноруссия (RU)». Общение с владельцем — на русском.

- Исходники мода: `GlobalChernarusRU/` (Enforce Script, config.cpp, stringtable.csv).
- Проверка после любых правок: `python3 tools/validate.py` (её же гоняет CI).
- DayZ Tools в облаке недоступны: не пытаться собирать PBO, только правка текста.
- Префиксы: классы `GCRU_*`, ключи строк `STR_GCRU_*`; ванилу меняем через `modded class` с вызовом `super`.
- Новые строки интерфейса добавлять в `stringtable.csv` (все 14 языковых колонок, русская обязательна).
- Изменения фиксировать в `CHANGELOG.md`.
