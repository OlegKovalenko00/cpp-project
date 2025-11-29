# ✅ **Issue #1 — Создать каркас репозитория и структуру проекта**

**Labels:** `setup`, `priority: high`
**Описание:**

* Инициализировать репозиторий `aggregation-service`.
* Создать базовую структуру проекта:

```
aggregation-service/
  src/
    main.cpp
    aggregator.cpp
  include/
    aggregator.h
    database.h
  tests/
    test_aggregator.cpp
    test_database.cpp
  CMakeLists.txt
  Dockerfile
  README.md
  TODO.md
```

* Настроить `CMakeLists.txt` для сборки сервиса.
* Добавить простой код “Hello, Aggregation Service” в `main.cpp` для проверки сборки.

**Acceptance criteria:**

* Репозиторий собирается локально через CMake.
* Есть рабочий каркас файлов.

---

# ✅ **Issue #2 — Настроить подключение к PostgreSQL**

**Labels:** `backend`, `database`, `priority: high`
**Описание:**

* Реализовать подключение к PostgreSQL через библиотеку `libpqxx`.
* Создать `database.h` + `database.cpp` или реализовать в `aggregator.cpp`.
* Функции, которые надо реализовать:

  * `fetchMetrics()`
  * `writeAggregatedResult()`
* Подключение должно брать параметры через env-переменные (`POSTGRES_HOST`, `POSTGRES_DB`, `POSTGRES_USER`, `POSTGRES_PASSWORD`).

**Acceptance criteria:**

* Есть функция, которая делает SELECT к таблице `metrics`.
* Есть функция, которая делает INSERT в `aggregated_metrics`.
* Ошибки БД логируются.

---

# ✅ **Issue #3 — Реализовать модель данных для метрик**

**Labels:** `backend`, `design`
**Описание:**

* Создать структуру для хранения метрик:

```cpp
struct Metric {
    std::string name;
    double value;
    std::chrono::system_clock::time_point timestamp;
};
```

* Создать структуру для агрегированных метрик:

```cpp
struct AggregatedMetric {
    std::string name;
    double avg;
    double min;
    double max;
    std::chrono::system_clock::time_point timestamp;
};
```

* Привести формат timestamp к ISO или Unix time.

**Acceptance criteria:**

* Структуры объявлены в `aggregator.h`.
* Код компилируется.

---

# ✅ **Issue #4 — Реализовать выборку метрик из таблицы metrics**

**Labels:** `backend`, `database`, `priority: high`
**Описание:**

* Реализовать функцию `std::vector<Metric> fetchMetrics(Duration window)`.
* Должна выбирать данные за указанный период (например, за последние N минут/часов).
* Реализовать SQL:

```sql
SELECT metric_name, metric_value, timestamp
FROM metrics
WHERE timestamp > NOW() - INTERVAL 'N minutes';
```

**Acceptance criteria:**

* Функция возвращает корректный список метрик.
* Ошибки SQL обрабатываются.
* Покрыто хотя бы 1 простым тестом.

---

# ✅ **Issue #5 — Реализовать саму агрегацию данных**

**Labels:** `backend`, `logic`, `priority: high`
**Описание:**
Реализовать функции:

* `calculateAverage(vector<Metric>)`
* `calculateMin(vector<Metric>)`
* `calculateMax(vector<Metric>)`
* `aggregate(vector<Metric>) → AggregatedMetric`

Агрегированный результат должен включать:

* среднее
* минимум
* максимум
* timestamp агрегации

**Acceptance criteria:**

* Функция `aggregate()` возвращает корректный результат.
* Покрыто тестами.

---

# ✅ **Issue #6 — Запись агрегированных данных в aggregated_metrics**

**Labels:** `database`, `backend`
**Описание:**

* Реализовать SQL:

```sql
INSERT INTO aggregated_metrics (metric_name, average_value, max_value, min_value, timestamp)
VALUES ($1, $2, $3, $4, $5)
```

* Связать с результатом `aggregate()`.

**Acceptance criteria:**

* Данные появляются в таблице `aggregated_metrics`.
* Ошибки БД корректно обрабатываются.

---

# ✅ **Issue #7 — Реализовать режим запуска по таймеру**

**Labels:** `feature`, `cron`, `priority: medium`
**Описание:**

* В `main.cpp` реализовать цикл: каждые N секунд запускать агрегацию.
* Использовать `std::this_thread::sleep_for(...)`.
* Интервал задаётся через env-переменную: `AGGREGATION_INTERVAL_SEC`.

**Acceptance criteria:**

* Сервис периодически делает агрегацию.
* Интервал легко менять через env.

---

# ✅ **Issue #8 — Добавить логирование**

**Labels:** `logging`
**Описание:**

* Добавить логирование событий:

  * старт агрегации,
  * количество полученных метрик,
  * результат агрегации,
  * ошибки БД.
* Можно начать с простого логирования в stdout.

**Acceptance criteria:**

* В логах видно работу сервиса.
* Логи читаемы.

---

# ✅ **Issue #9 — Dockerfile + интеграция с docker-compose**

**Labels:** `docker`, `infrastructure`, `priority: medium`
**Описание:**

* Написать Dockerfile:

  * собирать проект через CMake,
  * запускать исполняемый файл.
* Проверить локальный запуск.
* Добавить сервис в общий `docker-compose.yml`.

**Acceptance criteria:**

* Команда `docker-compose up aggregation-service` запускает сервис.
* Сервис может подключиться к контейнеру postgres.

---

# ✅ **Issue #10 — Написать тесты (unit + integration)**

**Labels:** `tests`, `priority: high`
**Описание:**
Добавить тесты:

* unit-тесты для:

  * `aggregate()`
  * min/avg/max функций
* интеграционный тест (по возможности) для работы с тестовой БД.

**Acceptance criteria:**

* Тесты запускаются через `ctest`.
* Покрытие агрегационной логики ≥ простого уровня.

---

# ✅ **Issue #11 — Обновить README.md и TODO.md**

**Labels:** `documentation`, `priority: low`
**Описание:**

* Обновить README сервиса:

  * назначение,
  * сборка,
  * запуск,
  * переменные окружения.
* Обновить TODO.md.

---

Если хочешь — могу сделать такую же сетку **для всех сервисов** или сгенерировать **GitHub Project Kanban**, куда это всё сразу раскладывается.
