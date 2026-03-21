#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Скрипт для отображения структуры каталогов в виде дерева.
Использование: python tree.py [путь] [опции]
"""

import os
import sys
import argparse

def build_tree(path, prefix="", show_hidden=False, only_dirs=False, max_depth=None, current_depth=0):
    """
    Рекурсивно строит список строк, представляющих дерево каталогов.
    
    Аргументы:
        path        – путь к текущему каталогу
        prefix      – отступ для текущего уровня (строки "│   " или "    ")
        show_hidden – показывать ли скрытые файлы/папки (начинающиеся с точки)
        only_dirs   – выводить только директории (пропускать файлы)
        max_depth   – максимальная глубина обхода (None – без ограничений)
        current_depth – текущая глубина (используется для проверки max_depth)
    
    Возвращает:
        Список строк для текущей ветки дерева.
    """
    if max_depth is not None and current_depth > max_depth:
        return []
    
    try:
        entries = os.listdir(path)
    except PermissionError:
        return [prefix + "└── [Permission Denied]"]
    
    # Фильтр скрытых файлов
    if not show_hidden:
        entries = [e for e in entries if not e.startswith('.')]
    
    # Сортировка: папки идут первыми, затем файлы (в алфавитном порядке)
    dirs = []
    files = []
    for e in entries:
        full = os.path.join(path, e)
        if os.path.isdir(full) and not os.path.islink(full):
            dirs.append(e)
        else:
            files.append(e)
    dirs.sort()
    files.sort()
    sorted_entries = dirs + files

    lines = []
    for i, entry in enumerate(sorted_entries):
        is_last = i == len(sorted_entries) - 1
        full_path = os.path.join(path, entry)
        
        # Для only_dirs пропускаем файлы
        if only_dirs and not os.path.isdir(full_path):
            continue
        
        # Выбор соединительной линии
        connector = "└── " if is_last else "├── "
        
        # Если это символическая ссылка, покажем куда она ведёт
        if os.path.islink(full_path):
            link_target = os.readlink(full_path)
            lines.append(prefix + connector + entry + " -> " + link_target)
            continue
        
        lines.append(prefix + connector + entry)
        
        # Если элемент – директория (и не ссылка), рекурсивно обходим её
        if os.path.isdir(full_path) and not os.path.islink(full_path):
            extension = "    " if is_last else "│   "
            sub_lines = build_tree(full_path, prefix + extension,
                                   show_hidden, only_dirs, max_depth, current_depth + 1)
            lines.extend(sub_lines)
    
    return lines

def main():
    parser = argparse.ArgumentParser(
        description="Отображение структуры каталогов в виде дерева."
    )
    parser.add_argument(
        "path", nargs="?", default=".",
        help="Путь к начальному каталогу (по умолчанию текущий)"
    )
    parser.add_argument(
        "-a", "--all", action="store_true",
        help="Показывать скрытые файлы и папки (начинающиеся с точки)"
    )
    parser.add_argument(
        "-d", "--dirs-only", action="store_true",
        help="Показывать только директории, без файлов"
    )
    parser.add_argument(
        "-L", "--max-depth", type=int,
        help="Максимальная глубина обхода (по умолчанию без ограничений)"
    )
    parser.add_argument(
        "-o", "--output",
        help="Сохранить вывод в указанный файл (иначе печатается в консоль)"
    )
    
    args = parser.parse_args()
    
    # Проверка существования пути
    root = os.path.abspath(args.path)
    if not os.path.isdir(root):
        print(f"Ошибка: '{root}' не является директорией или не существует.",
              file=sys.stderr)
        sys.exit(1)
    
    # Заголовок дерева – имя корневой папки (или '.' если текущая)
    if root == os.path.abspath('.'):
        header = '.'
    else:
        header = os.path.basename(root)
    
    # Построение дерева
    tree_lines = build_tree(
        root,
        show_hidden=args.all,
        only_dirs=args.dirs_only,
        max_depth=args.max_depth
    )
    output_lines = [header] + tree_lines
    output = "\n".join(output_lines)
    
    # Вывод или сохранение
    if args.output:
        try:
            with open(args.output, "w", encoding="utf-8") as f:
                f.write(output)
        except IOError as e:
            print(f"Ошибка записи в файл: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        print(output)

if __name__ == "__main__":
    main()