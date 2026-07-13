# Лекция 01: Введение в системное программирование

## Что такое системное программирование?

Системное программирование — это разработка программного обеспечения, которое управляет аппаратными ресурсами компьютера или предоставляет сервисы для другого программного обеспечения. В отличие от прикладного программирования, системный программист работает близко к железу: с памятью, процессором, файловой системой и сетевым стеком напрямую.

Примеры системного ПО:
- Операционные системы (Linux, FreeBSD)
- Компиляторы (GCC, Clang)
- Драйверы устройств
- Стандартная библиотека C (glibc)
- Контейнерные рантаймы (runc, containerd)

## Уровни абстракции

```
+---------------------------+
|   Пользовательское ПО    |
+---------------------------+
|   Стандартная библиотека  |  <-- printf, malloc, fopen
+---------------------------+
|   Системные вызовы        |  <-- write, brk, open
+---------------------------+
|   Ядро Linux              |
+---------------------------+
|   Аппаратное обеспечение  |
+---------------------------+
```

Системный программист работает на уровне системных вызовов и стандартной библиотеки C.

## Инструменты

### Компилятор GCC

```bash
# Компиляция простой программы
gcc -o hello hello.c

# Компиляция с предупреждениями и отладочной информацией
gcc -Wall -Wextra -g -o hello hello.c

# Компиляция с оптимизацией
gcc -O2 -o hello hello.c
```

### Отладчик GDB

```bash
# Запуск отладчика
gdb ./hello

# Основные команды GDB
(gdb) break main       # точка останова на функции main
(gdb) run              # запустить программу
(gdb) next             # следующая строка (не заходить в функцию)
(gdb) step             # следующая строка (заходить в функцию)
(gdb) print var        # напечатать значение переменной
(gdb) backtrace        # стек вызовов
(gdb) quit             # выйти
```

### strace — трассировка системных вызовов

```bash
# Показать все системные вызовы программы
strace ./hello

# Только вызовы read/write
strace -e trace=read,write ./hello

# Статистика вызовов
strace -c ./hello
```

### Просмотр ELF-файла

```bash
# Заголовок ELF
readelf -h hello

# Список секций
readelf -S hello

# Таблица символов
nm hello

# Зависимости от разделяемых библиотек
ldd hello
```

## Первая программа: прямой системный вызов

Стандартная функция `printf` внутри использует системный вызов `write`. Напишем программу без `printf`:

```c
#include <unistd.h>

int main(void)
{
    const char msg[] = "Hello, System Programming!\n";
    /* write(fd, buf, count)
     * fd=1 — стандартный вывод (stdout) */
    write(1, msg, sizeof(msg) - 1);
    return 0;
}
```

Теперь то же самое через ассемблерную вставку (x86-64 Linux):

```c
#include <stddef.h>

static void sys_write(int fd, const char *buf, size_t count)
{
    /* syscall номер 1 = write на x86-64 */
    __asm__ volatile (
        "syscall"
        :
        : "a"(1),        /* rax = номер syscall */
          "D"((long)fd), /* rdi = файловый дескриптор */
          "S"(buf),      /* rsi = указатель на буфер */
          "d"(count)     /* rdx = количество байт */
        : "rcx", "r11", "memory"
    );
}

int main(void)
{
    const char msg[] = "Hello via raw syscall!\n";
    sys_write(1, msg, sizeof(msg) - 1);
    return 0;
}
```

## Числа файловых дескрипторов

| Дескриптор | Имя    | Макрос   | Назначение       |
|-----------|--------|----------|-----------------|
| 0         | stdin  | STDIN_FILENO  | Стандартный ввод  |
| 1         | stdout | STDOUT_FILENO | Стандартный вывод |
| 2         | stderr | STDERR_FILENO | Стандартные ошибки|

## Коды возврата и errno

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main(void)
{
    int fd = open("/nonexistent/file", O_RDONLY);
    if (fd == -1) {
        /* errno содержит код ошибки */
        fprintf(stderr, "open: %s (errno=%d)\n", strerror(errno), errno);
        return 1;
    }
    return 0;
}
```

Вывод: `open: No such file or directory (errno=2)`

## Полезные макросы для отладки

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/* Вывести ошибку и завершить программу */
#define DIE(msg)                                                      \
    do {                                                              \
        fprintf(stderr, "%s:%d: %s: %s\n",                          \
                __FILE__, __LINE__, (msg), strerror(errno));         \
        exit(1);                                                      \
    } while (0)

/* Вывести ошибку без завершения */
#define WARN(msg)                                                     \
    fprintf(stderr, "%s:%d: %s: %s\n",                               \
            __FILE__, __LINE__, (msg), strerror(errno))
```

## Итог

- Системное программирование работает близко к ядру ОС
- Основной язык — C, инструменты — GCC, GDB, strace
- Системные вызовы — интерфейс между программой и ядром
- Каждый системный вызов при ошибке возвращает -1 и устанавливает `errno`

## Дополнительное чтение

- `man 2 intro` — введение в системные вызовы
- `man 3 intro` — введение в библиотечные функции
- The Linux Programming Interface (Kerrisk)
