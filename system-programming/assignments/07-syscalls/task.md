# Задание 07: Мини-трассировщик через ptrace

## Цель

Реализовать простой трассировщик системных вызовов (упрощённый аналог `strace`) с использованием `ptrace`.

## Задача

Реализовать программу `mytrace`, которая:

```
mytrace <программа> [аргументы...]
```

### Функциональность

1. **Запуск трассируемой программы** через `fork()` + `execvp()`

2. **Перехват каждого системного вызова** через `PTRACE_SYSCALL`

3. **Вывод информации** о каждом вызове:
   ```
   [pid] syscall_name(arg0, arg1, arg2) = return_value
   ```

4. **Поддержать вывод имён** хотя бы для этих syscall:
   - `read`, `write`, `open`, `openat`, `close`
   - `mmap`, `munmap`, `brk`
   - `fork`, `execve`, `exit`, `exit_group`
   - Для остальных вывести номер: `syscall(42)`

5. **Статистика в конце**: сколько раз каждый syscall был вызван

### Пример вывода

```
$ ./mytrace /bin/echo hello
[12346] execve("/bin/echo", ...) = 0
[12346] brk(NULL) = 0x55f...
[12346] openat(AT_FDCWD, "/etc/ld.so.cache", ...) = 3
[12346] mmap(NULL, 12345, ...) = 0x7f...
[12346] close(3) = 0
[12346] write(1, "hello\n", 6) = 6
[12346] exit_group(0) = ?
hello

--- Статистика ---
write:      1
openat:     3
mmap:       4
close:      3
exit_group: 1
```

## Требования

- Использовать `ptrace(PTRACE_TRACEME)`, `ptrace(PTRACE_SYSCALL)`, `ptrace(PTRACE_GETREGS)`
- Корректно обрабатывать завершение трассируемого процесса
- Считать отдельно вход в syscall и выход (для получения возвращаемого значения)

## Проверка

```bash
gcc -Wall -Wextra -o solution solution.c
./solution /bin/ls /tmp
./solution /bin/echo "test"
./solution /bin/cat /etc/hostname
```

## Подсказка

На x86-64: при входе в syscall `regs.orig_rax` = номер вызова, `regs.rdi/rsi/rdx` = аргументы.
При выходе из syscall `regs.rax` = возвращаемое значение.
Чтобы различить вход и выход — использовать счётчик: чётный вызов `waitpid` = вход, нечётный = выход.
