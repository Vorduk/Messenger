#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Скрипт для вывода структуры всех папок и файлов директории, из которой он запущен,
в текстовый файл (по умолчанию 'directory_structure.txt').
"""

import sys
from pathlib import Path

def walk_dir(path: Path, file_handle, prefix: str = ''):
    """
    Рекурсивно обходит директорию и записывает её структуру в файл.

    :param path: Путь к текущей директории.
    :param file_handle: Открытый файловый объект для записи.
    :param prefix: Строка-префикс для отображения уровня вложенности.
    """
    try:
        # Получаем отсортированный список содержимого: сначала папки, затем файлы
        items = sorted(path.iterdir(), key=lambda p: (not p.is_dir(), p.name.lower()))
    except PermissionError:
        # Если нет доступа к директории, записываем предупреждение и выходим
        file_handle.write(prefix + "└── [Permission denied]\n")
        return

    # Обходим все элементы в директории
    for i, item in enumerate(items):
        # Пропускаем сам файл вывода, если он находится внутри обходимой директории
        if output_path.absolute() == item.absolute():
            continue

        is_last = (i == len(items) - 1)          # Последний ли элемент в текущей папке?
        connector = '└── ' if is_last else '├── '
        # Добавляем слеш к именам папок для наглядности
        suffix = '/' if item.is_dir() else ''
        file_handle.write(prefix + connector + item.name + suffix + '\n')

        # Если это директория, рекурсивно обходим её
        if item.is_dir():
            extension = '    ' if is_last else '│   '
            walk_dir(item, file_handle, prefix + extension)


def main():
    # Определяем выходной файл (можно переопределить через аргумент командной строки)
    global output_path
    output_filename = sys.argv[1] if len(sys.argv) > 1 else 'directory_structure.txt'
    output_path = Path.cwd() / output_filename

    # Запрашиваемая директория – та, из которой запущен скрипт
    start_path = Path.cwd()

    # Открываем выходной файл для записи (кодировка UTF-8)
    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(f"Directory structure of: {start_path}\n\n")
            walk_dir(start_path, f)
        print(f"Структура успешно сохранена в файл: {output_path}")
    except Exception as e:
        print(f"Ошибка при записи в файл: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()