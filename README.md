# ConfigurationUSARTn

Библиотека на языке C для конфигурации периферии USART/UART на микроконтроллерах GD32F303 и GD32F450.
Предоставляет функции инициализации USART, настройки DMA для передачи и приёма, а также вспомогательные функции polling-отправки.

---

## Поддерживаемые семейства

| Макрос          | Семейство    | Заголовочный файл SDK |
|-----------------|--------------|----------------------|
| `GD32F303_`     | GD32F303xx   | `gd32f30x.h`         |
| `GD32F450_`     | GD32F450xx   | `gd32f4xx.h`         |

Выбор семейства выполняется в `ConfigurationUSARTn.h` раскомментированием нужного макроса:

```c
//#define GD32F303_
#define GD32F450_
```

Одновременное определение обоих макросов вызывает ошибку компиляции.

---

## Настройка GPIO на стороне проекта

Библиотека не инициализирует GPIO. Пользователь обязан настроить выводы TX, RX (и, при необходимости, CK) до вызова функций конфигурации USART.

### GD32F303

```c
gpio_init(USARTx_PORT, GPIO_MODE_AF_PP,       GPIO_OSPEED_50MHZ, USARTx_TX_PIN);
gpio_init(USARTx_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, USARTx_RX_PIN);
```

### GD32F450

```c
gpio_af_set            (USARTx_PORT, USARTx_AF,                        USARTx_TX_PIN);
gpio_mode_set          (USARTx_PORT, GPIO_MODE_AF,   GPIO_PUPD_PULLUP,  USARTx_TX_PIN);
gpio_output_options_set(USARTx_PORT, GPIO_OTYPE_PP,  GPIO_OSPEED_50MHZ, USARTx_TX_PIN);

gpio_af_set            (USARTx_PORT, USARTx_AF,                        USARTx_RX_PIN);
gpio_mode_set          (USARTx_PORT, GPIO_MODE_AF,   GPIO_PUPD_NONE,    USARTx_RX_PIN);
gpio_output_options_set(USARTx_PORT, GPIO_OTYPE_PP,  GPIO_OSPEED_50MHZ, USARTx_RX_PIN);
```

Конкретные номера выводов, порты и значения AF необходимо уточнять по reference manual используемого МК и схеме платы.

---

## Функции конфигурации

### ConfigUsart

Инициализирует выбранный USART/UART: сброс, установка скорости, формата кадра, режима MSB/LSB, включение передатчика и приёмника. При необходимости — настройка прерываний и NVIC.

**GD32F303:**

```c
void ConfigUsart(uint32_t usart, uint32_t baudrate, uint32_t msbf,
                 uint8_t configIrqn, uint8_t priority, uint8_t sub_priority);
```

**GD32F450:**

```c
void ConfigUsart(uint32_t usart, uint32_t baudrate, uint32_t msbf,
                 uint32_t oversample, uint8_t configIrqn,
                 uint8_t priority, uint8_t sub_priority);
```

Параметр `oversample` (только GD32F450): `USART_OVSMOD_8` или `USART_OVSMOD_16`.

Параметр `configIrqn` — битовая маска из `enum confInterrupt`:

| Значение           | Описание                               |
|--------------------|----------------------------------------|
| `non`              | Прерывания не используются             |
| `receiveRBNE`      | Прерывание по приёму (флаг RBNE)       |
| `transmissionTBE`  | Прерывание по готовности буфера TX     |
| `transmissionTC`   | Прерывание по завершению передачи      |

Пример (GD32F303):

```c
ConfigUsart(USART1, 115200, USART_MSBF_LSB, non, 1, 4);
```

Пример (GD32F450):

```c
ConfigUsart(UART7, 115200, USART_MSBF_LSB, USART_OVSMOD_8, receiveRBNE, 0, 5);
```

---

### ConfigUsartDMA_Tx

Настраивает канал DMA для передачи данных из памяти в USART. Должна вызываться после `ConfigUsart`.

```c
// GD32F303
void ConfigUsartDMA_Tx(enum usartDMA usart, uint32_t* buf, uint32_t lenBuf,
                       _Bool circulationEnable, uint32_t channelPriorityDMA,
                       uint8_t priority, uint8_t sub_priority, uint8_t iRQn);

// GD32F450
void ConfigUsartDMA_Tx(uint32_t usart, uint8_t* buf, uint32_t lenBuf,
                       _Bool circulationEnable, uint32_t channelPriorityDMA,
                       uint8_t priority, uint8_t sub_priority, uint8_t iRQn);
```

Параметр `iRQn` — битовая маска из `enum confInterruptDMATransmit`:

| Значение                | Описание                                    |
|-------------------------|---------------------------------------------|
| `iRQn_non_Tx`           | Прерывания DMA не используются              |
| `iRQn_half_transmit_Dma`| Прерывание по половинной передаче (HTF)     |
| `iRQn_full_transmit_Dma`| Прерывание по полной передаче (FTF)         |

Пример (GD32F450):

```c
ConfigUsartDMA_Tx(UART7, buffer_tx, BUFFER_SIZE, 0, DMA_PRIORITY_HIGH, 0, 6, iRQn_full_transmit_Dma);
```

---

### ConfigUsartDMA_Rx

Настраивает канал DMA для приёма данных из USART в память. Должна вызываться после `ConfigUsart`.

```c
// GD32F303
void ConfigUsartDMA_Rx(enum usartDMA usart, uint32_t* buf, uint32_t lenBuf,
                       _Bool circulationEnable, uint32_t channelPriorityDMA,
                       uint8_t priority, uint8_t sub_priority, uint8_t iRQn);

// GD32F450
void ConfigUsartDMA_Rx(uint32_t usart, uint8_t* buf, uint32_t lenBuf,
                       _Bool circulationEnable, uint32_t channelPriorityDMA,
                       uint8_t priority, uint8_t sub_priority, uint8_t iRQn);
```

Параметр `iRQn` — битовая маска из `enum confInterruptDMAReceiv`:

| Значение               | Описание                                    |
|------------------------|---------------------------------------------|
| `iRQn_non_Rx`          | Прерывания не используются                  |
| `iRQn_receiv_Uart`     | Прерывание USART по флагу IDLE              |
| `iRQn_half_receiv_Dma` | Прерывание DMA по половинному приёму (HTF)  |
| `iRQn_full_receiv_Dma` | Прерывание DMA по полному приёму (FTF)      |

Пример (GD32F303):

```c
ConfigUsartDMA_Rx(Usart1, &rx_buffer, RX_BUFFER_SIZE, 0, DMA_PRIORITY_HIGH, 1, 4, iRQn_full_receiv_Dma);
```

---

## Polling-функции отправки

Функции доступны при определённом макросе `SENDING_VIA_USART` (включён по умолчанию).

```c
// Отправить один байт (блокирующий режим)
void Usart_send_byte(const uint8_t byte, const uint32_t usart_perith);

// Отправить буфер заданной длины
void Usart_send_buf(const void* const buf, const uint32_t usart_perith, const uint32_t len);

// Отправить строку, завершённую нулём
void Usart_send_string(const void* const str, const uint32_t usart_perith);
```

Все функции работают в режиме polling: процессор занят на время передачи каждого байта. Применять там, где это допустимо по требованиям к реальному времени.

---

## Прерывания

Заголовочный файл содержит объявления обработчиков прерываний USART и DMA. Тела обработчиков в исходнике закомментированы как примеры. Проект обязан самостоятельно определить нужные обработчики и очистить соответствующие флаги.

Шаблон обработчика USART (из комментариев в .h):

```c
void USART0_IRQHandler(void)
{
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE) != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
    }
}
```

Шаблон обработчика DMA:

```c
void DMA0_Channel3_IRQHandler(void)
{
    if (dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF) != RESET) {
        dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);
    }
}
```

Для повторного запуска DMA-канала после завершения передачи или приёма:

```c
dma_channel_disable(DMA0, DMA_CH3);
dma_memory_address_config(DMA0, DMA_CH3, (uint32_t)data_buffer);
dma_transfer_number_config(DMA0, DMA_CH3, length);
dma_channel_enable(DMA0, DMA_CH3);
```

---

## Соответствие DMA-каналов, IRQ и GPIO

Соответствие конкретных DMA-каналов, номеров субпериферий, IRQ и выводов GPIO зависит от ревизии МК и конфигурации платы. Значения, заданные в библиотеке, должны быть проверены по reference manual конкретного МК и схеме платы перед использованием в проекте. Работоспособность конфигурации на конкретной плате без такой проверки не гарантируется.

---

## Ограничения

- Библиотека не выполняет инициализацию тактирования GPIO и USART для GD32F303 (это обязанность вызывающего кода). Для GD32F450 тактирование USART включается внутри `ConfigUsart`.
- Нет защиты от некорректных значений baudrate или нулевой длины буфера.
- Циклический режим DMA (`circulationEnable = 1`) требует ручного управления указателями и счётчиком в обработчике прерывания — это не реализовано в библиотеке.
- Аппаратная проверка библиотеки на конкретных платах не проводилась в рамках данной версии.
