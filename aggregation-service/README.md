# Aggregation Service

Сервис агрегации метрик для системы аналитики. Получает сырые события от `metrics-service` через gRPC, агрегирует их в 5-минутные бакеты и сохраняет в PostgreSQL.

## Архитектура

```
┌─────────────────────┐       gRPC        ┌─────────────────────┐
│ aggregation-service │ ◄──────────────── │   metrics-service   │
│                     │                   │                     │
│   MetricsClient     │   RawEvents       │  MetricsService     │
│   Aggregator        │                   │                     │
│   Database          │                   │                     │
└─────────┬───────────┘                   └─────────────────────┘
          │
          ▼
┌─────────────────────┐
│     PostgreSQL      │
│  (aggregation_db)   │
│                     │
│  - agg_page_views   │
│  - agg_clicks       │
│  - agg_performance  │
│  - agg_errors       │
│  - agg_custom_events│
│  - watermark        │
└─────────────────────┘
```

## Функциональность

### Типы агрегируемых событий

| Тип события | Агрегируемые метрики |
|-------------|---------------------|
| `page_view` | views_count, unique_users, unique_sessions |
| `click` | clicks_count по element_id |
| `performance` | avg/p95 для total_load, ttfb, fcp, lcp |
| `error` | errors_count, warning_count, critical_count по error_type |
| `custom` | events_count по event_name |

### Схема БД

**agg_page_views** — агрегация просмотров страниц
```sql
(time_bucket, project_id, page) → views_count, unique_users, unique_sessions
```

**agg_clicks** — агрегация кликов
```sql
(time_bucket, project_id, page, element_id) → clicks_count, unique_users, unique_sessions
```

**agg_performance** — агрегация метрик производительности
```sql
(time_bucket, project_id, page) → samples_count, avg/p95 для ttfb, fcp, lcp, total_load
```

**agg_errors** — агрегация ошибок
```sql
(time_bucket, project_id, page, error_type) → errors_count, warning_count, critical_count
```

**agg_custom_events** — агрегация кастомных событий
```sql
(time_bucket, project_id, event_name, page) → events_count, unique_users, unique_sessions
```

**aggregation_watermark** — точка последней агрегации
```sql
(id=1) → last_aggregated_at
```

## Сборка

### Зависимости

- CMake >= 3.16
- C++23 compiler (GCC/Clang)
- PostgreSQL client library (libpq)
- gRPC и Protobuf
- Docker (для PostgreSQL)

### Установка зависимостей (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    libpq-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc
```

### Сборка проекта

```bash
# Из корня проекта cpp-project
mkdir -p build && cd build
cmake ..
make aggregation-service
```

### Запуск PostgreSQL

```bash
cd aggregation-service
docker-compose up -d
```

PostgreSQL будет доступен на `localhost:5434`:
- Database: `aggregation_db`
- User: `agguser`
- Password: `aggpassword`

### Запуск сервиса

```bash
./build/aggregation-service/aggregation-service
```

### Переменные окружения

| Переменная | По умолчанию | Описание |
|------------|--------------|----------|
| `AGG_DB_HOST` | `localhost` | Хост PostgreSQL |
| `AGG_DB_PORT` | `5434` | Порт PostgreSQL |
| `AGG_DB_NAME` | `aggregation_db` | Имя базы данных |
| `AGG_DB_USER` | `agguser` | Пользователь БД |
| `AGG_DB_PASSWORD` | `aggpassword` | Пароль БД |

## Структура проекта

```
aggregation-service/
├── CMakeLists.txt          # Конфигурация сборки
├── docker-compose.yml      # PostgreSQL контейнер
├── Dockerfile              # Docker образ сервиса
├── include/
│   ├── aggregator.h        # Логика агрегации, структуры данных
│   ├── database.h          # Работа с PostgreSQL
│   └── metrics_client.h    # gRPC клиент к metrics-service
├── src/
│   ├── main.cpp            # Точка входа
│   ├── aggregator.cpp      # Реализация агрегации
│   ├── database.cpp        # Реализация работы с БД
│   └── metrics_client.cpp  # Реализация gRPC клиента
└── tests/
    ├── test_aggregator.cpp # Тесты агрегации
    └── test_database.cpp   # Тесты БД
```

## Алгоритмы агрегации

- **calculateAverage()** — среднее арифметическое
- **calculateP95()** — 95-й перцентиль (сортировка + индекс 0.95*n)
- **calculateMin() / calculateMax()** — минимум/максимум
- **Unique counts** — через `std::unordered_set`

## Watermark

Сервис использует паттерн **watermark** для отслеживания прогресса:
1. Читает `last_aggregated_at` из таблицы `aggregation_watermark`
2. Запрашивает события от `metrics-service` за период `[watermark, now]`
3. Агрегирует и записывает результаты
4. Обновляет watermark на текущее время

Это гарантирует, что события не будут агрегированы повторно.

## Тестирование

```bash
cd build
make test_aggregator test_database
ctest --output-on-failure
```
