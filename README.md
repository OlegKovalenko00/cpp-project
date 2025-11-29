# MetricSys

**MetricSys** — микросервисная система для сбора, агрегации и предоставления метрик, а также мониторинга состояния сервисов.

Проект состоит из четырёх независимых C++-сервисов, взаимодействующих через PostgreSQL и запускаемых в Docker.

## Архитектура

- **metrics-service** — сбор и запись метрик в базу данных.
- **aggregation-service** — агрегация метрик (среднее, минимум, максимум) и сохранение результатов.
- **api-service** — REST API для получения метрик и агрегированных данных.
- **monitoring-service** — health-check сервисов и логирование в БД.

База данных: **PostgreSQL**

Контейнеризация: **Docker + Docker Compose**

## Структура проекта

```

metrics-service/
aggregation-service/
api-service/
monitoring-service/
docker-compose.yml

````

## Запуск

```bash
docker-compose up --build
````

## Используемые технологии

* C++
* PostgreSQL
* Docker / Docker Compose
* CMake

