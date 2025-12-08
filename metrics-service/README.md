# Metrics Service

Микросервис для сбора и хранения веб-аналитики. Написан на C++23 с использованием gRPC для коммуникации между сервисами.

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

## Стек технологий

- C++23
- gRPC + Protobuf для API
- PostgreSQL для хранения данных
- libpqxx для работы с БД
- Docker + Docker Compose для деплоя
- CMake для сборки

## Структура проекта

```
metrics-service/
├── include/
│   ├── database.h      # Конфигурация подключения к БД
│   └── metrics.h       # gRPC сервис
├── src/
│   ├── main.cpp        # Точка входа, запуск сервера
│   ├── database.cpp    # Работа с PostgreSQL
│   └── metrics.cpp     # Реализация gRPC методов
├── init.sql            # Схема БД и тестовые данные
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

При первом запуске автоматически добавляются тестовые данные.

## Интеграция с aggregation-service

aggregation-service подключается к этому сервису через gRPC и запрашивает события для агрегации:

```
aggregation-service  ──gRPC:50051──►  metrics-service
                                            │
                                            ▼
                                      PostgreSQL:5433
```
