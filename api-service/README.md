# API сервис (API Service)

API сервис для приёма метрик с фронтенда через REST API и отправки в RabbitMQ для обработки metrics-service.

Полная спецификация API описана в файле [`openapi.yaml`](openapi.yaml).

## Эндпоинты

| Метод | Путь                    | Описание                                          |
| ----- | ----------------------- | ------------------------------------------------- |
| GET   | `/health/ping`          | Health check                                      |
| POST  | `/page-views`           | Отправка события просмотра страницы               |
| POST  | `/clicks`               | Отправка события клика/UI взаимодействия          |
| POST  | `/performance`          | Отправка метрик производительности                |
| POST  | `/errors`               | Отправка события ошибки фронтенда                 |
| POST  | `/custom-events`        | Отправка кастомного события                       |
| GET   | `/uptime`               | Uptime по сервису (day/week/month/year в периоде) |
| GET   | `/uptime/{period}`      | Uptime по конкретному периоду                     |
| GET   | `/aggregation/watermark`| Метка завершенной агрегации                       |
| POST  | `/aggregation/*`        | Получение агрегированных метрик                   |

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
# {"status":"ok"}

# Отправка page view
curl -X POST http://localhost:8080/page-views \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "timestamp": 1733505600}'
# {"status":"accepted"}

# Отправка клика
curl -X POST http://localhost:8080/clicks \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "element_id": "btn-signup", "timestamp": 1733505600}'
# {"status":"accepted"}

# Отправка performance
curl -X POST http://localhost:8080/performance \
  -H "Content-Type: application/json" \
  -d '{"page": "/dashboard", "ttfb_ms": 120, "timestamp": 1733505610}'
# {"status":"accepted"}

# Отправка ошибки
curl -X POST http://localhost:8080/errors \
  -H "Content-Type: application/json" \
  -d '{"page": "/dashboard", "error_type": "js_exception", "message": "Oops", "timestamp": 1733505615}'
# {"status":"accepted"}

# Отправка кастомного события
curl -X POST http://localhost:8080/custom-events \
  -H "Content-Type: application/json" \
  -d '{"name": "signup_completed", "page": "/signup/success", "timestamp": 1733505620}'
# {"status":"accepted"}

# Uptime (все периоды)
curl "http://localhost:8080/uptime?service=api-service"
# {"service":"api-service","period":"all","periods":{"day":{"ok":0,"total":0,"percent":0},"week":{"ok":0,"total":0,"percent":0},"month":{"ok":0,"total":0,"percent":0},"year":{"ok":0,"total":0,"percent":0}}}

# Uptime с периодом
curl "http://localhost:8080/uptime?service=api-service&period=day"
# {"service":"api-service","period":"day","periods":{"day":{"ok":0,"total":0,"percent":0}}}

# Uptime fixed endpoints
curl "http://localhost:8080/uptime/day?service=api-service"
curl "http://localhost:8080/uptime/week?service=metrics-service"
# {"service":"metrics-service","period":"week","periods":{"week":{"ok":0,"total":0,"percent":0}}}

# Aggregation watermark
curl http://localhost:8080/aggregation/watermark
# {"last_aggregated_at":"2024-12-06T10:05:00Z"}

# Aggregation page-views
curl -X POST http://localhost:8080/aggregation/page-views \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/dashboard", "pagination": {"limit": 100, "offset": 0}}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/dashboard","views_count":2400,"unique_users":350,"unique_sessions":420,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation clicks
curl -X POST http://localhost:8080/aggregation/clicks \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/pricing", "element_id": "cta-subscribe-button"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/pricing","element_id":"cta-subscribe-button","clicks_count":180,"unique_users":120,"unique_sessions":140,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation performance
curl -X POST http://localhost:8080/aggregation/performance \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/dashboard"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/dashboard","samples_count":500,"avg_total_load_ms":1100,"p95_total_load_ms":1500,"avg_ttfb_ms":120,"p95_ttfb_ms":200,"avg_fcp_ms":450,"p95_fcp_ms":600,"avg_lcp_ms":900,"p95_lcp_ms":1250,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation errors
curl -X POST http://localhost:8080/aggregation/errors \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/dashboard", "error_type": "js_exception"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/dashboard","error_type":"js_exception","errors_count":45,"warning_count":12,"critical_count":8,"unique_users":30,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation custom events
curl -X POST http://localhost:8080/aggregation/custom-events \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "event_name": "signup_completed", "page": "/signup/success"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","event_name":"signup_completed","page":"/signup/success","events_count":75,"unique_users":60,"unique_sessions":65,"created_at":"2024-12-06T10:05:00Z"}]}
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
