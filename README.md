# MetricSys

**MetricSys** — микросервисная система для сбора, агрегации и предоставления метрик веб-аналитики.

Проект состоит из четырёх независимых C++-сервисов, взаимодействующих через gRPC и PostgreSQL.

## Быстрый старт

```bash
chmod +x run.sh  
./run.sh   
```

## Архитектура

```
┌─────────────┐     RabbitMQ  ┌───────────────────┐
│ api-service │◄─────────────►│  metrics-service  │
│  (REST API) │               │   (gRPC:50051)    │
└──────┬──────┘               └─────────┬─────────┘
       │                                │
       │                                │ gRPC
       │                                ▼
       │                      ┌───────────────────┐
       │                      │aggregation-service│
       │                      │   (HTTP:8081)     │
       │                      └─────────┬─────────┘
       │                                │
       ▼                                ▼
┌─────────────────────────────────────────────────┐
│                   PostgreSQL                    │
│  metrics_db:5433  │  aggregation_db:5434        │
└─────────────────────────────────────────────────┘
       ▲
       │
┌──────┴──────┐
│ monitoring  │
│   service   │
└─────────────┘
```

## Сервисы

| Сервис | Порт | Описание |
|--------|------|----------|
| **metrics-service** | gRPC:50051, PG:5433 | Сбор и хранение сырых метрик |
| **aggregation-service** | HTTP:8081, PG:5434 | Агрегация метрик в 5-минутные бакеты |
| **api-service** | HTTP:8080 | REST API для получения данных |
| **monitoring-service** | - | Health-check и логирование |

## Типы метрик

- **Page Views** — просмотры страниц
- **Clicks** — клики по элементам
- **Performance** — TTFB, FCP, LCP, total page load
- **Errors** — ошибки на клиенте (warning/error/critical)
- **Custom Events** — кастомные события

## Технологии

* C++23
* gRPC + Protobuf
* PostgreSQL
* Docker / Docker Compose
* CMake
* httplib (HTTP сервер)

## Структура проекта

```
cpp-project/
├── proto/
│   └── metrics.proto         # Общий proto для gRPC
├── metrics-service/          # Сбор метрик
├── aggregation-service/      # Агрегация метрик
├── api-service/              # REST API
├── monitoring-service/       # Мониторинг
└── README.md
```

## Документация

Подробные инструкции в README каждого сервиса:
- [metrics-service/README.md](metrics-service/README.md)
- [aggregation-service/README.md](aggregation-service/README.md)
- [api-service/README.md](api-service/README.md)
- [monitoring-service/README.md](monitoring-service/README.md)

