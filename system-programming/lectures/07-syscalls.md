# Лекция 07: Системные вызовы

## Как работают системные вызовы

Системный вызов — это механизм, через который программы в user-space запрашивают сервисы у ядра. Переход в режим ядра — дорогая операция (переключение привилегий, сохранение контекста).

```
User space                    Kernel space
----------                    ------------

printf("hi")
  └─→ write() [libc]
        └─→ syscall instruction ──→ sys_write()
                                         └─→ vfs_write()
                                               └─→ файловая система
              ←── возврат из ядра ←──────────────
```

### Механизм на x86-64 Linux

1. Программа помещает номер syscall в `rax`
2. Аргументы — в `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9`
3. Выполняет инструкцию `syscall`
4. Ядро обрабатывает запрос
5. Результат возвращается в `rax` (отрицательный → errno)

```c
#include <unistd.h>

/* Прямой системный вызов write без libc */
static ssize_t raw_write(int fd, const void *buf, size_t count)
{
    ssize_t ret;
    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "0"  (1L),          /* rax = SYS_write = 1 */
          "D"  ((long)fd),    /* rdi */
          "S"  (buf),         /* rsi */
          "d"  (count)        /* rdx */
        : "rcx", "r11", "memory"
    );
    return ret;
}

int main(void)
{
    raw_write(1, "Hello!\n", 7);
    return 0;
}
```

## Таблица часто используемых syscall (Linux x86-64)

| Номер | Имя       | Описание                      |
|-------|-----------|-------------------------------|
| 0     | read      | Чтение из fd                  |
| 1     | write     | Запись в fd                   |
| 2     | open      | Открыть файл                  |
| 3     | close     | Закрыть fd                    |
| 4     | stat      | Метаданные файла              |
| 9     | mmap      | Отображение памяти            |
| 11    | munmap    | Удалить отображение           |
| 12    | brk       | Изменить границу кучи         |
| 20    | writev    | Scatter/gather запись         |
| 22    | pipe      | Создать канал                 |
| 32    | dup       | Дублировать fd                |
| 39    | getpid    | ID текущего процесса          |
| 41    | socket    | Создать сокет                 |
| 56    | clone     | Создать процесс/поток         |
| 57    | fork      | Создать дочерний процесс      |
| 59    | execve    | Запустить программу           |
| 60    | exit      | Завершить процесс             |
| 61    | wait4     | Ждать дочернего процесса      |
| 62    | kill      | Послать сигнал                |
| 89    | readdir   | Читать директорию             |
| 231   | exit_group| Завершить группу потоков      |

## strace — анализ системных вызовов

```bash
# Трассировка всех syscall
strace ls

# Трассировка конкретных вызовов
strace -e trace=open,read,write ls

# Статистика
strace -c ls

# Присоединиться к работающему процессу
strace -p 1234

# Сохранить в файл
strace -o strace.log ls
```

Пример вывода `strace ls`:
```
execve("/bin/ls", ["ls"], 0x... /* 20 vars */) = 0
brk(NULL)                                 = 0x55f...
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=...}) = 0
mmap(NULL, ..., PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f...
...
write(1, "file1\nfile2\n", 12)            = 12
```

## ptrace — трассировка и отладка процессов

`ptrace` — мощный syscall для инспекции и контроля процессов. На нём построены GDB, strace, ltrace.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>

int main(void)
{
    pid_t child = fork();
    if (child == 0) {
        /* Дочерний процесс разрешает трассировку */
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
        return 1;
    }

    int status;
    int syscall_count = 0;

    while (1) {
        /* Ждём остановки на системном вызове */
        waitpid(child, &status, 0);
        if (WIFEXITED(status)) break;

        /* Читаем регистры */
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, child, NULL, &regs);

        /* rax на входе в syscall = номер вызова */
        printf("[syscall %3lld] rdi=%lld\n",
               (long long)regs.orig_rax,
               (long long)regs.rdi);
        syscall_count++;

        /* Продолжаем до следующего syscall */
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }

    printf("Итого syscall: %d\n", syscall_count);
    return 0;
}
```

Компиляция: `gcc -o tracer tracer.c`

## seccomp — фильтрация системных вызовов

`seccomp` (Secure Computing Mode) позволяет ограничить набор доступных syscall для процесса:

```c
#include <stdio.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <stddef.h>

int main(void)
{
    /* BPF-фильтр: разрешить только write, read, exit_group */
    struct sock_filter filter[] = {
        /* Загрузить архитектуру */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, arch))),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        /* Загрузить номер syscall */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, nr))),

        /* Разрешить write */
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_write, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* Разрешить exit_group */
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_exit_group, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* Всё остальное — убить процесс */
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
    };

    struct sock_fprog prog = {
        .len    = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, 0, &prog);

    /* Теперь только write и exit_group разрешены */
    write(1, "seccomp активен\n", 16);

    /* Попытка открыть файл убьёт процесс */
    /* open("/tmp/test", 0, 0); */

    return 0;
}
```

## vDSO — виртуальные системные вызовы

Некоторые syscall (gettimeofday, clock_gettime) реализованы в vDSO — маленькой shared library, отображаемой ядром в адресное пространство процесса. Это позволяет избежать перехода в режим ядра.

```bash
# Посмотреть vDSO в карте памяти
cat /proc/self/maps | grep vdso
```

```c
#include <time.h>
#include <stdio.h>

int main(void)
{
    struct timespec ts;
    /* clock_gettime через vDSO — очень быстро, не полный syscall */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("%ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
    return 0;
}
```

## Написание собственного системного вызова (теория)

В ядре Linux системный вызов регистрируется через макрос `SYSCALL_DEFINE`:

```c
/* Пример в исходниках ядра (kernel/sys.c) */
SYSCALL_DEFINE0(getpid)
{
    return task_tgid_vnr(current);
}

SYSCALL_DEFINE3(write, unsigned int, fd,
                const char __user *, buf, size_t, count)
{
    return ksys_write(fd, buf, count);
}
```

Добавление нового syscall требует:
1. Добавить запись в `arch/x86/entry/syscalls/syscall_64.tbl`
2. Объявить прототип в `include/linux/syscalls.h`
3. Реализовать функцию с макросом `SYSCALL_DEFINE`
4. Пересобрать ядро

## Итог

- Syscall — единственный способ программы обратиться к ядру
- `strace` — незаменимый инструмент диагностики
- `ptrace` — основа отладчиков и трассировщиков
- `seccomp` — ограничение syscall для безопасности

## Дополнительное чтение

- `man 2 syscall`, `man 2 ptrace`, `man 2 seccomp`
- `man 1 strace`
- Linux Kernel source: `arch/x86/entry/syscalls/syscall_64.tbl`
