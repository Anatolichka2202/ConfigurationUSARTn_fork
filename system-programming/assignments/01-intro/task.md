# Задание 01: Hello, World через системный вызов

## Цель

Написать программу на C, которая выводит `"Hello, System Programming!"` в stdout **без использования `printf` и `puts`** — только через системный вызов `write`.

## Задача

1. Написать функцию `sys_write(int fd, const char *buf, size_t count)`, которая вызывает `write` напрямую через:
   - Вариант A: POSIX-функцию `write()` из `<unistd.h>`
   - Вариант B (усложнённый): встроенный ассемблер (`__asm__ volatile`)

2. Вывести сообщение `"Hello, System Programming!\n"` в stdout (fd=1).

3. Вывести сообщение `"Error: something went wrong\n"` в stderr (fd=2).

4. Завершить процесс с кодом 0.

## Требования

- Запрещено использовать `printf`, `puts`, `fputs`, `fprintf`
- Запрещено подключать `<stdio.h>`
- Разрешено использовать `<unistd.h>` и `<string.h>` (только `strlen`)

## Проверка

```bash
gcc -Wall -Wextra -o solution solution.c
./solution
echo "Exit code: $?"
```

Ожидаемый вывод:
```
Hello, System Programming!
```

В stderr:
```
Error: something went wrong
```

Exit code: `0`

## Подсказка

Системный вызов `write` в Linux:
- fd=0: stdin, fd=1: stdout, fd=2: stderr
- Возвращает количество записанных байт или -1 при ошибке
