# Metrics Service

Микросервис для сбора, хранения и предоставления веб-аналитики. Написан на C++23.

## Быстрый старт

```bash
cd metrics-service
docker-compose up -d

# Проверить логи
docker-compose logs -f
# Должны увидеть: "Metrics gRPC server listening on 0.0.0.0:50051"
```

## Что сделано

Полноценный gRPC-сервер, который умеет:

1. **GetPageViews** — получать просмотры страниц
2. **GetClicks** — получать клики пользователей  
3. **GetPerformance** — получать метрики производительности (TTFB, FCP, LCP)
4. **GetErrors** — получать ошибки на клиенте
5. **GetCustomEvents** — получать кастомные события

Все методы поддерживают фильтрацию по `user_id`, `page`, `time_range` и пагинацию.

## Порты

| Сервис | Порт | Описание |
|--------|------|----------|
| gRPC сервер | 50051 | API для других сервисов |
| PostgreSQL | 5433 | База данных metrics_db |

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

## Как проверить

Использую grpcurl для тестирования:

```bash
# Получить все page views
grpcurl -plaintext -import-path ./proto -proto metrics.proto -d '{}' \
  localhost:50051 metricsys.MetricsService/GetPageViews

# Получить clicks
grpcurl -plaintext -import-path ./proto -proto metrics.proto -d '{}' \
  localhost:50051 metricsys.MetricsService/GetClicks

# Фильтрация по user_id
grpcurl -plaintext -import-path ./proto -proto metrics.proto -d '{
  "user_id_filter": "user-001"
}' localhost:50051 metricsys.MetricsService/GetPageViews
```

Или через aggregation-service:
```bash
cd ../aggregation-service/build
./test_grpc_connection localhost 50051
```

## Схема базы данных

Создаются 5 таблиц:
- `page_views` — просмотры страниц
- `click_events` — клики
- `performance_events` — метрики производительности
- `error_events` — ошибки
- `custom_events` — кастомные события

Management UI: http://localhost:15672 (guest/guest)

## Интеграция с aggregation-service

aggregation-service подключается к этому сервису через gRPC и запрашивает события для агрегации:

```
aggregation-service  ──gRPC:50051──►  metrics-service
                                            │
                                            ▼
                                      PostgreSQL:5433
```
