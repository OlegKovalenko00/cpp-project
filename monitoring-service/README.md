# Сервис мониторинга и логирования (Monitoring Service)

Сервис мониторинга отслеживает состояние всех сервисов через health-check эндпоинты и записывает логи о действиях сервисов и возможных ошибках в таблицу `logs`.

## Функциональность
- Health-check других сервисов (api-service, metrics-service, aggregation-service)
- Логирование событий и ошибок в PostgreSQL
- Эндпоинт `/status` для получения состояния всех сервисов

## Технологии

- **C++23**
- **cpp-httplib** — HTTP клиент и сервер
- **libpqxx** — PostgreSQL клиент
- **CMake + FetchContent** — сборка зависимостей

## Docker (рекомендуется)

```bash
cd monitoring-service
docker compose up --build -d
```

## Локальная сборка

### Требования
- macOS или Linux с установленным CMake 3.14+
- C++ компилятор с поддержкой C++23 (GCC 13+)
- PostgreSQL (libpq)
- pkg-config

### Установка зависимостей (macOS)

```bash
brew install cmake libpqxx pkg-config postgresql@14
```

### Сборка

```bash
cd monitoring-service
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

Исполняемый файл `monitoring-service` будет находиться в `build/`.

## Запуск

```bash
./build/monitoring-service
```

Сервер запустится на `http://localhost:8083`.

## Переменные окружения

| Переменная                 | По умолчанию           | Описание                 |
| -------------------------- | ---------------------- | ------------------------ |
| `HTTP_PORT`                | `8083`                 | Порт HTTP сервера        |
| `POSTGRES_HOST`            | `postgres-service`     | Хост PostgreSQL          |
| `POSTGRES_DB`              | `postgres`             | Имя базы данных          |
| `POSTGRES_USER`            | `postgres`             | Пользователь             |
| `POSTGRES_PASSWORD`        | `postgres`             | Пароль                   |
| `API_SERVICE_HOST`         | `host.docker.internal` | Хост api-service         |
| `API_SERVICE_PORT`         | `8080`                 | Порт api-service         |
| `METRICS_SERVICE_HOST`     | `host.docker.internal` | Хост metrics-service     |
| `METRICS_SERVICE_PORT`     | `8082`                 | Порт metrics-service     |
| `AGGREGATION_SERVICE_HOST` | `host.docker.internal` | Хост aggregation-service |
| `AGGREGATION_SERVICE_PORT` | `8081`                 | Порт aggregation-service |

## Схема БД

```sql
CREATE TABLE IF NOT EXISTS logs (
    id SERIAL PRIMARY KEY,
    service_name VARCHAR(255),
    status VARCHAR(10),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    details TEXT
);
```

## Структура проекта

```
monitoring-service/
├── src/
│   ├── main.cpp           # Точка входа
│   ├── monitor.cpp        # Логика мониторинга
│   ├── database.cpp       # Работа с БД
│   ├── logging.cpp        # Логирование
│   └── http_client.cpp    # HTTP-клиент
├── include/
│   ├── monitor.h          # Интерфейс мониторинга
│   ├── http_client.h      # Интерфейс HTTP-клиента
│   ├── logging.h          # Интерфейс логирования
│   └── database.h         # Интерфейс БД
├── tests/
│   ├── test_monitor.cpp
│   ├── test_http_client.cpp
│   └── test_database.cpp
├── CMakeLists.txt
├── Dockerfile
├── docker-compose.yml
└── README.md
```
