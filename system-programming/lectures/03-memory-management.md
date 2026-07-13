# Лекция 03: Управление памятью

## Виртуальная память

Linux использует виртуальную память: каждый процесс видит «свой» непрерывный адресный диапазон, а ядро отображает виртуальные адреса на физические страницы через таблицы страниц (page tables).

Страница памяти — минимальная единица выделения (обычно 4 КБ).

```bash
# Карта памяти процесса
cat /proc/self/maps

# Или через утилиту
pmap $$
```

## Стек и куча

### Стек (Stack)

- Выделяется автоматически при вызове функции
- Освобождается при возврате из функции
- Ограничен по размеру (обычно 8 МБ)
- Работает через регистр `rsp` (x86-64)

```c
void foo(void)
{
    int x = 42;          /* x — на стеке */
    char buf[1024];      /* buf — 1 КБ на стеке */
    /* при выходе из foo всё автоматически освобождается */
}
```

### Куча (Heap)

- Выделяется явно через `malloc`/`calloc`/`realloc`
- Освобождается явно через `free`
- Размер ограничен только доступной памятью

```c
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    /* malloc — не инициализирует память */
    int *arr = malloc(10 * sizeof(int));
    if (!arr) {
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < 10; i++)
        arr[i] = i * i;

    /* calloc — выделяет и обнуляет */
    int *zeros = calloc(10, sizeof(int));

    /* realloc — изменяет размер блока */
    arr = realloc(arr, 20 * sizeof(int));

    free(arr);
    free(zeros);
    return 0;
}
```

## Системные вызовы управления памятью

```c
#include <unistd.h>
#include <sys/mman.h>

/* brk/sbrk — изменить границу кучи (низкий уровень) */
void *old_break = sbrk(0);   /* текущая граница */
sbrk(4096);                   /* увеличить на 4 КБ */
sbrk(-4096);                  /* уменьшить обратно */

/* mmap — отобразить память */
void *mem = mmap(NULL, 4096,
                 PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS,
                 -1, 0);

/* munmap — освободить отображение */
munmap(mem, 4096);
```

## Простой аллокатор памяти

```c
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

/* Заголовок блока памяти */
typedef struct Block {
    size_t        size;   /* размер полезных данных */
    int           free;   /* 1 = свободен */
    struct Block *next;
} Block;

#define BLOCK_META sizeof(Block)

static Block *heap_start = NULL;

/* Найти подходящий свободный блок */
static Block *find_free(Block **last, size_t size)
{
    Block *cur = heap_start;
    while (cur && !(cur->free && cur->size >= size)) {
        *last = cur;
        cur = cur->next;
    }
    return cur;
}

/* Запросить у ОС новый блок через sbrk */
static Block *request_space(Block *last, size_t size)
{
    Block *block = sbrk(0);
    if (sbrk(BLOCK_META + size) == (void *)-1)
        return NULL;
    block->size = size;
    block->free = 0;
    block->next = NULL;
    if (last)
        last->next = block;
    return block;
}

void *my_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    Block *last = NULL;
    Block *block;

    if (heap_start) {
        block = find_free(&last, size);
        if (block) {
            block->free = 0;
        } else {
            block = request_space(last, size);
            if (!block) return NULL;
        }
    } else {
        block = request_space(NULL, size);
        if (!block) return NULL;
        heap_start = block;
    }
    return (void *)(block + 1);  /* указатель за заголовком */
}

void my_free(void *ptr)
{
    if (!ptr) return;
    Block *block = (Block *)ptr - 1;
    block->free = 1;
}

int main(void)
{
    int *a = my_malloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++)
        a[i] = i + 1;

    printf("a = [");
    for (int i = 0; i < 5; i++)
        printf("%d%s", a[i], i < 4 ? ", " : "");
    printf("]\n");

    my_free(a);
    return 0;
}
```

## mmap — отображение файлов в память

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    fstat(fd, &st);

    /* отображаем весь файл только для чтения */
    char *data = mmap(NULL, st.st_size,
                      PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    /* теперь data — как массив байтов файла */
    write(STDOUT_FILENO, data, st.st_size);

    munmap(data, st.st_size);
    return 0;
}
```

## Типичные ошибки работы с памятью

### 1. Утечка памяти

```c
/* ПЛОХО */
void leak(void)
{
    char *p = malloc(1024);
    /* забыли free(p) */
}

/* ХОРОШО */
void no_leak(void)
{
    char *p = malloc(1024);
    if (!p) return;
    /* ... работа с p ... */
    free(p);
}
```

### 2. Double free

```c
free(ptr);
free(ptr);  /* ОШИБКА: undefined behavior */

/* Хорошая практика */
free(ptr);
ptr = NULL; /* после освобождения обнуляем указатель */
```

### 3. Use after free

```c
free(ptr);
*ptr = 42;  /* ОШИБКА: использование после освобождения */
```

### 4. Выход за границу буфера

```c
char buf[10];
buf[10] = 'x';   /* ОШИБКА: выход за границу (индексы 0–9) */
strcpy(buf, "это слишком длинная строка"); /* ОШИБКА */
```

## Поиск ошибок памяти: Valgrind

```bash
# Установка
sudo apt install valgrind

# Запуск
valgrind --leak-check=full ./my_program

# Вывод будет содержать:
# ==PID== LEAK SUMMARY
# ==PID== definitely lost: N bytes in M blocks
```

## AddressSanitizer (ASan)

```bash
# Компиляция с ASan
gcc -fsanitize=address -g -o my_program my_program.c

# Запуск — ASan автоматически обнаружит ошибки
./my_program
```

## Отображение разделяемой памяти между процессами

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main(void)
{
    /* SHARED + ANONYMOUS — разделяется с дочерними процессами */
    int *shared = mmap(NULL, sizeof(int),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *shared = 0;

    if (fork() == 0) {
        (*shared)++;
        printf("[child]  shared = %d\n", *shared);
        return 0;
    }

    wait(NULL);
    printf("[parent] shared = %d\n", *shared);  /* видит изменение */

    munmap(shared, sizeof(int));
    return 0;
}
```

## Итог

- Стек — автоматическое управление, куча — ручное
- `malloc`/`free` — стандартные функции работы с кучей
- `mmap` — гибкий механизм отображения памяти
- Ошибки: утечки, double free, use-after-free, buffer overflow
- Инструменты проверки: Valgrind, AddressSanitizer

## Дополнительное чтение

- `man 3 malloc`, `man 2 mmap`, `man 2 brk`
- `man 1 valgrind`
