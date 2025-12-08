# Metrics Service

Микросервис для сбора, хранения и предоставления веб-аналитики. Написан на C++23.

## Что сделано

Реализован полноценный сервис с тремя интерфейсами:

### gRPC API (порт 50051)

Методы для получения метрик:
- `GetPageViews` — просмотры страниц
- `GetClicks` — клики пользователей
- `GetPerformance` — метрики производительности (TTFB, FCP, LCP)
- `GetErrors` — ошибки на клиенте
- `GetCustomEvents` — кастомные события

Все методы поддерживают фильтрацию по user_id, page, временному диапазону и пагинацию.

### HTTP API (порт 8080)

Эндпоинты для health checks (используется monitoring-service):
- `GET /health/ping` — liveness probe, возвращает статус сервиса
- `GET /health/ready` — readiness probe, проверяет подключение к БД
- `GET /health` — общий health check
- `GET /ping` — простой ping/pong

### RabbitMQ Consumer

Слушает очередь `metrics_events` и обрабатывает входящие события от api-service.

## Стек технологий

- C++23
- gRPC + Protobuf для API между сервисами
- cpp-httplib для HTTP endpoints
- AMQP-CPP для работы с RabbitMQ
- PostgreSQL + libpqxx для хранения данных
- Docker + Docker Compose для деплоя
- CMake + FetchContent для сборки

## Структура проекта

```
metrics-service/
├── include/
│   ├── database.h       — конфигурация подключения к БД
│   ├── metrics.h        — gRPC сервис
│   ├── rabbitmq.h       — RabbitMQ consumer
│   └── http_handler.h   — HTTP сервер
├── src/
│   ├── main.cpp         — точка входа
│   ├── database.cpp     — работа с PostgreSQL
│   ├── metrics.cpp      — реализация gRPC методов
│   ├── rabbitmq.cpp     — подключение к RabbitMQ
│   └── http_handler.cpp — HTTP endpoints
├── init.sql             — схема БД и тестовые данные
├── Dockerfile
├── docker-compose.yml
└── CMakeLists.txt
```

## Как запустить

```bash
cd metrics-service
docker compose up --build
```

Сервисы:
- metrics-service: gRPC на 50051, HTTP на 8080
- PostgreSQL: 5432
- RabbitMQ: 5672 (AMQP), 15672 (Management UI)

## Как проверить

### HTTP endpoints

```bash
curl http://localhost:8080/health/ping
curl http://localhost:8080/health/ready
curl http://localhost:8080/health
curl http://localhost:8080/ping
```

### gRPC (из корня проекта)

```bash
grpcurl -plaintext -import-path ./proto -proto metrics.proto \
  -d '{}' localhost:50051 metricsys.MetricsService/GetPageViews

grpcurl -plaintext -import-path ./proto -proto metrics.proto \
  -d '{"user_id_filter": "user-001"}' localhost:50051 metricsys.MetricsService/GetPageViews
```

### RabbitMQ

Management UI: http://localhost:15672 (guest/guest)

## Схема базы данных

5 таблиц:
- `page_views` — просмотры страниц
- `click_events` — клики
- `performance_events` — метрики производительности
- `error_events` — ошибки
- `custom_events` — кастомные события

При первом запуске автоматически создаются таблицы и добавляются тестовые данные.

## Взаимодействие с другими сервисами

- **api-service** → отправляет события в RabbitMQ → metrics-service сохраняет в БД
- **aggregation-service** → запрашивает данные через gRPC
- **monitoring-service** → проверяет здоровье через HTTP /health/ping и /health/ready

## Переменные окружения

| Переменная | Значение по умолчанию | Описание |
|------------|----------------------|----------|
| GRPC_PORT | 50051 | Порт gRPC сервера |
| HTTP_PORT | 8080 | Порт HTTP сервера |
| POSTGRES_HOST | postgres | Хост PostgreSQL |
| POSTGRES_DB | metrics_db | Имя базы данных |
| POSTGRES_USER | metrics_user | Пользователь БД |
| POSTGRES_PASSWORD | metrics_password | Пароль БД |
| RABBITMQ_HOST | rabbitmq | Хост RabbitMQ |
| RABBITMQ_PORT | 5672 | Порт RabbitMQ |
| RABBITMQ_USER | guest | Пользователь RabbitMQ |
| RABBITMQ_PASSWORD | guest | Пароль RabbitMQ |
| RABBITMQ_QUEUE | metrics_events | Имя очереди |
