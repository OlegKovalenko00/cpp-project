### ✅ Issue #1 — Инициализировать репозиторий и структуру проекта

**Labels:** `setup`, `priority: high`

**Описание:**

* Создать репозиторий `api-service`.
* Добавить базовую структуру:

```text
api-service/
  src/
    main.cpp
    api.cpp
  include/
    api.h
    database.h
  tests/
    test_api.cpp
    test_database.cpp
  CMakeLists.txt
  Dockerfile
  README.md
  TODO.md
```

* В `main.cpp` сделать минимальный запуск HTTP-сервера или заглушки: вывод `"API service started"`.

**Acceptance criteria:**

* Проект собирается локально через CMake.
* Бинарник запускается и выводит базовое сообщение.

---

### ✅ Issue #2 — Выбор HTTP-библиотеки и базовый HTTP-сервер

**Labels:** `backend`, `network`, `priority: high`

**Описание:**

* Выбрать HTTP-библиотеку для C++ (например, `cpp-httplib`, `cpprestsdk`, `boost::beast` — то, что удобно).
* Реализовать минимальный HTTP-сервер:

  * слушает заданный порт (например, 8080),
  * отвечает на `GET /ping` строкой `"pong"`.

**Acceptance criteria:**

* При запуске сервиса можно сделать `curl http://localhost:8080/ping` и получить `"pong"`.
* Сервер корректно обрабатывает несколько последовательных запросов.

---

### ✅ Issue #3 — Настроить подключение к PostgreSQL

**Labels:** `backend`, `database`, `priority: high`

**Описание:**

* Реализовать модуль работы с БД (`database.h` / `database.cpp`).
* Подключение через `libpqxx`.
* Настройки подключения брать из env-переменных:

  * `POSTGRES_HOST`
  * `POSTGRES_DB`
  * `POSTGRES_USER`
  * `POSTGRES_PASSWORD`
* Сделать тестовый запрос `SELECT 1` при старте сервиса.

**Acceptance criteria:**

* Сервис при старте пытается подключиться к базе.
* При успешном подключении логируется успех.
* При ошибке — понятное сообщение, но сервис не обязательно падает.

---

### ✅ Issue #4 — Реализовать функции получения метрик из БД

**Labels:** `backend`, `database`, `priority: high`

**Описание:**

* Добавить в `database.h` / `database.cpp` функции:

```cpp
struct MetricDto {
    std::string name;
    double value;
    std::string timestamp; // или std::chrono, но для API понадобится строка
};

struct AggregatedMetricDto {
    std::string name;
    double avg;
    double min;
    double max;
    std::string timestamp;
};

std::vector<MetricDto> get_latest_metrics(std::size_t limit);
std::vector<AggregatedMetricDto> get_aggregated_metrics(std::size_t limit);
```

* Для `get_latest_metrics`:

  * выбирать последние N записей из `metrics` по timestamp.
* Для `get_aggregated_metrics`:

  * выбирать последние N записей из `aggregated_metrics`.

**Acceptance criteria:**

* Функции возвращают данные из БД.
* При отсутствии данных возвращается пустой вектор.
* Ошибки SQL логируются.

---

### ✅ Issue #5 — Реализовать эндпоинт `GET /metrics/latest`

**Labels:** `backend`, `api`, `priority: high`

**Описание:**

* В `api.cpp`/`api.h` реализовать обработчик для `GET /metrics/latest`.
* Логика:

  * вызывать `get_latest_metrics(limit)`, где `limit` можно взять:

    * либо константой (например 100),
    * либо из query-параметра `?limit=...` (если удобно).
  * вернуть JSON-массив с метриками, формат примерно:

```json
[
  {
    "name": "request_count",
    "value": 123,
    "timestamp": "2025-11-29T10:23:45Z"
  }
]
```

**Acceptance criteria:**

* При запросе `GET /metrics/latest` API возвращает JSON со списком метрик.
* При отсутствии данных — пустой массив `[]`.
* При ошибке БД — корректный HTTP-код (например, 500) и описание проблемы.

---

### ✅ Issue #6 — Реализовать эндпоинт `GET /metrics/aggregated`

**Labels:** `backend`, `api`, `priority: high`

**Описание:**

* Реализовать обработчик `GET /metrics/aggregated`.
* Логика:

  * вызывать `get_aggregated_metrics(limit)`.
  * возвращать JSON-массив вида:

```json
[
  {
    "name": "request_count",
    "avg": 123.4,
    "min": 5,
    "max": 350,
    "timestamp": "2025-11-29T10:00:00Z"
  }
]
```

**Acceptance criteria:**

* При запросе `GET /metrics/aggregated` возвращается корректный JSON.
* При отсутствии данных — `[]`.
* При ошибке БД — HTTP 500 и сообщение.

---

### ✅ Issue #7 — Обработка ошибок и коды ответов API

**Labels:** `backend`, `api`, `error-handling`, `priority: medium`

**Описание:**

* Продумать и реализовать базовую схему ошибок API:

  * 200 — успешный ответ.
  * 400 — некорректные параметры (если есть).
  * 500 — внутренняя ошибка (БД, неожиданные исключения).
* В случае ошибок возвращать JSON, например:

```json
{
  "error": "Database error",
  "details": "..."
}
```

* Логировать ошибки на стороне сервера.

**Acceptance criteria:**

* Неверные запросы/параметры не крашат сервис.
* Клиент получает осмысленный HTTP-код и JSON-описание ошибки.

---

### ✅ Issue #8 — Написать базовые тесты для API

**Labels:** `tests`, `priority: medium`

**Описание:**

* Добавить тесты для:

  * эндпоинта `/metrics/latest` (можно через http-клиент или интеграционные тесты),
  * эндпоинта `/metrics/aggregated`.
* В упрощённом варианте можно:

  * поднять сервер на тестовом порту,
  * сделать к нему HTTP-запросы из теста.

**Acceptance criteria:**

* `ctest` запускается и выполняет минимум 1–2 теста API.
* Тесты проверяют формат ответа и базовую корректность.

---

### ✅ Issue #9 — Логирование запросов и ответов

**Labels:** `logging`, `priority: low`

**Описание:**

* Добавить логирование:

  * входящих запросов (метод, путь, параметры),
  * кода ответа,
  * времени обработки запроса (по возможности).
* Логи писать хотя бы в stdout.

**Acceptance criteria:**

* В логах видно, какие запросы приходят и как обрабатываются.
* Это не мешает работе сервиса даже при большом количестве запросов.

---

### ✅ Issue #10 — Dockerfile для api-service

**Labels:** `docker`, `infrastructure`, `priority: medium`

**Описание:**

* Написать `Dockerfile`, который:

  * устанавливает все зависимости (cmake, компилятор, HTTP-библиотека, `libpqxx`),
  * собирает проект через CMake,
  * стартует API сервис (слушает порт, например 8080).
* Настроить для контейнера использование env-переменных Postgres.

**Acceptance criteria:**

* `docker build` для api-service проходит успешно.
* Контейнер при запуске поднимает сервер и отвечает на `GET /ping`.

---

### ✅ Issue #11 — Интеграция `api-service` в общий `docker-compose` и документация

**Labels:** `docker`, `integration`, `documentation`, `priority: medium`

**Описание:**

* Добавить `api-service` в общий `docker-compose.yml`:

  * зависимость от `postgres`,
  * настройки env-переменных,
  * проброс порта наружу (например, `8080:8080`).
* Обновить `README.md` для `api-service`:

  * краткое описание сервиса,
  * как собрать,
  * как запустить локально и через Docker,
  * примеры запросов:

    * `GET /metrics/latest`
    * `GET /metrics/aggregated`
* Обновить `TODO.md` (идеи: фильтрация по имени метрики, временные диапазоны, пагинация, аутентификация).

**Acceptance criteria:**

* При запуске всего проекта через `docker-compose up` API доступен с хоста, например:

  * `curl http://localhost:8080/metrics/latest`
* Документация даёт понятное описание того, как пользоваться сервисом.

---

Если хочешь, дальше могу:

* сделать сводную таблицу/README по всем сервисам с ссылками на их issues,
* или помочь разложить эти issues по milestone’ам (до 8.12, до 20.12 и т.д.).
