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
Connected to database successfully
Database schema initialized successfully (5 tables + watermark)
Connecting to metrics-service via gRPC at localhost:50051
MetricsClient: gRPC channel is ready
Aggregator::run() started
HTTP server listening on 0.0.0.0:8081
Fetching events from metrics-service via gRPC...
MetricsClient: received X page_view events
...
Aggregation completed successfully. Watermark updated.
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

### Тест gRPC соединения

```bash
./build/test_grpc_connection localhost 50051
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
│                     │    (port 50051)   │                     │
│   MetricsClient     │   RawEvents       │  MetricsService     │
│   Aggregator        │                   │  PostgreSQL:5433    │
│   Database          │                   │                     │
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

## Переменные окружения

| Переменная | По умолчанию | Описание |
|------------|--------------|----------|
| `AGG_DB_HOST` | `localhost` | Хост PostgreSQL aggregation_db |
| `AGG_DB_PORT` | `5434` | Порт PostgreSQL |
| `AGG_DB_NAME` | `aggregation_db` | Имя базы данных |
| `AGG_DB_USER` | `agguser` | Пользователь БД |
| `AGG_DB_PASSWORD` | `aggpassword` | Пароль БД |
| `METRICS_GRPC_HOST` | `localhost` | Хост metrics-service |
| `METRICS_GRPC_PORT` | `50051` | Порт gRPC metrics-service |
| `AGG_HTTP_HOST` | `0.0.0.0` | Хост HTTP сервера |
| `AGG_HTTP_PORT` | `8081` | Порт HTTP сервера |

## Типы агрегируемых событий

| Тип события | Агрегируемые метрики |
|-------------|---------------------|
| `page_view` | views_count, unique_users, unique_sessions |
| `click` | clicks_count по element_id |
| `performance` | avg/p95 для total_load, ttfb, fcp, lcp |
| `error` | errors_count, warning_count, critical_count по error_type |
| `custom` | events_count по event_name |

## Структура проекта

```
aggregation-service/
├── CMakeLists.txt          # Конфигурация сборки
├── docker-compose.yml      # PostgreSQL контейнер (порт 5434)
├── Dockerfile              # Docker образ сервиса
├── README.md               # Документация
├── scripts/
│   └── add_test_data.sh    # Скрипт добавления тестовых данных
├── include/
│   ├── aggregator.h        # Логика агрегации, структуры данных
│   ├── database.h          # Работа с PostgreSQL
│   ├── handlers.h          # HTTP endpoints
│   └── metrics_client.h    # gRPC клиент к metrics-service
├── src/
│   ├── main.cpp            # Точка входа
│   ├── aggregator.cpp      # Реализация агрегации
│   ├── database.cpp        # Реализация работы с БД
│   ├── handlers.cpp        # HTTP handlers
│   └── metrics_client.cpp  # Реализация gRPC клиента
└── tests/
    ├── test_aggregator.cpp     # Тесты агрегации
    ├── test_database.cpp       # Тесты БД
    └── test_grpc_connection.cpp # Тест gRPC соединения
```

## Как работает Watermark

Сервис использует паттерн **watermark** для инкрементальной агрегации:

1. Читает `last_aggregated_at` из таблицы `aggregation_watermark`
2. Запрашивает события от `metrics-service` за период `[watermark, now]`
3. Агрегирует события в 5-минутные бакеты
4. Записывает результаты в соответствующие таблицы
5. Обновляет watermark на текущее время

Это гарантирует, что события не будут агрегированы повторно.

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
