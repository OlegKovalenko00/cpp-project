### ✅ Issue #1 — Инициализировать репозиторий и структуру проекта

**Labels:** `setup`, `priority: high`

**Описание:**

* Создать репозиторий `metrics-service`.
* Добавить базовую структуру:

```text
metrics-service/
  src/
    main.cpp
    metrics.cpp
  include/
    metrics.h
    database.h
  tests/
    test_metrics.cpp
    test_database.cpp
  CMakeLists.txt
  Dockerfile
  README.md
  TODO.md
```

* В `main.cpp` сделать простой запуск сервиса: вывод `"Metrics service started"`.

**Acceptance criteria:**

* Проект собирается локально через CMake.
* Есть минимальный исполняемый файл, который запускается без ошибок.

---

### ✅ Issue #2 — Настроить подключение к PostgreSQL

**Labels:** `backend`, `database`, `priority: high`

**Описание:**

* Реализовать модуль для работы с БД (файлы `database.h` / `database.cpp` или внутри `metrics.cpp` на первом этапе).
* Подключение к PostgreSQL через `libpqxx`.
* Конфигурация подключения берётся из переменных окружения:

  * `POSTGRES_HOST`
  * `POSTGRES_DB`
  * `POSTGRES_USER`
  * `POSTGRES_PASSWORD`
* Написать простую проверку: выполнить `SELECT 1`.

**Acceptance criteria:**

* Сервис при старте пробует подключиться к БД.
* В случае успеха — логирует, что подключение успешно.
* В случае ошибки — выводит в лог адекватное сообщение.

---

### ✅ Issue #3 — Определить модель данных для метрик

**Labels:** `backend`, `design`

**Описание:**

* Описать структуру метрики в `metrics.h`, например:

```cpp
struct Metric {
    std::string name;
    double value;
    std::chrono::system_clock::time_point timestamp;
};
```

* Определиться с тем, какие типы метрик будут на первом этапе:

  * например: `request_count`, `response_time_ms`, `error_count`.
* Продумать формат `timestamp` (ISO-строка или Unix time) для записи в БД.

**Acceptance criteria:**

* Структура `Metric` объявлена и используется в коде.
* Проект компилируется.

---

### ✅ Issue #4 — Создать таблицу `metrics` и описать SQL-слой

**Labels:** `database`, `backend`, `priority: high`

**Описание:**

* Определить SQL-схему таблицы `metrics`:

```sql
CREATE TABLE IF NOT EXISTS metrics (
    id SERIAL PRIMARY KEY,
    metric_name VARCHAR(255) NOT NULL,
    metric_value DOUBLE PRECISION NOT NULL,
    timestamp TIMESTAMP NOT NULL
);
```

* Добавить функцию инициализации БД (создание таблицы, если её нет).
* Вынести SQL в отдельный модуль/функции при необходимости.

**Acceptance criteria:**

* При старте сервиса выполняется создание таблицы (если отсутствует).
* Таблица `metrics` корректно создаётся в Postgres.

---

### ✅ Issue #5 — Реализовать функцию записи метрики в БД

**Labels:** `backend`, `database`, `priority: high`

**Описание:**

* Реализовать функцию:

```cpp
void save_metric(const Metric& metric);
```

* SQL примерно такой:

```sql
INSERT INTO metrics (metric_name, metric_value, timestamp)
VALUES ($1, $2, $3);
```

* Обработать ошибки вставки (логировать).

**Acceptance criteria:**

* Вызов `save_metric` приводит к появлению записи в таблице `metrics`.
* Ошибки подключения/SQL не крашат сервис «тихо», а логируются.

---

### ✅ Issue #6 — Сделать минимальный публичный интерфейс для записи метрик

**Labels:** `feature`, `backend`, `priority: medium`

**Описание:**

На этом этапе нужна **минимальная точка входа**, чтобы можно было «снаружи» кинуть метрику:

Варианты (выберите один и реализуйте):

1. **Простой HTTP-эндпоинт** (если несложно поднять HTTP в C++):

   * `POST /metrics`
   * Тело запроса: `{ "name": "...", "value": ... }`.

2. **CLI-режим**:

   * Запуск сервиса как утилиты:

     ```bash
     ./metrics-service add-metric request_count 1
     ```
   * В этом случае можно оставить постоянный сервис на потом, а сейчас сделать удобную точку записи.

Главное — иметь способ проверить запись **без правки кода**.

**Acceptance criteria:**

* Существует удобный способ вручную добавить метрику (через curl или CLI).
* После вызова этого интерфейса в таблице `metrics` появляется новая запись.

---

### ✅ Issue #7 — Написать базовые тесты для логики метрик

**Labels:** `tests`, `priority: medium`

**Описание:**

* Добавить unit-тесты для:

  * корректности создания `Metric`,
  * проверки, что функция `save_metric` вызывается и не падает при корректных данных (можно с тестовой БД / in-memory / mock).
* Настроить запуск тестов через CTest.

**Acceptance criteria:**

* `ctest` запускается и выполняется минимум 1–2 теста.
* Тесты зелёные в локальной среде.

---

### ✅ Issue #8 — Логирование работы сервиса

**Labels:** `logging`

**Описание:**

* Добавить простое логирование (stdout или отдельный лог-файл):

  * старт сервиса,
  * успешное подключение к БД,
  * запись метрики (с именем и значением),
  * ошибки БД.

**Acceptance criteria:**

* Логи читаемые и помогают понять, что делает сервис.
* При записи метрики в логах видно её имя и значение.

---

### ✅ Issue #9 — Dockerfile для metrics-service

**Labels:** `docker`, `infrastructure`, `priority: medium`

**Описание:**

* Написать `Dockerfile` для сборки и запуска сервиса:

  * Базовый образ: `ubuntu`/`debian` или `gcc`/`llvm`,
  * установка `cmake`, `g++`, `libpq-dev`, `libpqxx-dev`,
  * сборка проекта,
  * `CMD` для запуска сервиса.
* Проверить локально `docker build` + `docker run`.

**Acceptance criteria:**

* Контейнер собирается без ошибок.
* В контейнере сервис запускается и пытается подключиться к БД (ошибки подключения — ок, если БД нет).

---

### ✅ Issue #10 — Интеграция в общий docker-compose

**Labels:** `docker`, `integration`

**Описание:**

* Добавить `metrics-service` в общий `docker-compose.yml` проекта:

  * указать зависимость от `postgres`,
  * прокинуть нужные env-переменные,
  * настроить сеть.
* Протестировать запуск: `docker-compose up metrics-service postgres`.

**Acceptance criteria:**

* При запуске через `docker-compose up` сервис поднимается.
* Сервис может подключиться к `postgres` внутри docker-compose-сети.

---

### ✅ Issue #11 — Обновить README.md и TODO.md для metrics-service

**Labels:** `documentation`, `priority: low`

**Описание:**

* В `README.md` для `metrics-service` описать:

  * назначение сервиса,
  * как собрать,
  * как запустить локально,
  * как запустить через Docker,
  * пример добавления метрики (curl/CLI).
* В `TODO.md` зафиксировать оставшиеся улучшения:

  * новые типы метрик,
  * метрики по нескольким сервисам,
  * возможный HTTP-интерфейс, если пока нет.

**Acceptance criteria:**

* README понятен человеку, который впервые видит сервис.
* TODO отражает реальные следующие шаги.

---

Если хочешь, дальше можем:

* сделать такой же набор issue для **api-service** и **monitoring-service**,
* или на основе этих issues набросать Kanban-колонки под GitHub Projects (Backlog / In progress / Done).
