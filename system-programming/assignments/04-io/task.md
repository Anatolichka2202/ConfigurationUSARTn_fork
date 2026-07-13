# Задание 04: Копировщик файлов

## Цель

Написать утилиту копирования файлов, аналогичную `cp`, используя только POSIX-системные вызовы.

## Задача

Реализовать программу `mycp`:

```
mycp <источник> <назначение> [--progress]
```

### Требования

1. **Базовое копирование**:
   - Открыть исходный файл через `open()`
   - Читать блоками по 4096 байт через `read()`
   - Записывать через `write()` с гарантией полной записи
   - Сохранить права доступа исходного файла

2. **Обработка ошибок**:
   - Проверять все возвращаемые значения
   - Выводить информативные сообщения через `perror()` или `strerror()`
   - Коды выхода: 0 — успех, 1 — ошибка

3. **Опция `--progress`** (по желанию):
   - Вывести процент выполнения в stderr

4. **Проверить** через `diff`:
   ```bash
   ./mycp /etc/hostname /tmp/hostname_copy
   diff /etc/hostname /tmp/hostname_copy && echo "OK"
   ```

## Структура программы

```c
int main(int argc, char *argv[]);
static ssize_t write_all(int fd, const void *buf, size_t count);
static int copy_file(const char *src, const char *dst);
```

## Проверка

```bash
gcc -Wall -Wextra -o solution solution.c

# Тест 1: копирование текстового файла
./solution /etc/passwd /tmp/passwd_copy
diff /etc/passwd /tmp/passwd_copy && echo "Тест 1: OK"

# Тест 2: несуществующий файл
./solution /nonexistent /tmp/out
echo "Exit code: $?"  # должен быть 1

# Тест 3: права доступа
ls -la /etc/passwd
ls -la /tmp/passwd_copy
```

## Усложнение (по желанию)

Поддержать копирование директории рекурсивно (`-r` флаг) через `opendir`/`readdir`.
