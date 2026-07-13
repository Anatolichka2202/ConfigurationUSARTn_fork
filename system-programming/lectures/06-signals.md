# Лекция 06: Сигналы

## Что такое сигналы?

Сигнал — это программное прерывание, доставляемое процессу ядром или другим процессом. Сигналы используются для:
- Уведомления о событиях (завершение дочернего процесса, ввод с терминала)
- Завершения или приостановки процесса
- Обработки ошибок (деление на ноль, обращение по неверному адресу)

## Стандартные сигналы

| Сигнал   | Номер | Описание                          | Действие по умолчанию |
|----------|-------|-----------------------------------|-----------------------|
| SIGHUP   | 1     | Разрыв соединения (hangup)        | Завершение            |
| SIGINT   | 2     | Прерывание (Ctrl+C)               | Завершение            |
| SIGQUIT  | 3     | Выход (Ctrl+\\)                   | Дамп памяти           |
| SIGILL   | 4     | Недопустимая инструкция           | Дамп памяти           |
| SIGABRT  | 6     | Аварийное завершение (abort())    | Дамп памяти           |
| SIGFPE   | 8     | Ошибка арифметики (деление на 0)  | Дамп памяти           |
| SIGKILL  | 9     | Безусловное завершение            | Завершение (неперех.) |
| SIGSEGV  | 11    | Нарушение сегментации             | Дамп памяти           |
| SIGPIPE  | 13    | Запись в закрытый pipe            | Завершение            |
| SIGTERM  | 15    | Запрос завершения                 | Завершение            |
| SIGCHLD  | 17    | Завершение дочернего процесса     | Игнор                 |
| SIGCONT  | 18    | Возобновление приостановленного   | Продолжение           |
| SIGSTOP  | 19    | Приостановить процесс             | Остановка (неперех.)  |
| SIGUSR1  | 10    | Пользовательский сигнал 1         | Завершение            |
| SIGUSR2  | 12    | Пользовательский сигнал 2         | Завершение            |

## signal() — простая установка обработчика

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

static void sigint_handler(int sig)
{
    (void)sig;
    printf("\nПолучен SIGINT, завершаемся...\n");
    running = 0;
}

int main(void)
{
    signal(SIGINT, sigint_handler);
    /* signal(SIGINT, SIG_IGN);  -- игнорировать */
    /* signal(SIGINT, SIG_DFL);  -- действие по умолчанию */

    printf("Нажмите Ctrl+C для завершения\n");
    while (running) {
        printf("Работаю...\n");
        sleep(1);
    }
    return 0;
}
```

**Важно**: используй `volatile sig_atomic_t` для флагов в обработчиках сигналов — это единственный безопасный тип.

## sigaction() — надёжная установка обработчика

`signal()` имеет непортируемое поведение. Предпочтительна `sigaction()`:

```c
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static void handler(int sig, siginfo_t *info, void *ctx)
{
    (void)ctx;
    /* Безопасные функции в обработчике: write, _exit, ...
     * НЕ безопасны: printf, malloc, и большинство libc функций */
    const char *msg = "Получен сигнал\n";
    write(STDOUT_FILENO, msg, 15);
    (void)sig; (void)info;
}

int main(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    /* Блокировать SIGTERM пока выполняется обработчик SIGINT */
    sigaddset(&sa.sa_mask, SIGTERM);

    sigaction(SIGINT, &sa, NULL);

    pause();  /* ждём сигнала */
    return 0;
}
```

## Маскирование сигналов

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    /* Заблокировать SIGINT и SIGTERM */
    sigprocmask(SIG_BLOCK, &mask, &old_mask);

    printf("Сигналы заблокированы на 3 секунды...\n");
    sleep(3);

    /* Восстановить маску */
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
    printf("Сигналы разблокированы\n");

    return 0;
}
```

## Отправка сигналов

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = 12345;  /* PID целевого процесса */

    /* Отправить SIGTERM процессу */
    if (kill(pid, SIGTERM) < 0)
        perror("kill");

    /* Отправить сигнал самому себе */
    kill(getpid(), SIGUSR1);

    /* Отправить сигнал группе процессов */
    kill(-getpgrp(), SIGTERM);

    /* raise — отправить самому себе */
    raise(SIGUSR1);

    return 0;
}
```

## Таймеры через SIGALRM

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static int timer_count = 0;

static void alarm_handler(int sig)
{
    (void)sig;
    timer_count++;
    printf("Таймер сработал %d раз\n", timer_count);
    if (timer_count < 5)
        alarm(1);  /* следующий будильник через 1 секунду */
}

int main(void)
{
    signal(SIGALRM, alarm_handler);
    alarm(1);  /* первый будильник через 1 секунду */

    while (timer_count < 5)
        pause();  /* ждём сигнала (не тратим CPU) */

    printf("Готово\n");
    return 0;
}
```

## signalfd() — получение сигналов через fd

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/signalfd.h>

int main(void)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    /* Блокируем стандартную доставку */
    sigprocmask(SIG_BLOCK, &mask, NULL);

    /* Создаём файловый дескриптор для чтения сигналов */
    int sfd = signalfd(-1, &mask, 0);

    printf("Ожидаю сигналы...\n");

    struct signalfd_siginfo si;
    ssize_t n = read(sfd, &si, sizeof(si));
    if (n == sizeof(si)) {
        printf("Получен сигнал %u от PID %u\n",
               si.ssi_signo, si.ssi_pid);
    }

    close(sfd);
    return 0;
}
```

Преимущество: можно использовать `select`/`poll`/`epoll` для обработки сигналов вместе с другими I/O.

## Обработка SIGCHLD: уборка зомби

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

static void sigchld_handler(int sig)
{
    (void)sig;
    int status;
    pid_t pid;
    /* Собираем всех завершившихся дочерних */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf),
                             "Child %d exited: %d\n",
                             pid, WEXITSTATUS(status));
            write(STDOUT_FILENO, buf, n);
        }
    }
}

int main(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sa.sa_flags   = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
            sleep(i + 1);
            return i;
        }
    }

    sleep(5);  /* ждём завершения всех дочерних */
    return 0;
}
```

## Что безопасно делать в обработчике?

POSIX определяет список **async-signal-safe** функций — только их можно вызывать внутри обработчика сигнала:

✅ Безопасно: `write`, `read`, `close`, `open`, `_exit`, `kill`, `signal`, `sigaction`, `waitpid`, `getpid`

❌ Небезопасно: `printf`, `malloc`, `free`, `exit`, `fopen`, любые функции, использующие глобальные буферы

## Итог

- Сигналы — асинхронные уведомления процессу
- `sigaction()` предпочтительнее `signal()`
- В обработчиках только async-signal-safe функции
- `signalfd()` позволяет обрабатывать сигналы как I/O

## Дополнительное чтение

- `man 7 signal`, `man 2 sigaction`, `man 2 sigprocmask`
- `man 2 kill`, `man 2 signalfd`
