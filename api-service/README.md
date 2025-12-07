# API сервис (API Service)

API сервис для приёма метрик с фронтенда через REST API.

Полная спецификация API описана в файле [`openapi.yaml`](openapi.yaml).

## Эндпоинты

| Метод | Путь             | Описание                                 |
| ----- | ---------------- | ---------------------------------------- |
| GET   | `/health/ping`   | Health check                             |
| POST  | `/page-views`    | Отправка события просмотра страницы      |
| POST  | `/clicks`        | Отправка события клика/UI взаимодействия |
| POST  | `/performance`   | Отправка метрик производительности       |
| POST  | `/errors`        | Отправка события ошибки фронтенда        |
| POST  | `/custom-events` | Отправка кастомного события              |

## Технологии

- **C++23**
- **cpp-httplib** — HTTP сервер
- **nlohmann/json** — работа с JSON

## Сборка и запуск

### Требования

- CMake 3.10+
- Компилятор с поддержкой C++23 (GCC 13+, Clang 16+, MSVC 2022+)

### Сборка

```bash
cd api-service
mkdir -p build && cd build
cmake ..
make
```

### Запуск сервера

```bash
./api-service
```

Сервер запустится на `http://localhost:8080`.

## Тестирование

### Запуск тестов

```bash
# Установить зависимость
pip install requests

# Запустить тесты (сервер должен быть запущен)
python3 tests/test_handlers.py
```

### Пример запроса

```bash
# Health check
curl http://localhost:8080/health/ping

# Отправка page view
curl -X POST http://localhost:8080/page-views \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "timestamp": 1733505600}'
```

## Структура проекта

```
api-service/
├── CMakeLists.txt
├── openapi.yaml          # OpenAPI спецификация
├── README.md
├── include/
│   ├── handlers.hpp      # Объявления handlers
│   └── models.hpp        # Модели данных
├── src/
│   ├── main.cpp          # Точка входа
│   ├── handlers.cpp      # Реализация handlers
│   └── models.cpp        # Сериализация моделей
└── tests/
    └── test_handlers.py  # Python тесты
```
