# Metrics Service

Микросервис для сбора и хранения веб-аналитики. Написан на C++23 с использованием gRPC для коммуникации между сервисами.

## Что сделано

На данный момент реализован полноценный gRPC-сервер, который умеет:

1. Получать просмотры страниц (GetPageViews)
2. Получать клики пользователей (GetClicks)  
3. Получать метрики производительности - TTFB, FCP, LCP (GetPerformance)
4. Получать ошибки на клиенте (GetErrors)
5. Получать кастомные события (GetCustomEvents)

Все методы поддерживают фильтрацию по user_id, page, временному диапазону и пагинацию.

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

## Как запустить

```bash
cd metrics-service
docker compose up --build
```

Сервис запустится на порту 50051, PostgreSQL на 5432.

## Как проверить

Использую grpcurl для тестирования (запускать из корня проекта):

```bash
grpcurl -plaintext -import-path ./proto -proto metrics.proto -d '{}' \
  localhost:50051 metricsys.MetricsService/GetPageViews

grpcurl -plaintext -import-path ./proto -proto metrics.proto -d '{}' \
  localhost:50051 metricsys.MetricsService/GetClicks

grpcurl -plaintext -import-path ./proto -proto metrics.proto -d '{
  "user_id_filter": "user-001"
}' localhost:50051 metricsys.MetricsService/GetPageViews
```

## Схема базы данных

Создаются 5 таблиц:
- page_views - просмотры страниц
- click_events - клики
- performance_events - метрики производительности
- error_events - ошибки
- custom_events - кастомные события

При первом запуске автоматически добавляются тестовые данные.

## Особенности реализации

- Proto-файл лежит в общей папке proto/ и используется всеми сервисами
- Генерация gRPC кода происходит внутри Docker-контейнера
- libpqxx подтягивается через FetchContent (требование проекта)
- gRPC и Protobuf берутся из системных пакетов Ubuntu
