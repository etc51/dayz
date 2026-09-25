#!/usr/bin/env bash
# Автосинхронизация проекта с GitHub (ПК и облако).
# commit всех изменений -> merge с origin -> push. Секреты не отправляются.
# Запускается хуками Claude Code (.claude/settings.json) и планировщиком Windows.
# Флаг --pull-only: только подтянуть изменения (при старте сессии).

cd "$(git rev-parse --show-toplevel 2>/dev/null)" || exit 0
branch=$(git symbolic-ref --short HEAD 2>/dev/null) || exit 0

# Одновременно работает только один экземпляр (lock с защитой от «зависшего»).
lock=.git/gc-sync.lock
if ! mkdir "$lock" 2>/dev/null; then
  if [ -n "$(find "$lock" -maxdepth 0 -mmin +10 2>/dev/null)" ]; then
    rmdir "$lock" 2>/dev/null; mkdir "$lock" 2>/dev/null || exit 0
  else
    exit 0
  fi
fi
trap 'rmdir "$lock" 2>/dev/null' EXIT

say() { echo "[gc-sync] $*"; }

merge_from() { # $1 = удалённая ветка
  git rev-parse -q --verify "refs/remotes/origin/$1" >/dev/null || return 0
  if ! git merge -q --no-edit "origin/$1" >/dev/null 2>&1; then
    git merge --abort 2>/dev/null
    say "КОНФЛИКТ при объединении с origin/$1 — синхронизация остановлена, нужна ручная помощь."
    exit 0
  fi
}

commit_local() {
  git add -A
  # Файлы, которые нельзя отправлять ни при каких условиях.
  blocked=$(git diff --cached --name-only | grep -iE '\.(biprivatekey|pem|p12|pfx)$|(^|/)\.env|cookie|secret|(^|/)serverDZ\.cfg$' || true)
  # Содержимое: закрытые ключи и непустые пароли.
  for f in $(git diff --cached --name-only --diff-filter=AM); do
    [ -f "$f" ] || continue
    if [ "$(wc -c <"$f")" -gt 52428800 ]; then blocked="$blocked"$'\n'"$f"; continue; fi
    if grep -IqE 'BEGIN ([A-Z]+ )?PRIVATE KEY|password(Admin)?[[:space:]]*=[[:space:]]*"[^"]+"|sk-ant-[A-Za-z0-9_-]{10,}|ghp_[A-Za-z0-9]{20,}' "$f" 2>/dev/null; then
      blocked="$blocked"$'\n'"$f"
    fi
  done
  blocked=$(echo "$blocked" | sed '/^$/d' | sort -u)
  if [ -n "$blocked" ]; then
    echo "$blocked" | while read -r f; do
      git reset -q -- "$f"
      grep -qxF "/$f" .git/info/exclude 2>/dev/null || echo "/$f" >> .git/info/exclude
    done
    say "НЕ отправлено (секрет или >50 МБ): $(echo "$blocked" | tr '\n' ' ')"
  fi
  if ! git diff --cached --quiet; then
    git commit -q -m "Автосинхронизация: $(hostname) $(date '+%Y-%m-%d %H:%M')" && say "изменения закоммичены"
  fi
}

git fetch -q origin 2>/dev/null || { say "нет связи с GitHub"; exit 0; }

if [ "$1" != "--pull-only" ]; then commit_local; fi

merge_from "$branch"
[ "$branch" != "main" ] && merge_from main

if [ "$1" != "--pull-only" ]; then
  if [ -n "$(git log "origin/$branch..HEAD" --oneline 2>/dev/null)" ] || ! git rev-parse -q --verify "refs/remotes/origin/$branch" >/dev/null; then
    git push -q origin "HEAD:$branch" 2>/dev/null && say "отправлено в origin/$branch" || say "push не удался"
  fi
fi
exit 0
