# Лекция 04: Файловый ввод/вывод

## Файловые дескрипторы

В Unix всё является файлом: обычные файлы, директории, устройства, сокеты, каналы. Все они открываются через одни и те же системные вызовы и возвращают целочисленный **файловый дескриптор**.

```
Таблица файловых дескрипторов процесса:
fd=0 → stdin
fd=1 → stdout
fd=2 → stderr
fd=3 → /home/user/log.txt
fd=4 → /tmp/tmpfile
...
```

## Базовые системные вызовы

### open / close

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    /* Флаги открытия:
     * O_RDONLY — только чтение
     * O_WRONLY — только запись
     * O_RDWR   — чтение и запись
     * O_CREAT  — создать если не существует
     * O_TRUNC  — обрезать до нуля при открытии
     * O_APPEND — писать в конец файла
     */
    int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* Всегда закрывай файлы */
    close(fd);
    return 0;
}
```

Права доступа (mode) в восьмеричной записи:
```
0644 = rw-r--r--
0755 = rwxr-xr-x
0600 = rw-------
0777 = rwxrwxrwx
```

### read / write

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    /* Запись */
    int fd = open("hello.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const char *text = "Hello, File I/O!\n";
    ssize_t written = write(fd, text, strlen(text));
    printf("Записано %zd байт\n", written);
    close(fd);

    /* Чтение */
    fd = open("hello.txt", O_RDONLY);
    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Прочитано: %s", buf);
    }
    close(fd);
    return 0;
}
```

**Важно**: `read` может прочитать меньше запрошенного! Всегда проверяй возвращаемое значение:

```c
ssize_t read_all(int fd, void *buf, size_t count)
{
    size_t total = 0;
    while (total < count) {
        ssize_t n = read(fd, (char *)buf + total, count - total);
        if (n < 0) return -1;  /* ошибка */
        if (n == 0) break;     /* EOF */
        total += n;
    }
    return total;
}
```

### lseek — перемещение позиции

```c
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main(void)
{
    int fd = open("data.bin", O_RDONLY);

    /* SEEK_SET — от начала файла */
    lseek(fd, 0, SEEK_SET);

    /* SEEK_CUR — от текущей позиции */
    lseek(fd, 10, SEEK_CUR);

    /* SEEK_END — от конца файла */
    off_t size = lseek(fd, 0, SEEK_END);
    printf("Размер файла: %ld байт\n", (long)size);

    close(fd);
    return 0;
}
```

## Полный пример: копирование файла

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <src> <dst>\n", argv[0]);
        return 1;
    }

    int src = open(argv[1], O_RDONLY);
    if (src < 0) { perror("open src"); return 1; }

    int dst = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst < 0) { perror("open dst"); close(src); return 1; }

    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(src, buf, BUF_SIZE)) > 0) {
        char *p = buf;
        while (n > 0) {
            ssize_t w = write(dst, p, n);
            if (w < 0) { perror("write"); goto cleanup; }
            p += w;
            n -= w;
        }
    }
    if (n < 0) perror("read");

cleanup:
    close(src);
    close(dst);
    return 0;
}
```

## Информация о файле: stat

```c
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    struct stat st;
    if (stat(argv[1], &st) < 0) {
        perror("stat");
        return 1;
    }

    printf("Файл:         %s\n", argv[1]);
    printf("Размер:       %ld байт\n", (long)st.st_size);
    printf("Инод:         %ld\n", (long)st.st_ino);
    printf("Ссылки:       %ld\n", (long)st.st_nlink);
    printf("Права:        %o\n", st.st_mode & 0777);

    char timebuf[64];
    struct tm *tm = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);
    printf("Изменён:      %s\n", timebuf);

    /* Тип файла */
    if (S_ISREG(st.st_mode))  printf("Тип: обычный файл\n");
    if (S_ISDIR(st.st_mode))  printf("Тип: директория\n");
    if (S_ISLNK(st.st_mode))  printf("Тип: символьная ссылка\n");
    if (S_ISBLK(st.st_mode))  printf("Тип: блочное устройство\n");
    if (S_ISCHR(st.st_mode))  printf("Тип: символьное устройство\n");
    if (S_ISFIFO(st.st_mode)) printf("Тип: FIFO (pipe)\n");

    return 0;
}
```

## Чтение директории

```c
#include <dirent.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    const char *path = argc > 1 ? argv[1] : ".";

    DIR *d = opendir(path);
    if (!d) { perror("opendir"); return 1; }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;  /* пропустить скрытые файлы */

        const char *type = "?";
        switch (entry->d_type) {
        case DT_REG:  type = "file"; break;
        case DT_DIR:  type = "dir";  break;
        case DT_LNK:  type = "link"; break;
        }
        printf("[%s] %s\n", type, entry->d_name);
    }

    closedir(d);
    return 0;
}
```

## Неблокирующий ввод/вывод

```c
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void)
{
    set_nonblocking(STDIN_FILENO);

    char buf[128];
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            printf("Нет данных (неблокирующий режим)\n");
        else
            perror("read");
    }
    return 0;
}
```

## select — мультиплексирование I/O

```c
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};  /* 3 сек */

    int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    if (ready < 0) {
        perror("select");
    } else if (ready == 0) {
        printf("Таймаут — нет данных за 3 сек\n");
    } else {
        char buf[256];
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        buf[n] = '\0';
        printf("Прочитано: %s", buf);
    }
    return 0;
}
```

## Именованный канал (FIFO)

```c
/* writer.c */
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

int main(void)
{
    mkfifo("/tmp/myfifo", 0644);
    int fd = open("/tmp/myfifo", O_WRONLY);
    const char *msg = "Привет через FIFO!\n";
    write(fd, msg, strlen(msg));
    close(fd);
    return 0;
}

/* reader.c */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    int fd = open("/tmp/myfifo", O_RDONLY);
    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("Получено: %s", buf);
    close(fd);
    return 0;
}
```

## sendfile — эффективная передача данных

```c
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/* Копирует файл без промежуточного буфера в user-space */
int fast_copy(const char *src, const char *dst)
{
    int in = open(src, O_RDONLY);
    struct stat st;
    fstat(in, &st);

    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    sendfile(out, in, NULL, st.st_size);

    close(in);
    close(out);
    return 0;
}
```

## Итог

- Всё — файл: обычные файлы, устройства, каналы, сокеты
- Основные вызовы: `open`, `read`, `write`, `close`, `lseek`
- `stat` — метаданные файла, `opendir`/`readdir` — директории
- Неблокирующий I/O + `select`/`poll`/`epoll` для мультиплексирования

## Дополнительное чтение

- `man 2 open`, `man 2 read`, `man 2 write`, `man 2 lseek`
- `man 2 stat`, `man 3 opendir`
- `man 2 select`, `man 7 epoll`
