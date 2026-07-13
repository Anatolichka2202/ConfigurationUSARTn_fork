# Лекция 02: Процессы и потоки

## Процесс

Процесс — это экземпляр выполняемой программы. Каждый процесс имеет:
- Уникальный идентификатор **PID** (Process ID)
- Собственное виртуальное адресное пространство
- Набор открытых файловых дескрипторов
- Информацию о владельце (UID, GID)
- Рабочий каталог
- Переменные окружения

```
Виртуальное адресное пространство процесса (Linux x86-64):

0xFFFFFFFFFFFFFFFF
   +-------------------+
   |   Ядро (kernel)   |  недоступно из user-space
   +-------------------+  0xFFFF800000000000
   |       ...         |
   +-------------------+
   |       Stack       |  ← растёт вниз
   +-------------------+
   |       ...         |
   +-------------------+
   |    Heap (куча)    |  ← растёт вверх (malloc/brk)
   +-------------------+
   |  BSS (неинит.)    |  глобальные/статические нули
   +-------------------+
   |  Data (инит.)     |  глобальные/статические переменные
   +-------------------+
   |  Text (код)       |  машинный код программы
   +-------------------+
0x0000000000000000
```

## fork() — создание дочернего процесса

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Дочерний процесс */
        printf("[child]  PID=%d, PPID=%d\n", getpid(), getppid());
        exit(0);
    } else {
        /* Родительский процесс */
        printf("[parent] PID=%d, child PID=%d\n", getpid(), pid);
        int status;
        waitpid(pid, &status, 0);  /* ждём завершения дочернего */
        if (WIFEXITED(status))
            printf("[parent] child exited with code %d\n", WEXITSTATUS(status));
    }
    return 0;
}
```

**Copy-on-Write**: при `fork()` физические страницы памяти не копируются немедленно. Копирование происходит только при первой записи — это оптимизация ядра.

## exec() — замена образа процесса

```c
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("До exec\n");

    /* execl заменяет текущий процесс программой /bin/ls */
    execl("/bin/ls", "ls", "-la", NULL);

    /* Эта строка не выполнится при успехе exec */
    perror("execl");
    return 1;
}
```

Семейство функций exec:
| Функция   | Аргументы       | Поиск в PATH | Окружение |
|-----------|-----------------|--------------|-----------|
| execl     | список          | нет          | текущее   |
| execv     | массив          | нет          | текущее   |
| execlp    | список          | да           | текущее   |
| execvp    | массив          | да           | текущее   |
| execle    | список          | нет          | своё      |
| execve    | массив          | нет          | своё      |

## Ожидание дочернего процесса

```c
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();
    if (pid == 0) {
        sleep(1);
        return 42;  /* код завершения */
    }

    int status;
    pid_t died = wait(&status);   /* ждём любого дочернего */

    if (WIFEXITED(status))
        printf("Процесс %d завершился с кодом %d\n",
               died, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Процесс %d убит сигналом %d\n",
               died, WTERMSIG(status));

    return 0;
}
```

## Дерево процессов

```bash
# Просмотр дерева процессов
pstree -p

# Информация о процессе
cat /proc/$$/status

# Список всех процессов
ps aux
```

## Потоки (pthreads)

Поток разделяет адресное пространство с другими потоками процесса. У каждого потока есть:
- Свой стек
- Свой набор регистров
- TID (Thread ID)

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int id;
    const char *message;
} ThreadArg;

static void *thread_func(void *arg)
{
    ThreadArg *a = (ThreadArg *)arg;
    for (int i = 0; i < 3; i++) {
        printf("[thread %d] %s (iter %d)\n", a->id, a->message, i);
        usleep(100000);
    }
    return (void *)(long)a->id;
}

int main(void)
{
    pthread_t t1, t2;
    ThreadArg a1 = {1, "Hello"};
    ThreadArg a2 = {2, "World"};

    pthread_create(&t1, NULL, thread_func, &a1);
    pthread_create(&t2, NULL, thread_func, &a2);

    void *ret1, *ret2;
    pthread_join(t1, &ret1);
    pthread_join(t2, &ret2);

    printf("t1 returned %ld, t2 returned %ld\n",
           (long)ret1, (long)ret2);
    return 0;
}
```

Компиляция: `gcc -Wall -o threads threads.c -lpthread`

## Мьютекс — защита разделяемых данных

```c
#include <stdio.h>
#include <pthread.h>

static long counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *increment(void *arg)
{
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter = %ld (ожидается 2000000)\n", counter);
    pthread_mutex_destroy(&lock);
    return 0;
}
```

## Условные переменные

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
static int ready = 0;

static void *producer(void *arg)
{
    (void)arg;
    sleep(1);
    pthread_mutex_lock(&mutex);
    ready = 1;
    printf("[producer] данные готовы\n");
    pthread_cond_signal(&cond);   /* сигнал потребителю */
    pthread_mutex_unlock(&mutex);
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&mutex);
    while (!ready)
        pthread_cond_wait(&cond, &mutex);  /* ждёт и отпускает mutex */
    printf("[consumer] получил данные\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(void)
{
    pthread_t tp, tc;
    pthread_create(&tc, NULL, consumer, NULL);
    pthread_create(&tp, NULL, producer, NULL);
    pthread_join(tp, NULL);
    pthread_join(tc, NULL);
    return 0;
}
```

## Процесс vs. Поток

| Характеристика       | Процесс          | Поток                   |
|---------------------|------------------|-------------------------|
| Адресное пространство| Изолировано      | Разделяемое             |
| Создание             | fork() — дорого  | pthread_create() — дешево|
| Взаимодействие       | IPC (pipe, socket)| Разделяемая память      |
| Отказоустойчивость   | Высокая          | Ниже (крэш убивает всех)|
| Переключение контекста| Дорого          | Дешевле                 |

## IPC — межпроцессное взаимодействие

### Неименованный канал (pipe)

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int pipefd[2];
    pipe(pipefd);  /* pipefd[0]=читать, pipefd[1]=писать */

    if (fork() == 0) {
        /* дочерний: пишем */
        close(pipefd[0]);
        const char *msg = "hello from child\n";
        write(pipefd[1], msg, strlen(msg));
        close(pipefd[1]);
        return 0;
    }

    /* родительский: читаем */
    close(pipefd[1]);
    char buf[128];
    ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("received: %s", buf);
    close(pipefd[0]);
    return 0;
}
```

## Итог

- `fork()` создаёт копию процесса; `exec()` заменяет образ
- Потоки дешевле процессов, но требуют синхронизации
- Мьютексы защищают разделяемые данные
- Условные переменные используются для оповещения потоков

## Дополнительное чтение

- `man 2 fork`, `man 2 execve`, `man 2 waitpid`
- `man 7 pthreads`
- `man 7 pipe`
