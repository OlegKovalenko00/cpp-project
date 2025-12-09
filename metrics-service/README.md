# Metrics Service

Микросервис для сбора, хранения и предоставления веб-аналитики. Часть распределённой системы мониторинга.

## Краткое описание

Сервис принимает события от api-service через RabbitMQ, сохраняет их в PostgreSQL и предоставляет данные другим сервисам через gRPC. Для мониторинга реализованы HTTP health-check эндпоинты.

## Реализованный функционал

| Компонент         | Что делает                                                       | Порт  |
| ----------------- | ---------------------------------------------------------------- | ----- |
| gRPC API          | 5 методов для получения метрик с фильтрацией и пагинацией        | 50051 |
| HTTP API          | Health checks для monitoring-service (liveness/readiness probes) | 8082  |
| RabbitMQ Consumer | Приём событий из 5 очередей                                      | —     |
| PostgreSQL        | 5 таблиц для разных типов событий                                | 5433  |

## Архитектура взаимодействия

```
                    события                      
api-service ──────────────────► RabbitMQ ──────► metrics-service
     ▲                                                 │
     │                                                 │ сохранение
     │                                                 ▼
     │                                            PostgreSQL
     │              gRPC запрос                        │
     └─────────────── aggregation-service ◄────────────┘
                           │                      чтение данных
                           │
                    monitoring-service
                   (HTTP health checks)
```

**Поток данных:**
1. api-service отправляет события в RabbitMQ (5 очередей)
2. metrics-service читает из очередей и сохраняет в PostgreSQL
3. aggregation-service запрашивает данные у metrics-service через gRPC
4. monitoring-service проверяет здоровье всех сервисов через HTTP

## Стек технологий

- **Язык:** C++23
- **API:** gRPC + Protobuf, cpp-httplib (HTTP)
- **Очереди:** rabbitmq-c (RabbitMQ consumer)
- **БД:** PostgreSQL + libpqxx
- **Сборка:** CMake + FetchContent (libpqxx, cpp-httplib, nlohmann/json)
- **Деплой:** Docker + Docker Compose

## Очереди RabbitMQ

Сервис подписывается на следующие очереди:
- `page_views` — просмотры страниц
- `clicks` — клики
- `performance_events` — метрики производительности
- `error_events` — ошибки
- `custom_events` — кастомные события

## Структура файлов

```
metrics-service/
├── include/           — заголовочные файлы
│   ├── database.h     — конфигурация БД
│   ├── metrics.h      — gRPC сервис
│   ├── rabbitmq.h     — RabbitMQ consumer
│   └── http_handler.h — HTTP сервер
├── src/               — реализация
│   ├── main.cpp       — точка входа, инициализация всех компонентов
│   ├── database.cpp   — подключение к PostgreSQL
│   ├── metrics.cpp    — реализация 5 gRPC методов
│   ├── rabbitmq.cpp   — подключение и потребление из RabbitMQ
│   └── http_handler.cpp — HTTP эндпоинты
├── init.sql           — DDL таблиц и тестовые данные
├── Dockerfile         — multi-stage сборка
├── docker-compose.yml — оркестрация сервисов (postgres, rabbitmq, metrics-service)
└── CMakeLists.txt     — конфигурация сборки
```

## Запуск

```bash
cd metrics-service
docker compose up --build
```

## Проверка работоспособности

```bash
# HTTP health checks
curl http://localhost:8082/health/ping
curl http://localhost:8082/health/ready

# gRPC (из корня проекта)
grpcurl -plaintext -import-path ./proto -proto metrics.proto \
  -d '{}' localhost:50051 metricsys.MetricsService/GetPageViews

# Проверка очередей RabbitMQ
docker exec metrics-rabbitmq rabbitmqctl list_queues name messages consumers
```

## API

### gRPC методы

| Метод             | Описание               |
| ----------------- | ---------------------- |
| `GetPageViews`    | Просмотры страниц      |
| `GetClicks`       | Клики пользователей    |
| `GetPerformance`  | Метрики TTFB, FCP, LCP |
| `GetErrors`       | Ошибки на клиенте      |
| `GetCustomEvents` | Кастомные события      |

### HTTP эндпоинты

| Эндпоинт            | Назначение                     | Код     |
| ------------------- | ------------------------------ | ------- |
| `GET /health/ping`  | Liveness probe                 | 200     |
| `GET /health/ready` | Readiness probe (проверяет БД) | 200/503 |

## Схема БД

5 таблиц: `page_views`, `click_events`, `performance_events`, `error_events`, `custom_events`

Все таблицы создаются автоматически при первом запуске через `init.sql`.
