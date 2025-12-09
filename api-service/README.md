# API сервис (API Service)

API сервис для приёма метрик с фронтенда через REST API и отправки в RabbitMQ для обработки metrics-service.

Полная спецификация API описана в файле [`openapi.yaml`](openapi.yaml).

## Эндпоинты

| Метод | Путь             | Описание                                 |
| ----- | ---------------- | ---------------------------------------- |
| GET   | `/health/ping`   | Health check                             |
| POST  | `/page-views`    | Отправка события просмотра страницы      |
| POST  | `/clicks`        | Отправка события клика/UI взаимодействия |
| POST  | `/performance`   | Отправка метрик производительности       |
| POST  | `/errors`        | Отправка события ошибки фронтенда        |
| POST  | `/custom-events` | Отправка кастомного события              |

## Технологии

- **C++23**
- **cpp-httplib** — HTTP сервер
- **nlohmann/json** — работа с JSON
- **rabbitmq-c** — отправка событий в RabbitMQ

## Сборка и запуск

### Docker (рекомендуется)

```bash
cd api-service
docker compose up --build -d
```

### Локальная сборка

#### Требования

- CMake 3.14+
- Компилятор с поддержкой C++23 (GCC 13+, Clang 16+)
- librabbitmq-dev

#### Сборка

```bash
cd api-service
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

#### Запуск сервера

```bash
./api-service
```

Сервер запустится на `http://localhost:8080`.

## Переменные окружения

| Переменная          | По умолчанию | Описание      |
| ------------------- | ------------ | ------------- |
| `RABBITMQ_HOST`     | `localhost`  | Хост RabbitMQ |
| `RABBITMQ_PORT`     | `5672`       | Порт RabbitMQ |
| `RABBITMQ_USERNAME` | `guest`      | Пользователь  |
| `RABBITMQ_PASSWORD` | `guest`      | Пароль        |
| `RABBITMQ_VHOST`    | `/`          | Virtual host  |

## Тестирование

### Запуск тестов

```bash
# Установить зависимость
pip install requests

# Запустить тесты (сервер должен быть запущен)
python3 tests/test_handlers.py
```

### Пример запроса

```bash
# Health check
curl http://localhost:8080/health/ping

# Отправка page view
curl -X POST http://localhost:8080/page-views \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "timestamp": 1733505600}'

# Отправка клика
curl -X POST http://localhost:8080/clicks \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "element_id": "btn-signup", "timestamp": 1733505600}'
```

## Структура проекта

```
api-service/
├── CMakeLists.txt
├── Dockerfile
├── docker-compose.yml
├── openapi.yaml          # OpenAPI спецификация
├── README.md
├── include/
│   ├── handlers.hpp      # Объявления handlers
│   ├── models.hpp        # Модели данных
│   └── rabbitmq.hpp      # RabbitMQ клиент
├── src/
│   ├── main.cpp          # Точка входа
│   ├── handlers.cpp      # Реализация handlers
│   ├── models.cpp        # Сериализация моделей
│   └── rabbitmq.cpp      # Реализация RabbitMQ клиента
└── tests/
    └── test_handlers.py  # Python тесты
```

## Очереди RabbitMQ

Сервис публикует события в следующие очереди:
- `page_views` — просмотры страниц
- `clicks` — клики
- `performance_events` — метрики производительности
- `error_events` — ошибки
- `custom_events` — кастомные события
