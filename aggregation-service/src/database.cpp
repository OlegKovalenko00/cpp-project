#include "database.h"
#include "aggregator.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace aggregation {

Database::Database() : dbConnection_(nullptr) {}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& connectionString) {
    disconnect();

    dbConnection_ = PQconnectdb(connectionString.c_str());

    if (dbConnection_ == nullptr) {
        std::cerr << "Failed to allocate PGconn" << std::endl;
        return false;
    }

    if (PQstatus(dbConnection_) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: "
                  << PQerrorMessage(dbConnection_) << std::endl;
        disconnect();
        return false;
    }

    std::cout << "Connected to database successfully" << std::endl;
    return true;
}

void Database::disconnect() {
    if (dbConnection_ != nullptr) {
        PQfinish(dbConnection_);
        dbConnection_ = nullptr;
    }
}

bool Database::isConnected() const {
    return dbConnection_ != nullptr && PQstatus(dbConnection_) == CONNECTION_OK;
}

bool Database::initializeSchema() {
    if (!isConnected()) {
        std::cerr << "Cannot initialize schema: no active database connection" << std::endl;
        return false;
    }

    // Таблица для агрегации page_view событий
    const char* pageViewsTable =
        "CREATE TABLE IF NOT EXISTS agg_page_views ("
        "    id SERIAL PRIMARY KEY,"
        "    time_bucket TIMESTAMPTZ NOT NULL,"
        "    project_id TEXT NOT NULL,"
        "    page TEXT NOT NULL,"
        "    views_count BIGINT NOT NULL DEFAULT 0,"
        "    unique_users BIGINT DEFAULT 0,"
        "    unique_sessions BIGINT DEFAULT 0,"
        "    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),"
        "    UNIQUE(time_bucket, project_id, page)"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_agg_page_views_time "
        "    ON agg_page_views(time_bucket);"
        "CREATE INDEX IF NOT EXISTS idx_agg_page_views_project "
        "    ON agg_page_views(project_id, page);";

    // Таблица для агрегации click событий
    const char* clicksTable =
        "CREATE TABLE IF NOT EXISTS agg_clicks ("
        "    id SERIAL PRIMARY KEY,"
        "    time_bucket TIMESTAMPTZ NOT NULL,"
        "    project_id TEXT NOT NULL,"
        "    page TEXT NOT NULL,"
        "    element_id TEXT,"
        "    clicks_count BIGINT NOT NULL DEFAULT 0,"
        "    unique_users BIGINT DEFAULT 0,"
        "    unique_sessions BIGINT DEFAULT 0,"
        "    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),"
        "    UNIQUE(time_bucket, project_id, page, element_id)"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_agg_clicks_time "
        "    ON agg_clicks(time_bucket);"
        "CREATE INDEX IF NOT EXISTS idx_agg_clicks_project "
        "    ON agg_clicks(project_id, page);";

    // Таблица для агрегации performance метрик
    const char* performanceTable =
        "CREATE TABLE IF NOT EXISTS agg_performance ("
        "    id SERIAL PRIMARY KEY,"
        "    time_bucket TIMESTAMPTZ NOT NULL,"
        "    project_id TEXT NOT NULL,"
        "    page TEXT NOT NULL,"
        "    samples_count BIGINT NOT NULL DEFAULT 0,"
        "    avg_total_load_ms DOUBLE PRECISION,"
        "    p95_total_load_ms DOUBLE PRECISION,"
        "    avg_ttfb_ms DOUBLE PRECISION,"
        "    p95_ttfb_ms DOUBLE PRECISION,"
        "    avg_fcp_ms DOUBLE PRECISION,"
        "    p95_fcp_ms DOUBLE PRECISION,"
        "    avg_lcp_ms DOUBLE PRECISION,"
        "    p95_lcp_ms DOUBLE PRECISION,"
        "    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),"
        "    UNIQUE(time_bucket, project_id, page)"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_agg_performance_time "
        "    ON agg_performance(time_bucket);"
        "CREATE INDEX IF NOT EXISTS idx_agg_performance_project "
        "    ON agg_performance(project_id, page);";

    // Таблица для агрегации error событий
    const char* errorsTable =
        "CREATE TABLE IF NOT EXISTS agg_errors ("
        "    id SERIAL PRIMARY KEY,"
        "    time_bucket TIMESTAMPTZ NOT NULL,"
        "    project_id TEXT NOT NULL,"
        "    page TEXT NOT NULL,"
        "    error_type TEXT,"
        "    errors_count BIGINT NOT NULL DEFAULT 0,"
        "    warning_count BIGINT DEFAULT 0,"
        "    critical_count BIGINT DEFAULT 0,"
        "    unique_users BIGINT DEFAULT 0,"
        "    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),"
        "    UNIQUE(time_bucket, project_id, page, error_type)"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_agg_errors_time "
        "    ON agg_errors(time_bucket);"
        "CREATE INDEX IF NOT EXISTS idx_agg_errors_project "
        "    ON agg_errors(project_id, page);";

    // Таблица для агрегации custom событий
    const char* customEventsTable =
        "CREATE TABLE IF NOT EXISTS agg_custom_events ("
        "    id SERIAL PRIMARY KEY,"
        "    time_bucket TIMESTAMPTZ NOT NULL,"
        "    project_id TEXT NOT NULL,"
        "    event_name TEXT NOT NULL,"
        "    page TEXT,"
        "    events_count BIGINT NOT NULL DEFAULT 0,"
        "    unique_users BIGINT DEFAULT 0,"
        "    unique_sessions BIGINT DEFAULT 0,"
        "    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),"
        "    UNIQUE(time_bucket, project_id, event_name, page)"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_agg_custom_events_time "
        "    ON agg_custom_events(time_bucket);"
        "CREATE INDEX IF NOT EXISTS idx_agg_custom_events_project "
        "    ON agg_custom_events(project_id, event_name);";

    // Таблица watermark для отслеживания прогресса агрегации
    const char* watermarkTable =
        "CREATE TABLE IF NOT EXISTS aggregation_watermark ("
        "    id INTEGER PRIMARY KEY,"
        "    last_aggregated_at TIMESTAMPTZ NOT NULL"
        ");"
        "INSERT INTO aggregation_watermark (id, last_aggregated_at) "
        "VALUES (1, '1970-01-01T00:00:00Z') "
        "ON CONFLICT (id) DO NOTHING;";

    // Выполняем все запросы
    const char* queries[] = {
        pageViewsTable,
        clicksTable,
        performanceTable,
        errorsTable,
        customEventsTable,
        watermarkTable
    };

    for (const char* sql : queries) {
        PGresult* res = PQexec(dbConnection_, sql);

        if (res == nullptr) {
            std::cerr << "Failed to execute schema query: null result" << std::endl;
            return false;
        }

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Failed to initialize schema: " << PQerrorMessage(dbConnection_) << std::endl;
            PQclear(res);
            return false;
        }

        PQclear(res);
    }

    std::cout << "Database schema initialized successfully (5 tables + watermark)" << std::endl;
    return true;
}

bool Database::executeQuery(const std::string& query) {
    if (!isConnected()) {
        std::cerr << "Cannot execute query: no active database connection" << std::endl;
        return false;
    }

    PGresult* res = PQexec(dbConnection_, query.c_str());

    if (res == nullptr) {
        std::cerr << "Failed to execute query: null result" << std::endl;
        return false;
    }

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::cerr << "Query execution failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

std::string Database::formatTimestamp(std::chrono::system_clock::time_point tp) const {
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_val{};
    gmtime_r(&time_t_val, &tm_val);

    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::chrono::system_clock::time_point Database::parseTimestamp(const std::string& ts) const {
    std::tm tm_val{};
    std::istringstream iss(ts);
    iss >> std::get_time(&tm_val, "%Y-%m-%d %H:%M:%S");

    if (iss.fail()) {
        return std::chrono::system_clock::time_point{};
    }

    std::time_t time_t_val = timegm(&tm_val);
    return std::chrono::system_clock::from_time_t(time_t_val);
}

std::string Database::escapeString(const std::string& str) const {
    if (!dbConnection_) return str;

    char* escaped = PQescapeLiteral(dbConnection_, str.c_str(), str.length());
    if (!escaped) return "''";

    std::string result(escaped);
    PQfreemem(escaped);
    return result;
}

std::chrono::system_clock::time_point Database::getWatermark() {
    if (!isConnected()) {
        std::cerr << "Cannot get watermark: no active database connection" << std::endl;
        return std::chrono::system_clock::time_point{};
    }

    const char* sql = "SELECT last_aggregated_at FROM aggregation_watermark WHERE id = 1";
    PGresult* res = PQexec(dbConnection_, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        std::cerr << "Failed to get watermark: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return std::chrono::system_clock::time_point{};
    }

    std::string timestamp = PQgetvalue(res, 0, 0);
    PQclear(res);

    return parseTimestamp(timestamp);
}

bool Database::updateWatermark(std::chrono::system_clock::time_point timestamp) {
    if (!isConnected()) {
        std::cerr << "Cannot update watermark: no active database connection" << std::endl;
        return false;
    }

    std::ostringstream sql;
    sql << "UPDATE aggregation_watermark SET last_aggregated_at = '"
        << formatTimestamp(timestamp) << "' WHERE id = 1";

    return executeQuery(sql.str());
}

bool Database::writePageViews(const std::vector<AggregatedPageViews>& data) {
    if (data.empty()) return true;
    if (!isConnected()) return false;

    std::ostringstream sql;
    sql << "INSERT INTO agg_page_views (time_bucket, project_id, page, views_count, unique_users, unique_sessions) VALUES ";

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& d = data[i];
        if (i > 0) sql << ", ";
        sql << "('" << formatTimestamp(d.timeBucket) << "', "
            << escapeString(d.projectId) << ", "
            << escapeString(d.page) << ", "
            << d.viewsCount << ", "
            << d.uniqueUsers << ", "
            << d.uniqueSessions << ")";
    }
    sql << " ON CONFLICT (time_bucket, project_id, page) DO UPDATE SET "
        << "views_count = agg_page_views.views_count + EXCLUDED.views_count, "
        << "unique_users = EXCLUDED.unique_users, "
        << "unique_sessions = EXCLUDED.unique_sessions";

    return executeQuery(sql.str());
}

bool Database::writeClicks(const std::vector<AggregatedClicks>& data) {
    if (data.empty()) return true;
    if (!isConnected()) return false;

    std::ostringstream sql;
    sql << "INSERT INTO agg_clicks (time_bucket, project_id, page, element_id, clicks_count, unique_users, unique_sessions) VALUES ";

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& d = data[i];
        if (i > 0) sql << ", ";
        sql << "('" << formatTimestamp(d.timeBucket) << "', "
            << escapeString(d.projectId) << ", "
            << escapeString(d.page) << ", "
            << escapeString(d.elementId) << ", "
            << d.clicksCount << ", "
            << d.uniqueUsers << ", "
            << d.uniqueSessions << ")";
    }
    sql << " ON CONFLICT (time_bucket, project_id, page, element_id) DO UPDATE SET "
        << "clicks_count = agg_clicks.clicks_count + EXCLUDED.clicks_count, "
        << "unique_users = EXCLUDED.unique_users, "
        << "unique_sessions = EXCLUDED.unique_sessions";

    return executeQuery(sql.str());
}

bool Database::writePerformance(const std::vector<AggregatedPerformance>& data) {
    if (data.empty()) return true;
    if (!isConnected()) return false;

    std::ostringstream sql;
    sql << "INSERT INTO agg_performance (time_bucket, project_id, page, samples_count, "
        << "avg_total_load_ms, p95_total_load_ms, avg_ttfb_ms, p95_ttfb_ms, "
        << "avg_fcp_ms, p95_fcp_ms, avg_lcp_ms, p95_lcp_ms) VALUES ";

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& d = data[i];
        if (i > 0) sql << ", ";
        sql << "('" << formatTimestamp(d.timeBucket) << "', "
            << escapeString(d.projectId) << ", "
            << escapeString(d.page) << ", "
            << d.samplesCount << ", "
            << d.avgTotalLoadMs << ", "
            << d.p95TotalLoadMs << ", "
            << d.avgTtfbMs << ", "
            << d.p95TtfbMs << ", "
            << d.avgFcpMs << ", "
            << d.p95FcpMs << ", "
            << d.avgLcpMs << ", "
            << d.p95LcpMs << ")";
    }
    sql << " ON CONFLICT (time_bucket, project_id, page) DO UPDATE SET "
        << "samples_count = agg_performance.samples_count + EXCLUDED.samples_count, "
        << "avg_total_load_ms = EXCLUDED.avg_total_load_ms, "
        << "p95_total_load_ms = EXCLUDED.p95_total_load_ms, "
        << "avg_ttfb_ms = EXCLUDED.avg_ttfb_ms, "
        << "p95_ttfb_ms = EXCLUDED.p95_ttfb_ms, "
        << "avg_fcp_ms = EXCLUDED.avg_fcp_ms, "
        << "p95_fcp_ms = EXCLUDED.p95_fcp_ms, "
        << "avg_lcp_ms = EXCLUDED.avg_lcp_ms, "
        << "p95_lcp_ms = EXCLUDED.p95_lcp_ms";

    return executeQuery(sql.str());
}

bool Database::writeErrors(const std::vector<AggregatedErrors>& data) {
    if (data.empty()) return true;
    if (!isConnected()) return false;

    std::ostringstream sql;
    sql << "INSERT INTO agg_errors (time_bucket, project_id, page, error_type, "
        << "errors_count, warning_count, critical_count, unique_users) VALUES ";

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& d = data[i];
        if (i > 0) sql << ", ";
        sql << "('" << formatTimestamp(d.timeBucket) << "', "
            << escapeString(d.projectId) << ", "
            << escapeString(d.page) << ", "
            << escapeString(d.errorType) << ", "
            << d.errorsCount << ", "
            << d.warningCount << ", "
            << d.criticalCount << ", "
            << d.uniqueUsers << ")";
    }
    sql << " ON CONFLICT (time_bucket, project_id, page, error_type) DO UPDATE SET "
        << "errors_count = agg_errors.errors_count + EXCLUDED.errors_count, "
        << "warning_count = agg_errors.warning_count + EXCLUDED.warning_count, "
        << "critical_count = agg_errors.critical_count + EXCLUDED.critical_count, "
        << "unique_users = EXCLUDED.unique_users";

    return executeQuery(sql.str());
}

bool Database::writeCustomEvents(const std::vector<AggregatedCustomEvents>& data) {
    if (data.empty()) return true;
    if (!isConnected()) return false;

    std::ostringstream sql;
    sql << "INSERT INTO agg_custom_events (time_bucket, project_id, event_name, page, "
        << "events_count, unique_users, unique_sessions) VALUES ";

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& d = data[i];
        if (i > 0) sql << ", ";
        sql << "('" << formatTimestamp(d.timeBucket) << "', "
            << escapeString(d.projectId) << ", "
            << escapeString(d.eventName) << ", "
            << escapeString(d.page) << ", "
            << d.eventsCount << ", "
            << d.uniqueUsers << ", "
            << d.uniqueSessions << ")";
    }
    sql << " ON CONFLICT (time_bucket, project_id, event_name, page) DO UPDATE SET "
        << "events_count = agg_custom_events.events_count + EXCLUDED.events_count, "
        << "unique_users = EXCLUDED.unique_users, "
        << "unique_sessions = EXCLUDED.unique_sessions";

    return executeQuery(sql.str());
}

bool Database::writeAggregationResult(const AggregationResult& result) {
    bool success = true;

    success &= writePageViews(result.pageViews);
    success &= writeClicks(result.clicks);
    success &= writePerformance(result.performance);
    success &= writeErrors(result.errors);
    success &= writeCustomEvents(result.customEvents);

    return success;
}

} // namespace aggregation
