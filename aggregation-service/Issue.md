# ✅ **Issue #1 — Создать каркас aggregation-service**

**Labels:** `setup`, `priority: high`
**Описание:**

* Создать сервис `aggregation-service` в монорепо.
* Создать базовую структуру проекта:

```
aggregation-service/
  src/
    main.cpp
    aggregator.cpp
    database.cpp
  include/
    aggregator.h
    database.h
  tests/
    test_aggregator.cpp
    test_database.cpp
  CMakeLists.txt
  Dockerfile
  docker-compose.yml
  README.md
  TODO.md
```

* Настроить `CMakeLists.txt` для сборки:
  * библиотека `aggregation-core` (aggregator + database),
  * бинарник `aggregation-service`.
* Добавить простой вывод `Aggregation Service started` в `main.cpp` для проверки сборки.

**Acceptance criteria:**

* Проект собирается локально через CMake (бинарь `aggregation-service` есть).
* Есть рабочий каркас файлов и запускаемый `main`.

---

# ✅ **Issue #2 — Подключение aggregation-service к своей БД PostgreSQL**

**Labels:** `backend`, `database`, `priority: high`
**Описание:**

* Реализовать подключение `aggregation-service` к собственной БД Postgres через библиотеку `libpq` (C-API).
* Реализовать `Database` в `database.h` + `database.cpp`:
  * методы:
    * `bool connect(const std::string& connectionString);`
    * `void disconnect();`
    * `bool isConnected() const;`
    * `bool initSchema();` — создаёт таблицы схемы, если их нет.
* Параметры подключения брать из env-переменных:
  * `AGG_DB_HOST`
  * `AGG_DB_PORT`
  * `AGG_DB_NAME`
  * `AGG_DB_USER`
  * `AGG_DB_PASSWORD`
* В `main.cpp`:
  * собрать строку подключения из ENV,
  * создать `Database`,
  * вызвать `connect()` и `initSchema()`,
  * завершать работу при ошибке подключения / инициализации схемы.

**Acceptance criteria:**

* При запуске `aggregation-service`:
  * устанавливается соединение с Postgres,
  * в логах видно успешное/неуспешное подключение.
* При отсутствии БД с именем `AGG_DB_NAME` сервис падает с читаемой ошибкой.
* Ошибки БД логируются в `stderr`.

---

# ✅ **Issue #3 — Схема БД для aggregation-service**

**Labels:** `backend`, `database`, `design`, `priority: high`
**Описание:**

* Внутренняя БД `aggregation-service` должна хранить агрегированные метрики и технический watermark.
* В `Database::initSchema()` (или через миграции) создать таблицы:

  * Таблица `aggregated_events`:

    ```sql
    CREATE TABLE IF NOT EXISTS aggregated_events (
        id              SERIAL PRIMARY KEY,
        time_bucket     TIMESTAMPTZ NOT NULL,
        project_id      TEXT NOT NULL,
        page            TEXT,
        event_type      TEXT NOT NULL,
        events_count    BIGINT NOT NULL DEFAULT 0,
        unique_users    BIGINT,
        unique_sessions BIGINT,
        avg_perf_ms     DOUBLE PRECISION,
        p95_perf_ms     DOUBLE PRECISION,
        errors_count    BIGINT,
        created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW()
    );
    ```

  * Таблица `aggregation_watermark`:

    ```sql
    CREATE TABLE IF NOT EXISTS aggregation_watermark (
        id                 INTEGER PRIMARY KEY,
        last_aggregated_at TIMESTAMPTZ NOT NULL
    );

    INSERT INTO aggregation_watermark (id, last_aggregated_at)
    VALUES (1, '1970-01-01T00:00:00Z')
    ON CONFLICT (id) DO NOTHING;
    ```

**Acceptance criteria:**

* При первом старте сервиса таблицы создаются автоматически, если их нет.
* Повторный запуск не приводит к ошибкам (`CREATE IF NOT EXISTS` / `ON CONFLICT DO NOTHING`).
* Структура таблиц соответствует описанной схеме.

---

# ✅ **Issue #4 — Модель данных внутри aggregation-service**

**Labels:** `backend`, `design`
**Описание:**

* Определить структуры данных, с которыми будет работать `Aggregator`.
* Сырые события (то, что приходит от `metrics-service` по gRPC):

  ```cpp
  struct RawEvent {
      std::string projectId;
      std::string page;
      std::string eventType;
      double performanceMs;
      bool isError;
      std::string userId;
      std::string sessionId;
      std::chrono::system_clock::time_point timestamp;
  };
  ```

* Агрегированный результат для одной группы (один `time_bucket + projectId + page + eventType`):

  ```cpp
  struct AggregatedEvent {
      std::string projectId;
      std::string page;
      std::string eventType;
      std::chrono::system_clock::time_point timeBucket;
      std::int64_t eventsCount;
      std::int64_t uniqueUsers;
      std::int64_t uniqueSessions;
      double avgPerfMs;
      double p95PerfMs;
      std::int64_t errorsCount;
  };
  ```

* Структуры объявить в `aggregator.h` (или отдельном `models.h`).

**Acceptance criteria:**

* Структуры объявлены и компилируются.
* `timestamp` хранится in `std::chrono::system_clock::time_point`.
* Пока можно не реализовывать конвертацию в/из протобафа/SQL — только модель.

---

# ✅ **Issue #5 — gRPC-клиент к metrics-service (fetch сырых событий)**

**Labels:** `backend`, `grpc`, `priority: high`
**Описание:**

* `aggregation-service` должен получать сырые события не напрямую из БД, а от `metrics-service` по gRPC.
* Добавить proto-файл `metrics_service.proto` (или подключить общий), в котором есть метод:

  ```proto
  service MetricsService {
    rpc ListEvents(ListEventsRequest) returns (ListEventsResponse);
  }
  ```

* В `aggregation-service`:
  * настроить генерацию C++ stubs для клиента;
  * реализовать класс `MetricsClient` (например, `metrics_client.h`/`.cpp`) с методом:

    ```cpp
    std::vector<RawEvent> fetchEvents(
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to
    );
    ```

* Параметры подключения к `metrics-service` взять из ENV:
  * `METRICS_GRPC_HOST`
  * `METRICS_GRPC_PORT`

* Важно: никакого прямого `SELECT` к БД метрик из `aggregation-service` — только gRPC.

**Acceptance criteria:**

* Есть класс `MetricsClient` с методом `fetchEvents(from, to)`, который делает gRPC-вызов.
* При недоступности `metrics-service` метод логирует ошибку и возвращает пустой вектор/кидает исключение (стратегию оговорить).
* В тестах можно использовать заглушку вместо реального gRPC (mock/stub).

---

# ✅ **Issue #6 — Логика агрегации в Aggregator**

**Labels:** `backend`, `logic`, `priority: high`
**Описание:**

* Реализовать в `Aggregator`:
  * Базовые функции работы с числовыми значениями:

    ```cpp
    static double calculateAverage(const std::vector<double>& values);
    static double calculateMin(const std::vector<double>& values);
    static double calculateMax(const std::vector<double>& values);
    ```

  * Функцию агрегации сырых событий:

    ```cpp
    std::vector<AggregatedEvent> aggregateEvents(
        const std::vector<RawEvent>& events,
        std::chrono::minutes bucketSize
    );
    ```

* Логика `aggregateEvents`:
  * сгруппировать события по ключу:
    * `time_bucket` (округление `timestamp` до минут/5 минут),
    * `projectId`,
    * `page`,
    * `eventType`;
  * посчитать для каждой группы:
    * `eventsCount`,
    * `uniqueUsers`,
    * `uniqueSessions`,
    * `avgPerfMs`,
    * `p95PerfMs`,
    * `errorsCount`.

**Acceptance criteria:**

* Есть unit-тесты на `calculateAverage`/`Min`/`Max`.
* Есть тест на `aggregateEvents` для простого набора `RawEvent`.
* Логика не трогает БД — только in-memory агрегация.

---

# ✅ **Issue #7 — Запись агрегированных данных и watermark в БД**

**Labels:** `database`, `backend`, `priority: high`
**Описание:**

* В `Database` реализовать:

  ```cpp
  bool writeAggregatedEvents(const std::vector<AggregatedEvent>& rows);
  bool updateWatermark(const std::chrono::system_clock::time_point& ts);
  std::optional<std::chrono::system_clock::time_point> getWatermark();
  ```

* `writeAggregatedEvents`:
  * делает `INSERT` в `aggregated_events` для каждой строки (либо батчом).
* `updateWatermark`:
  * выполняет `UPDATE aggregation_watermark SET last_aggregated_at = ... WHERE id = 1;`
* `getWatermark`:
  * делает `SELECT last_aggregated_at FROM aggregation_watermark WHERE id = 1;`

**Acceptance criteria:**

* После вызова `writeAggregatedEvents` строки реально появляются в `aggregated_events`.
* После `updateWatermark` значение в `aggregation_watermark` меняется.
* Все SQL-ошибки логируются.

---

# ✅ **Issue #8 — Реализовать один шаг агрегации в Aggregator::run()**

**Labels:** `backend`, `logic`, `priority: high`
**Описание:**

* Реализовать в `Aggregator::run()` один шаг агрегации:
  * Прочитать текущий watermark из БД (`getWatermark()`).
  * Определить интервал агрегации:
    * `from = watermark;`
    * `to = now() - safetyLag` (например, 30 секунд).
  * Вызвать `MetricsClient::fetchEvents(from, to)`.
  * Прогнать список событий через `aggregateEvents(...)`.
  * Записать результат в `aggregated_events` (`writeAggregatedEvents`).
  * Обновить watermark на `to` (`updateWatermark(to)`).
* Пока можно запускать `run()` один раз при старте сервиса (без таймера).

**Acceptance criteria:**

* При наличии тестовых событий в `metrics-service` после вызова `run()` появляются строки в `aggregated_events`.
* В логах видно:
  * диапазон агрегации (`from`/`to`),
  * количество полученных сырых событий,
  * количество записанных агрегатов.

---

# ✅ **Issue #9 — Режим запуска по таймеру**

**Labels:** `feature`, `cron`, `priority: medium`
**Описание:**

* В `main.cpp` реализовать цикл, который:
  * раз в N секунд/минут вызывает `aggregator.run()`.
* Использовать `std::this_thread::sleep_for(...)`.
* Интервал задаётся через env-переменную:
  * `AGGREGATION_INTERVAL_SEC` (по умолчанию, например, 60).

**Acceptance criteria:**

* Сервис периодически выполняет шаг агрегации.
* Интервал легко менять через env.
* Ошибки внутри одного шага не «убивают» весь сервис (есть обработка исключений/ошибок).

---

# ✅ **Issue #10 — Логирование**

**Labels:** `logging`
**Описание:**

* Добавить логирование ключевых событий в stdout/stderr:
  * старт сервиса;
  * успешное/неуспешное подключение к БД;
  * успешная инициализация схемы;
  * начало и окончание шага агрегации;
  * количество полученных сырых событий;
  * количество записанных агрегатов;
  * ошибки БД;
  * ошибки gRPC.
* Можно начать с простого `std::cout` / `std::cerr`.

**Acceptance criteria:**

* В логах видно «жизненный цикл» одного шага агрегации.
* Ошибки сопровождаются понятными сообщениями.

---

# ✅ **Issue #11 — Dockerfile + интеграция с docker-compose**

**Labels:** `docker`, `infrastructure`, `priority: medium`
**Описание:**

* Написать `Dockerfile` для `aggregation-service`:
  * собрать проект через CMake,
  * внутри контейнера запускать бинарник `aggregation-service`.
* Подключить сервис к контейнеру `agg-postgres`:
  * через переменные окружения `AGG_DB_HOST=agg-postgres`, `AGG_DB_PORT=5432` и т.п.
* Добавить сервис в общий `docker-compose.yml` проекта (или отдельный compose для локального теста).

**Acceptance criteria:**

* `docker compose up aggregation-service agg-postgres` поднимает оба контейнера.
* `aggregation-service` в контейнере успешно подключается к `agg-postgres` и инициализирует схему.
* Логи видны через `docker compose logs`.

---

# ✅ **Issue #12 — Обновить README.md и TODO.md**

**Labels:** `documentation`, `priority: low`
**Описание:**

* Обновить `README.md` сервиса:
  * назначение сервиса;
  * схема взаимодействия (`metrics-service → aggregation-service → monitoring-service`);
  * сборка (локально и в Docker);
  * запуск;
  * переменные окружения (`AGG_DB_*`, `METRICS_GRPC_*`, `AGGREGATION_INTERVAL_SEC`).
* Обновить `TODO.md`:
  * перечислить оставшиеся улучшения/оптимизации.

**Acceptance criteria:**

* `README` даёт понятное представление о сервисе и позволяет новому человеку его собрать и запустить.
* `TODO` содержит актуальные задачи по развитию `aggregation-service`.
