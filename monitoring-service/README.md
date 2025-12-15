# Сервис мониторинга и логирования (Monitoring Service)

Сервис мониторинга отслеживает состояние всех сервисов через health-check эндпоинты и записывает логи о действиях сервисов и возможных ошибках в таблицу `logs`.

## Функциональность
- Health-check других сервисов.
- Логирование событий и ошибок в БД.
- Эндпоинт `/status`.

## Требования
- macOS или Linux с установленным CMake 3.10+
- C++ компилятор с поддержкой C++23
- PostgreSQL (libpqxx 7.10.4+)
- pkg-config

### Установка зависимостей (macOS)

```bash
brew install cmake libpqxx pkg-config postgresql@14
```

## Сборка

```bash
# Перейти в директорию проекта
cd monitoring-service

# Создать и настроить директорию build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Собрать проект
cmake --build build -- -j$(sysctl -n hw.ncpu)
```

Исполняемый файл `monitoring-service` будет находиться в `build/`.

## Запуск

```bash
# Запустить сервис
./build/monitoring-service

# Или через CMake
cmake --build build --target run
```

## Конфигурация

Перед запуском убедитесь, что PostgreSQL доступен и таблица `logs` создана:

```sql
CREATE TABLE IF NOT EXISTS logs (
    id SERIAL PRIMARY KEY,
    service_name VARCHAR(255),
    status VARCHAR(10),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    details TEXT
);
```

### Переменные окружения

- `HTTP_PORT` — порт HTTP-сервера (по умолчанию `8083`).
- `POSTGRES_HOST` / `POSTGRES_PORT` — адрес БД (по умолчанию `localhost:5432`).
- `POSTGRES_DB`, `POSTGRES_USER`, `POSTGRES_PASSWORD` — реквизиты подключения (по умолчанию `postgres` / `postgres` / `postgres`).

### Эндпоинты

- `GET /health/ping` — liveness.
- `GET /health/ready` — readiness (проверяет подключение к БД).
- `GET /uptime?service=<name>[&period=day|week|month|year]` — количество успешных (`OK`) записей для сервиса за сутки/неделю/месяц/год. Если `period` не указан — вернутся все четыре.
- Короткие маршруты для конкретного периода: `GET /uptime/day|week|month|year?service=<name>`.

## Структура проекта

```
monitoring-service/
├── src/
│   ├── main.cpp           # Точка входа
│   ├── monitor.cpp        # Логика мониторинга
│   ├── database.cpp       # Работа с БД
│   └── http_client.cpp    # HTTP-клиент
├── include/
│   ├── monitor.h          # Интерфейс мониторинга
│   ├── http_client.h      # Интерфейс HTTP-клиента
│   └── database.h         # Интерфейс БД
├── tests/
│   ├── test_monitor.cpp
│   ├── test_http_client.cpp
│   └── test_database.cpp
├── CMakeLists.txt
├── Dockerfile
└── README.md
```
