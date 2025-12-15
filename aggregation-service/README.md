# Aggregation Service

Сервис агрегации метрик для системы аналитики. Получает сырые события от `metrics-service` через gRPC, агрегирует их в 5-минутные бакеты и сохраняет в PostgreSQL.

## Быстрый старт

### 1. Установка зависимостей (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    libpq-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    pkg-config \
    libssl-dev
```

### 2. Запуск metrics-service (обязательно первым!)

```bash
cd ../metrics-service
docker-compose up -d
```

Дождитесь запуска (проверьте логи):
```bash
docker-compose logs -f
# Должны увидеть: "Metrics gRPC server listening on 0.0.0.0:50051"
```

### 3. Запуск PostgreSQL для aggregation-service

```bash
cd ../aggregation-service
docker-compose up -d
```

### 4. Сборка проекта

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 5. Запуск сервиса

```bash
./aggregation-service
```

Ожидаемый вывод:
```
Aggregation Service started
Aggregation interval set to 60 seconds
Connected to database successfully
Database schema initialized successfully (5 tables + watermark)
[HTTP] Routes registered:
  GET /health/ping  - liveness probe
  GET /health/ready - readiness probe
Connecting to metrics-service via gRPC at localhost:50051
MetricsClient: gRPC channel is ready
HTTP server listening on 0.0.0.0:8081
Starting periodic aggregation loop...

=== Starting aggregation cycle ===
Aggregator::run() started
Watermark: last aggregated at epoch + 1765224298 seconds
Fetching events from metrics-service via gRPC...
MetricsClient: received X page_view events
...
Aggregation completed successfully. Watermark updated.
=== Aggregation cycle completed successfully ===

[60 секунд спустя начнется следующий цикл]

Press Ctrl+C to stop gracefully
```

### 6. Добавление тестовых данных (опционально)

```bash
# Добавить свежие данные в metrics-service
chmod +x scripts/add_test_data.sh
./scripts/add_test_data.sh

# Сбросить watermark чтобы получить все данные заново
docker exec -i aggregation-service_agg-postgres_1 psql -U agguser -d aggregation_db \
  -c "UPDATE aggregation_watermark SET last_aggregated_at = NOW() - INTERVAL '2 hours';"

# Перезапустить сервис
./build/aggregation-service
```

## Проверка работоспособности

### Health Check

```bash
curl http://localhost:8081/health/ping
# {"status":"ok"}

curl http://localhost:8081/health/ready
# {"status":"ready","database":"connected"}
```

### Тест gRPC соединения с metrics-service

```bash
./build/test_grpc_connection localhost 50051
```

### Тест gRPC сервера aggregation-service

```bash
# Запустите aggregation-service в одном терминале
./build/aggregation-service

# В другом терминале запустите тест
./build/test_aggregation_grpc_server localhost:50052
```

Ожидаемый вывод:
```
=== Testing Aggregation gRPC Server ===
Connecting to: localhost:50052

1. Testing GetWatermark()...
   ✓ Watermark retrieved successfully
   Last aggregated at: 1734374400 seconds since epoch

2. Testing GetPageViewsAgg()...
   Project ID: test-project
   Time range: last 24 hours
   ✓ Page views retrieved successfully
   Found 3 aggregated page view records

   Sample records:
   - Page: /home, Views: 15, Unique users: 8
   - Page: /products, Views: 10, Unique users: 5
   - Page: /checkout, Views: 5, Unique users: 3

=== Test Complete ===
```

### Просмотр агрегированных данных

```bash
docker exec -i aggregation-service_agg-postgres_1 psql -U agguser -d aggregation_db -c "
SELECT * FROM agg_page_views ORDER BY time_bucket DESC LIMIT 5;
SELECT * FROM agg_performance ORDER BY time_bucket DESC LIMIT 5;
"
```

## Архитектура

```
┌─────────────────────┐       gRPC        ┌─────────────────────┐
│ aggregation-service │ ◄──────────────── │   metrics-service   │
│                     │   (port 50051)    │                     │
│   MetricsClient     │   RawEvents       │  MetricsService     │
│   Aggregator        │                   │  PostgreSQL:5433    │
│   Database          │                   │                     │
│                     │                   └─────────────────────┘
│ AggregationService  │       gRPC
│   (gRPC Server)     │ ───────────────►  ┌─────────────────────┐
│   port 50052        │   Aggregated      │   api-service /     │
│                     │      Data         │ monitoring-service  │
└─────────┬───────────┘                   └─────────────────────┘
          │
          ▼
┌─────────────────────┐
│     PostgreSQL      │
│   (port 5434)       │
│  aggregation_db     │
│                     │
│  - agg_page_views   │
│  - agg_clicks       │
│  - agg_performance  │
│  - agg_errors       │
│  - agg_custom_events│
│  - watermark        │
└─────────────────────┘
```

### Роли сервиса

**aggregation-service** выполняет две роли:

1. **gRPC Клиент** (к metrics-service:50051):
   - Получает сырые события
   - Агрегирует их в 5-минутные бакеты
   - Сохраняет в свою БД

2. **gRPC Сервер** (порт 50052):
   - Предоставляет агрегированные данные
   - Используется api-service и monitoring-service
   - Методы: GetPageViewsAgg, GetClicksAgg, GetPerformanceAgg, GetErrorsAgg, GetCustomEventsAgg, GetWatermark

## Переменные окружения

| Переменная | По умолчанию | Описание |
|------------|--------------|----------|
| `AGG_DB_HOST` | `localhost` | Хост PostgreSQL aggregation_db |
| `AGG_DB_PORT` | `5434` | Порт PostgreSQL |
| `AGG_DB_NAME` | `aggregation_db` | Имя базы данных |
| `AGG_DB_USER` | `agguser` | Пользователь БД |
| `AGG_DB_PASSWORD` | `aggpassword` | Пароль БД |
| `METRICS_GRPC_HOST` | `localhost` | Хост metrics-service (клиент) |
| `METRICS_GRPC_PORT` | `50051` | Порт gRPC metrics-service (клиент) |
| `AGG_GRPC_HOST` | `0.0.0.0` | Хост gRPC сервера aggregation-service |
| `AGG_GRPC_PORT` | `50052` | Порт gRPC сервера aggregation-service |
| `AGG_HTTP_HOST` | `0.0.0.0` | Хост HTTP сервера |
| `AGG_HTTP_PORT` | `8081` | Порт HTTP сервера |
| `AGGREGATION_INTERVAL_SEC` | `60` | Интервал между циклами агрегации (секунды) |

### Примеры настройки

```bash
# Увеличить интервал агрегации до 5 минут
export AGGREGATION_INTERVAL_SEC=300

# Запустить с кастомной конфигурацией
AGGREGATION_INTERVAL_SEC=30 \
AGG_DB_HOST=postgres-server \
METRICS_GRPC_HOST=metrics-grpc \
./aggregation-service
```

## Типы агрегируемых событий

| Тип события | Агрегируемые метрики |
|-------------|---------------------|
| `page_view` | views_count, unique_users, unique_sessions |
| `click` | clicks_count по element_id |
| `performance` | avg/p95 для total_load, ttfb, fcp, lcp |
| `error` | errors_count, warning_count, critical_count по error_type |
| `custom` | events_count по event_name |

## gRPC API

aggregation-service предоставляет следующие gRPC методы (порт 50052):

### GetWatermark
Получить последнее время агрегации (watermark).

**Request:** `GetWatermarkRequest` (пустой)
**Response:** `GetWatermarkResponse`
- `last_aggregated_at`: timestamp последней агрегации

### GetPageViewsAgg
Получить агрегированные данные просмотров страниц.

**Request:** `GetPageViewsAggRequest`
- `project_id`: ID проекта (обязательно)
- `time_range`: временной диапазон (from, to)
- `page`: фильтр по странице (опционально)
- `pagination`: limit, offset

**Response:** `GetPageViewsAggResponse`
- `rows[]`: массив агрегатов с полями time_bucket, page, views_count, unique_users, unique_sessions

### GetClicksAgg
Получить агрегированные данные кликов.

**Request:** `GetClicksAggRequest`
- `project_id`: ID проекта
- `time_range`: временной диапазон
- `page`: фильтр по странице
- `element_id`: фильтр по элементу
- `pagination`: limit, offset

**Response:** `GetClicksAggResponse`
- `rows[]`: массив агрегатов с полями time_bucket, page, element_id, clicks_count, unique_users, unique_sessions

### GetPerformanceAgg
Получить агрегированные данные производительности.

**Request:** `GetPerformanceAggRequest`
- `project_id`: ID проекта
- `time_range`: временной диапазон
- `page`: фильтр по странице
- `pagination`: limit, offset

**Response:** `GetPerformanceAggResponse`
- `rows[]`: массив агрегатов с метриками avg/p95 для total_load, ttfb, fcp, lcp

### GetErrorsAgg
Получить агрегированные данные ошибок.

**Request:** `GetErrorsAggRequest`
- `project_id`: ID проекта
- `time_range`: временной диапазон
- `page`: фильтр по странице
- `error_type`: фильтр по типу ошибки
- `pagination`: limit, offset

**Response:** `GetErrorsAggResponse`
- `rows[]`: массив агрегатов с полями errors_count, warning_count, critical_count, unique_users

### GetCustomEventsAgg
Получить агрегированные данные кастомных событий.

**Request:** `GetCustomEventsAggRequest`
- `project_id`: ID проекта
- `time_range`: временной диапазон
- `event_name`: название события (обязательно)
- `page`: фильтр по странице
- `pagination`: limit, offset

**Response:** `GetCustomEventsAggResponse`
- `rows[]`: массив агрегатов с полями events_count, unique_users, unique_sessions

## Структура проекта

```
aggregation-service/
├── CMakeLists.txt          # Конфигурация сборки
├── docker-compose.yml      # PostgreSQL контейнер (порт 5434)
├── Dockerfile              # Docker образ сервиса
├── init.sql                # SQL схема базы данных
├── README.md               # Документация
├── scripts/
│   └── add_test_data.sh    # Скрипт добавления тестовых данных
├── include/
│   ├── aggregator.h        # Логика агрегации, структуры данных
│   ├── aggregation_server.h # gRPC сервер для API
│   ├── database.h          # Работа с PostgreSQL
│   ├── handlers.h          # HTTP endpoints
│   └── metrics_client.h    # gRPC клиент к metrics-service
├── src/
│   ├── main.cpp            # Точка входа
│   ├── aggregator.cpp      # Реализация агрегации
│   ├── aggregation_server.cpp # Реализация gRPC сервера
│   ├── database.cpp        # Реализация работы с БД
│   ├── handlers.cpp        # HTTP handlers
│   └── metrics_client.cpp  # Реализация gRPC клиента
└── tests/
    ├── test_aggregator.cpp     # Тесты агрегации
    ├── test_database.cpp       # Тесты БД
    ├── test_grpc_connection.cpp # Тест gRPC соединения (metrics-service)
    └── test_aggregation_grpc_server.cpp # Тест gRPC сервера
```

## Как работает сервис

### Периодическая агрегация

Сервис работает в **непрерывном режиме** с периодическим запуском агрегации:

1. При старте подключается к PostgreSQL и metrics-service
2. Запускает HTTP сервер для health checks
3. Входит в цикл агрегации с интервалом `AGGREGATION_INTERVAL_SEC` (по умолчанию 60 секунд)
4. При получении SIGINT/SIGTERM корректно завершает работу (graceful shutdown)

### Watermark механизм

Сервис использует паттерн **watermark** для инкрементальной агрегации:

1. Читает `last_aggregated_at` из таблицы `aggregation_watermark`
2. Запрашивает события от `metrics-service` за период `[watermark, now]`
3. Агрегирует события в 5-минутные бакеты
4. Записывает результаты в соответствующие таблицы
5. Обновляет watermark на текущее время

Это гарантирует, что события не будут агрегированы повторно.

### Обработка ошибок

Сервис устойчив к временным сбоям:

- **Ошибки gRPC**: логируются, агрегация пропускается, повторяется на следующем цикле
- **Ошибки БД**: логируются с полным стектрейсом, watermark не обновляется
- **Ошибки агрегации**: не прерывают работу сервиса, только текущий цикл
- **Graceful shutdown**: корректно завершает HTTP сервер и закрывает соединения

## База данных

### Схема БД

Схема базы данных определена в файле `init.sql`. При запуске через docker-compose, PostgreSQL автоматически выполняет этот файл для инициализации таблиц.

**Таблицы:**
- `agg_page_views` - агрегированные просмотры страниц
- `agg_clicks` - агрегированные клики
- `agg_performance` - агрегированные метрики производительности
- `agg_errors` - агрегированные ошибки
- `agg_custom_events` - агрегированные кастомные события
- `aggregation_watermark` - отслеживание прогресса агрегации

**Индексы:** Созданы индексы для оптимизации запросов по `time_bucket`, `project_id`, `page`.

### Ручная инициализация схемы

Если запускаете сервис локально (не через docker-compose):

```bash
# Убедитесь что init.sql находится в текущей директории или build/
cp init.sql build/

# Или можно выполнить SQL вручную
psql -h localhost -p 5434 -U agguser -d aggregation_db -f init.sql
```

Сервис автоматически пытается загрузить `init.sql` из следующих путей:
- `./init.sql`
- `../init.sql`
- `../../init.sql`
- `../aggregation-service/init.sql`

## Тестирование

```bash
cd build

# Запустить все тесты
make test_aggregator test_database test_grpc_connection
ctest --output-on-failure

# Или отдельно
./test_aggregator      # Тесты логики агрегации
./test_database        # Тесты работы с БД  
./test_grpc_connection # Тест gRPC соединения с metrics-service
```

## Устранение проблем

### "Connection refused" к metrics-service

```bash
# Проверить что metrics-service запущен
docker ps | grep metrics

# Проверить порт
nc -zv localhost 50051
```

### "0 events received" при запуске

Watermark может быть в будущем. Сбросьте его:
```bash
docker exec -i aggregation-service_agg-postgres_1 psql -U agguser -d aggregation_db \
  -c "UPDATE aggregation_watermark SET last_aggregated_at = NOW() - INTERVAL '2 hours';"
```

### Ошибка подключения к PostgreSQL

```bash
# Проверить контейнер
docker ps | grep agg-postgres

# Перезапустить
docker-compose down -v && docker-compose up -d
```
