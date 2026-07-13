# Лекция 05: Сетевое программирование

## Сокеты

Сокет — это конечная точка сетевого соединения. В Unix сокет является файловым дескриптором, работающим через стандартные `read`/`write`.

### Типы сокетов

| Тип           | Константа      | Протокол | Описание                    |
|---------------|---------------|----------|-----------------------------|
| Stream        | SOCK_STREAM   | TCP      | Надёжная доставка, с соединением |
| Datagram      | SOCK_DGRAM    | UDP      | Быстро, без гарантий        |
| Raw           | SOCK_RAW      | —        | Прямой доступ к протоколу   |
| Unix domain   | SOCK_STREAM   | AF_UNIX  | Локальный IPC               |

### Семейства адресов

| Константа  | Описание          |
|------------|-------------------|
| AF_INET    | IPv4              |
| AF_INET6   | IPv6              |
| AF_UNIX    | Unix domain       |

## TCP: шаблон сервера

```
Сервер:                         Клиент:
socket()                        socket()
bind()
listen()
accept() ←——— connect() ————→
read()   ←——— write()  ————→
write()  ————→ read()  ————→
close()                         close()
```

### Простой TCP-сервер

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BACKLOG 5
#define BUF_SIZE 1024

int main(void)
{
    /* 1. Создаём сокет */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    /* Разрешаем повторное использование адреса */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. Привязываем к адресу и порту */
    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,  /* любой интерфейс */
        .sin_port        = htons(PORT),
    };
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    /* 3. Начинаем слушать */
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen"); return 1;
    }
    printf("Сервер слушает порт %d...\n", PORT);

    /* 4. Принимаем соединения */
    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);
        if (client_fd < 0) { perror("accept"); continue; }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        printf("Подключился: %s:%d\n", ip, ntohs(client_addr.sin_port));

        /* 5. Обмен данными (echo) */
        char buf[BUF_SIZE];
        ssize_t n;
        while ((n = read(client_fd, buf, sizeof(buf))) > 0)
            write(client_fd, buf, n);  /* отправляем обратно */

        close(client_fd);
        printf("Клиент отключился\n");
    }

    close(server_fd);
    return 0;
}
```

### Простой TCP-клиент

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(PORT),
    };
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect"); return 1;
    }
    printf("Подключились к серверу\n");

    const char *msg = "Hello, Server!";
    write(fd, msg, strlen(msg));

    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("Ответ: %s\n", buf);

    close(fd);
    return 0;
}
```

## UDP: дейтаграммы

```c
/* UDP-сервер */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(9090),
    };
    bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    printf("UDP сервер на порту 9090\n");

    char buf[1024];
    struct sockaddr_in client;
    socklen_t clen = sizeof(client);

    for (;;) {
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0,
                             (struct sockaddr *)&client, &clen);
        buf[n] = '\0';

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
        printf("Получено от %s: %s\n", ip, buf);

        /* отправляем обратно */
        sendto(fd, buf, n, 0, (struct sockaddr *)&client, clen);
    }

    close(fd);
    return 0;
}
```

## Unix Domain Sockets — локальный IPC

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/example.sock"

/* Сервер */
int unix_server(void)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);  /* удалить если существует */
    bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(fd, 5);

    int client = accept(fd, NULL, NULL);
    char buf[256];
    ssize_t n = read(client, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("Получено: %s\n", buf);
    write(client, "OK", 2);

    close(client);
    close(fd);
    unlink(SOCKET_PATH);
    return 0;
}
```

## epoll — высокопроизводительное мультиплексирование

Для серверов с тысячами соединений `select`/`poll` слишком медленны. `epoll` — Linux-специфичный механизм с O(1) на событие:

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_EVENTS 64
#define PORT 8080

static int make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    make_nonblocking(server_fd);

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(PORT),
    };
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    /* Создаём epoll-экземпляр */
    int epfd = epoll_create1(0);

    struct epoll_event ev = {
        .events  = EPOLLIN,
        .data.fd = server_fd,
    };
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    struct epoll_event events[MAX_EVENTS];
    printf("epoll сервер на порту %d\n", PORT);

    for (;;) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                /* Новое соединение */
                int client = accept(server_fd, NULL, NULL);
                make_nonblocking(client);
                ev.events  = EPOLLIN | EPOLLET;  /* edge-triggered */
                ev.data.fd = client;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
            } else {
                /* Данные от клиента */
                char buf[1024];
                ssize_t len = read(fd, buf, sizeof(buf));
                if (len <= 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    write(fd, buf, len);  /* echo */
                }
            }
        }
    }

    return 0;
}
```

## Порядок байтов в сети

Сетевой порядок байтов — big-endian. Функции преобразования:

```c
#include <arpa/inet.h>

/* host to network */
uint16_t htons(uint16_t host_short);   /* для портов */
uint32_t htonl(uint32_t host_long);    /* для IPv4-адресов */

/* network to host */
uint16_t ntohs(uint16_t net_short);
uint32_t ntohl(uint32_t net_long);
```

## Итог

- Сокет — универсальный файловый дескриптор для сети и IPC
- TCP: надёжно, с соединением; UDP: быстро, без гарантий
- `epoll` — для высоконагруженных серверов
- Всегда конвертируй порядок байтов через `htons`/`htonl`

## Дополнительное чтение

- `man 2 socket`, `man 2 bind`, `man 2 connect`
- `man 7 ip`, `man 7 tcp`, `man 7 udp`
- `man 7 epoll`
