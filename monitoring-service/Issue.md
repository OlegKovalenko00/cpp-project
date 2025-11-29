### ✅ Issue #1 — Инициализировать репозиторий и структуру проекта

**Labels:** `setup`, `priority: high`

**Описание:**

* Создать репозиторий `monitoring-service`.
* Добавить базовую структуру:

```text
monitoring-service/
  src/
    main.cpp
    monitor.cpp
  include/
    monitor.h
    http_client.h
    database.h
  tests/
    test_monitor.cpp
    test_http_client.cpp
    test_database.cpp
  CMakeLists.txt
  Dockerfile
  README.md
  TODO.md
```

* В `main.cpp` сделать минимальный запуск сервиса: вывод `"Monitoring service started"`.

**Acceptance criteria:**

* Проект собирается локально через CMake.
* Запуск бинарника работает и выводит базовое сообщение.

---

### ✅ Issue #2 — Настроить подключение к PostgreSQL и работу с таблицей logs

**Labels:** `backend`, `database`, `priority: high`

**Описание:**

* Реализовать модуль работы с БД (`database.h` / `database.cpp`).
* Подключение к PostgreSQL через `libpqxx`.
* Конфиг из env-переменных:

  * `POSTGRES_HOST`
  * `POSTGRES_DB`
  * `POSTGRES_USER`
  * `POSTGRES_PASSWORD`
* Использовать существующую таблицу `logs` или создать, если нет:

```sql
CREATE TABLE IF NOT EXISTS logs (
    id SERIAL PRIMARY KEY,
    service_name VARCHAR(255) NOT NULL,
    log_message TEXT NOT NULL,
    timestamp TIMESTAMP NOT NULL
);
```

* Реализовать функцию:

```cpp
void write_log(const std::string& service_name,
               const std::string& message);
```

**Acceptance criteria:**

* При вызове `write_log(...)` в таблице `logs` появляется запись.
* Ошибки БД логируются в stdout/stderr.

---

### ✅ Issue #3 — Описать модели для статуса сервисов и логов

**Labels:** `backend`, `design`

**Описание:**

* В `monitor.h` описать структуру для статуса сервиса:

```cpp
enum class ServiceHealth {
    OK,
    FAIL,
    UNKNOWN
};

struct ServiceStatus {
    std::string name;
    std::string url;
    ServiceHealth health;
    std::string last_error;
    std::chrono::system_clock::time_point last_check;
};
```

* При необходимости — структуру для логической записи лога (для удобства), либо использовать сразу строки в `write_log`.

**Acceptance criteria:**

* Структуры объявлены, проект компилируется.
* Эти структуры будут использоваться в логике мониторинга.

---

### ✅ Issue #4 — Реализовать HTTP-клиент для health-check других сервисов

**Labels:** `backend`, `network`, `priority: high`

**Описание:**

* Реализовать простой HTTP-клиент в `http_client.h` / `http_client.cpp` (можно использовать любую выбранную либу, например cpp-httplib, cpprestsdk или curl).
* Функция:

```cpp
ServiceHealth check_health(const std::string& url,
                           std::string& error_message);
```

* Ожидается, что другие сервисы (metrics, aggregation, api) имеют эндпоинт вида `/health` или `/status`, возвращающий 200 при нормальной работе.

* Адреса сервисов брать из env-переменных:

  * `METRICS_SERVICE_URL`
  * `AGGREGATION_SERVICE_URL`
  * `API_SERVICE_URL`

**Acceptance criteria:**

* При вызове `check_health(url, error)` происходит HTTP-запрос.
* При HTTP 200 — возвращается `ServiceHealth::OK`.
* При ошибке соединения/не 200 — `ServiceHealth::FAIL` и заполненный `error_message`.

---

### ✅ Issue #5 — Реализовать функцию проверки всех сервисов

**Labels:** `backend`, `logic`, `priority: high`

**Описание:**

* В `monitor.cpp` реализовать функцию:

```cpp
std::vector<ServiceStatus> check_all_services();
```

* Эта функция:

  * читает список сервисов и их URL из env или конфигурации,
  * вызывает `check_health` для каждого,
  * формирует `ServiceStatus` с:

    * именем сервиса,
    * url,
    * статусом,
    * последней ошибкой (если есть),
    * временем проверки.
* При сбое какого-либо сервиса — писать лог через `write_log("monitoring-service", "...")`.

**Acceptance criteria:**

* Вызов `check_all_services()` возвращает валидный список статусов.
* При падении или проблеме сервиса появляется запись в таблице `logs`.

---

### ✅ Issue #6 — Реализовать эндпоинт `/status` у monitoring-service

**Labels:** `backend`, `api`, `priority: medium`

**Описание:**

* Сделать в `monitoring-service` HTTP-сервис с минимальным API:

  * `GET /status` — возвращает JSON со сводкой по всем сервисам:

    * имя сервиса,
    * url,
    * статус (`OK` / `FAIL` / `UNKNOWN`),
    * последнее время проверки.
* Формат ответа — простой JSON-массив объектов.

Пример (условный):

```json
[
  {
    "name": "metrics",
    "url": "http://metrics-service:8080/health",
    "status": "OK",
    "last_check": "2025-11-29T12:34:56Z"
  }
]
```

**Acceptance criteria:**

* При запросе `GET /status` monitoring-service выполняет `check_all_services()` и отдаёт актуальный результат.
* В случае ошибок всё равно возвращается корректный JSON (со статусом FAIL для соответствующих сервисов).

---

### ✅ Issue #7 — Запуск периодического мониторинга по таймеру

**Labels:** `feature`, `cron`, `priority: medium`

**Описание:**

* В `main.cpp` организовать фоновый цикл:

  * периодически (каждые N секунд) выполнять `check_all_services()`,
  * писать логи при изменении статуса или при ошибках.
* Интервал брать из env-переменной:

  * `MONITORING_INTERVAL_SEC` (например, по умолчанию 30 секунд).

**Acceptance criteria:**

* При запуске сервиса он:

  * поднимает HTTP-сервер с `/status`,
  * в фоне периодически проверяет другие сервисы.
* В логах видно регулярные проверки.

---

### ✅ Issue #8 — Логирование ключевых событий

**Labels:** `logging`

**Описание:**

* Дополнительно к логам об ошибках БД:

  * логировать старт monitoring-service,
  * логировать начало и окончание очередного цикла проверки,
  * логировать изменение статуса сервиса (например, OK → FAIL, FAIL → OK).
* Логи отправлять:

  * в `stdout` для просмотра в контейнере,
  * плюс в таблицу `logs` для критичных событий (падение/восстановление сервисов).

**Acceptance criteria:**

* При падении какого-то сервиса в `logs` (в БД) появляется запись об этом.
* При его восстановлении — тоже.

---

### ✅ Issue #9 — Написать базовые тесты для мониторинга

**Labels:** `tests`, `priority: medium`

**Описание:**

* Unit-тесты:

  * `check_health` с mock-HTTP (можно фейковый сервер или заглушки),
  * `check_all_services` с подставными URL.
* Если возможно — интеграционный тест:

  * поднять один тестовый HTTP-сервис (например, локально в тестах) и проверять, что monitoring-service правильно интерпретирует ответы.

**Acceptance criteria:**

* `ctest` запускается и выполняет тесты monitor-модуля.
* Основная логика (OK/FAIL) проверена.

---

### ✅ Issue #10 — Dockerfile для monitoring-service

**Labels:** `docker`, `infrastructure`, `priority: medium`

**Описание:**

* Написать `Dockerfile` для сборки и запуска monitoring-service:

  * установка зависимостей (cmake, компилятор, http-библиотека, libpqxx и т.д.),
  * сборка проекта,
  * запуск бинарника как `CMD`.
* Проверить:

  * `docker build .`,
  * `docker run` с нужными env-переменными.

**Acceptance criteria:**

* Контейнер собирается успешно.
* При запуске контейнера сервис стартует и логирует `Monitoring service started`.

---

### ✅ Issue #11 — Интеграция monitoring-service в общий docker-compose и документация

**Labels:** `docker`, `integration`, `documentation`

**Описание:**

* Добавить `monitoring-service` в общий `docker-compose.yml`:

  * указать зависимости от других сервисов и postgres,
  * прокинуть `*_SERVICE_URL` и параметры БД.
* Обновить `README.md` для monitoring-service:

  * что делает,
  * какие env-переменные нужны,
  * как дернуть `/status`.
* Обновить `TODO.md` (следующие идеи: интеграция с алертингом, метрики самого мониторинга и т.п.).

**Acceptance criteria:**

* При запуске всего стека через `docker-compose up`:

  * monitoring-service успешно поднимается,
  * может проверять другие сервисы по их URL.
* README даёт понятную инструкцию по запуску и использованию.

---

Если хочешь, дальше могу:

* сделать аналогичный набор issues для **api-service**,
* или набросать пример health-эндпоинта (`/health`) для остальных сервисов, чтобы monitoring-service было что пинговать.
