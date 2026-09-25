#!/usr/bin/env python3
"""Проверки мода, которые можно запускать без DayZ Tools (в облаке и в CI).

- stringtable.csv: заголовок, число колонок, уникальность ключей, наличие русского перевода;
- скрипты .c: баланс скобок, кодировка UTF-8;
- config.cpp: баланс фигурных скобок, пути к скриптовым модулям существуют.
"""
import csv
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
MOD = ROOT / "GlobalChernarusRU"
errors: list[str] = []


def err(msg: str) -> None:
    errors.append(msg)


def check_stringtable() -> None:
    path = MOD / "stringtable.csv"
    with path.open(encoding="utf-8", newline="") as f:
        rows = list(csv.reader(f))
    header = [c for c in rows[0] if c]
    if header[:2] != ["Language", "original"] or "russian" not in header:
        err(f"{path.name}: неверный заголовок")
        return
    ru = header.index("russian")
    seen = set()
    for n, row in enumerate(rows[1:], start=2):
        cells = row[:-1] if row and row[-1] == "" else row
        if len(cells) != len(header):
            err(f"{path.name}:{n}: {len(cells)} колонок вместо {len(header)}")
            continue
        key = cells[0]
        if not key.startswith("STR_GCRU_"):
            err(f"{path.name}:{n}: ключ {key!r} должен начинаться с STR_GCRU_")
        if key in seen:
            err(f"{path.name}:{n}: дубликат ключа {key}")
        seen.add(key)
        if not re.search("[А-Яа-яЁё]", cells[ru]):
            err(f"{path.name}:{n}: нет русского перевода для {key}")


def check_braces(path: pathlib.Path, text: str) -> None:
    # Убираем строки и комментарии перед подсчётом.
    stripped = re.sub(r'"(\\.|[^"\\])*"', '""', text)
    stripped = re.sub(r"//[^\n]*|/\*.*?\*/", "", stripped, flags=re.S)
    for o, c in ("{}", "()", "[]"):
        if stripped.count(o) != stripped.count(c):
            err(f"{path.relative_to(ROOT)}: несбалансированы скобки {o}{c}")


def check_scripts() -> None:
    for path in sorted((MOD / "Scripts").rglob("*.c")):
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            err(f"{path.relative_to(ROOT)}: файл не в UTF-8")
            continue
        check_braces(path, text)


def check_config() -> None:
    path = MOD / "config.cpp"
    text = path.read_text(encoding="utf-8")
    check_braces(path, text)
    for mod_path in re.findall(r'files\[\]\s*=\s*\{\s*"([^"]+)"', text):
        if not (ROOT / mod_path).is_dir():
            err(f"config.cpp: путь скриптового модуля не найден: {mod_path}")


def main() -> int:
    check_stringtable()
    check_scripts()
    check_config()
    for e in errors:
        print("ОШИБКА:", e)
    if errors:
        return 1
    print("OK: все проверки пройдены")
    return 0


if __name__ == "__main__":
    sys.exit(main())
