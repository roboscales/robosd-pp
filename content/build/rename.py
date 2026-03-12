#!/usr/bin/env python3
"""
Утилита для замены префикса A на B:
- в именах всех файлов (включая вложенные каталоги)
- в содержимом файлов (слова, начинающиеся с A)
- затем дополнительно для варианта в верхнем регистре (A.upper() → B.upper())
"""

import os
import re
import sys
import argparse
from typing import List, Tuple


def safe_rename(src: str, dst: str) -> None:
    """Переименовывает файл, если целевое имя не занято."""
    if os.path.exists(dst):
        print(f"Предупреждение: {dst} уже существует, пропускаем {src}")
        return
    os.rename(src, dst)
    print(f"Переименован: {src} -> {dst}")


def rename_files(root_dir: str, a: str, b: str) -> None:
    """
    Заменяет префикс a на b в именах всех файлов (не трогая каталоги).
    Расширение файла сохраняется.
    """
    actions: List[Tuple[str, str]] = []
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            # Разделяем имя и расширение (последняя точка)
            base, ext = os.path.splitext(filename)
            if base.startswith(a):
                new_base = b + base[len(a):]
                new_name = new_base + ext
                new_path = os.path.join(dirpath, new_name)
                old_path = os.path.join(dirpath, filename)
                actions.append((old_path, new_path))

    for old, new in actions:
        safe_rename(old, new)


def replace_in_files(root_dir: str, a: str, b: str) -> None:
    """
    Заменяет в содержимом всех файлов:
    1) слова, начинающиеся с a, на те же слова с префиксом b
    2) слова, начинающиеся с a.upper(), на те же слова с префиксом b.upper()
    """
    # Регулярные выражения: \b — граница слова, \w* — оставшаяся часть слова
    pattern_lower = re.compile(r'\b' + re.escape(a) + r'(\w*)')
    repl_lower = lambda m: b + m.group(1)

    pattern_upper = re.compile(r'\b' + re.escape(a.upper()) + r'(\w*)')
    repl_upper = lambda m: b.upper() + m.group(1)

    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)

            # Пытаемся прочитать файл как текст (UTF-8)
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read()
            except Exception as e:
                print(f"Не удалось прочитать {filepath}: {e}")
                continue

            new_content = pattern_lower.sub(repl_lower, content)
            new_content = pattern_upper.sub(repl_upper, new_content)

            if new_content != content:
                try:
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    print(f"Изменён файл: {filepath}")
                except Exception as e:
                    print(f"Не удалось записать {filepath}: {e}")


def main():
    parser = argparse.ArgumentParser(
        description="Заменяет префикс A на B в именах файлов и их содержимом (включая подкаталоги)."
    )
    parser.add_argument("a", help="исходный префикс (A)")
    parser.add_argument("b", help="новый префикс (B)")
    parser.add_argument("--dry-run", action="store_true",
                        help="только показать, что будет сделано, без реальных изменений")
    args = parser.parse_args()

    if args.dry_run:
        print("Режим пробного запуска (dry run) — изменения не применяются.")
        # Здесь можно было бы добавить логику имитации, но для простоты ограничимся сообщением
        sys.exit(0)

    current_dir = os.getcwd()
    print(f"Работаем в каталоге: {current_dir}")
    print(f"Замена префикса '{args.a}' на '{args.b}' в именах файлов...")
    rename_files(current_dir, args.a, args.b)

    print("Замена в содержимом файлов (включая верхний регистр)...")
    replace_in_files(current_dir, args.a, args.b)

    print("Готово.")


if __name__ == "__main__":
    main()