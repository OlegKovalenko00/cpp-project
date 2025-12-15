# MetricSys

**MetricSys** — микросервисная система для сбора, агрегации и предоставления метрик веб-аналитики.

Проект состоит из четырёх независимых C++-сервисов, взаимодействующих через REST API, RabbitMQ, gRPC и PostgreSQL.

## Быстрый старт

```bash
chmod +x build.sh  
./build.sh   
```

## Запуск клиента
```bash
pip3 install faker pyyaml requests
python3 client.py --base-url http://localhost:8080 --rate 2 --workers 1
```

## Архитектура

```
┌─────────────┐     RabbitMQ      ┌───────────────────┐
│ api-service │──────────────────►│  metrics-service  │
│ (HTTP:8080) │   message queue   │ (gRPC:50051)      │
└─────────────┘                   │ (HTTP:8082)       │
                                  └─────────┬─────────┘
                                            │
                                            │ gRPC
                                            ▼
                                  ┌───────────────────┐
                                  │aggregation-service│
                                  │   (HTTP:8081)     │
                                  └─────────┬─────────┘
                                            │
                                            ▼
┌─────────────────────────────────────────────────────┐
│                    PostgreSQL                       │
│   metrics_db:5433   │   aggregation_db:5434         │
└─────────────────────────────────────────────────────┘
        ▲
        │
┌───────┴───────┐
│  monitoring   │
│    service    │
│  (HTTP:8083)  │
└───────────────┘
```

## Сервисы

| Сервис                  | Порты                          | Описание                                           |
| ----------------------- | ------------------------------ | -------------------------------------------------- |
| **api-service**         | HTTP:8080                      | REST API для приёма метрик, отправка в RabbitMQ    |
| **metrics-service**     | gRPC:50051, HTTP:8082, PG:5433 | Приём из RabbitMQ, хранение в PostgreSQL, gRPC API |
| **aggregation-service** | HTTP:8081, PG:5434             | Агрегация метрик в 5-минутные бакеты через gRPC    |
| **monitoring-service**  | HTTP:8083                      | Health-check всех сервисов и логирование           |

## Типы метрик

- **Page Views** — просмотры страниц
- **Clicks** — клики по элементам
- **Performance** — TTFB, FCP, LCP, total page load
- **Errors** — ошибки на клиенте (warning/error/critical)
- **Custom Events** — кастомные события

## Технологии

* C++23
* gRPC + Protobuf
* RabbitMQ (rabbitmq-c)
* PostgreSQL (libpqxx)
* Docker / Docker Compose
* CMake + FetchContent
* cpp-httplib (HTTP сервер)
* nlohmann/json

## Структура проекта

```
cpp-project/
├── proto/
│   └── metrics.proto         # Общий proto для gRPC
├── api-service/              # REST API + RabbitMQ publisher
├── metrics-service/          # RabbitMQ consumer + gRPC server
├── aggregation-service/      # Агрегация метрик
├── monitoring-service/       # Мониторинг
├── build.sh                  # Скрипт сборки всех сервисов
├── client.py                 # Тестовый клиент
└── README.md
```

## Документация

Подробные инструкции в README каждого сервиса:
- [api-service/README.md](api-service/README.md)
- [metrics-service/README.md](metrics-service/README.md)
- [aggregation-service/README.md](aggregation-service/README.md)
- [monitoring-service/README.md](monitoring-service/README.md)

