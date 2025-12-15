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

    // Проверяем что схема уже создана PostgreSQL через init.sql
    // Просто проверим наличие основных таблиц
    const char* checkQuery =
        "SELECT COUNT(*) FROM information_schema.tables "
        "WHERE table_schema = 'public' AND table_name IN ("
        "'agg_page_views', 'agg_clicks', 'agg_performance', "
        "'agg_errors', 'agg_custom_events', 'aggregation_watermark')";

    PGresult* res = PQexec(dbConnection_, checkQuery);

    if (res == nullptr || PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to check database schema: " << PQerrorMessage(dbConnection_) << std::endl;
        if (res) PQclear(res);
        return false;
    }

    int tableCount = 0;
    if (PQntuples(res) > 0) {
        tableCount = std::atoi(PQgetvalue(res, 0, 0));
    }
    PQclear(res);

    if (tableCount != 6) {
        std::cerr << "Database schema incomplete: found " << tableCount << " tables, expected 6" << std::endl;
        std::cerr << "Make sure PostgreSQL initialized with init.sql (via docker-entrypoint-initdb.d)" << std::endl;
        return false;
    }

    std::cout << "Database schema verified successfully (6 tables found)" << std::endl;
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

// ===== Методы чтения для gRPC сервера =====

std::vector<AggregatedPageViews> Database::readPageViews(
    const std::string& projectId,
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to,
    const std::string& pageFilter,
    int limit,
    int offset
) {
    std::vector<AggregatedPageViews> result;

    std::ostringstream sql;
    sql << "SELECT time_bucket, project_id, page, views_count, unique_users, unique_sessions "
        << "FROM agg_page_views WHERE project_id = '" << escapeString(projectId) << "' "
        << "AND time_bucket >= '" << formatTimestamp(from) << "' "
        << "AND time_bucket < '" << formatTimestamp(to) << "' ";

    if (!pageFilter.empty()) {
        sql << "AND page = '" << escapeString(pageFilter) << "' ";
    }

    sql << "ORDER BY time_bucket DESC LIMIT " << limit << " OFFSET " << offset;

    PGresult* res = PQexec(dbConnection_, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Read page views failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return result;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        AggregatedPageViews agg;
        agg.timeBucket = parseTimestamp(PQgetvalue(res, i, 0));
        agg.projectId = PQgetvalue(res, i, 1);
        agg.page = PQgetvalue(res, i, 2);
        agg.viewsCount = std::stoll(PQgetvalue(res, i, 3));
        agg.uniqueUsers = std::stoll(PQgetvalue(res, i, 4));
        agg.uniqueSessions = std::stoll(PQgetvalue(res, i, 5));
        result.push_back(agg);
    }

    PQclear(res);
    return result;
}

std::vector<AggregatedClicks> Database::readClicks(
    const std::string& projectId,
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to,
    const std::string& pageFilter,
    const std::string& elementIdFilter,
    int limit,
    int offset
) {
    std::vector<AggregatedClicks> result;

    std::ostringstream sql;
    sql << "SELECT time_bucket, project_id, page, element_id, clicks_count, unique_users, unique_sessions "
        << "FROM agg_clicks WHERE project_id = '" << escapeString(projectId) << "' "
        << "AND time_bucket >= '" << formatTimestamp(from) << "' "
        << "AND time_bucket < '" << formatTimestamp(to) << "' ";

    if (!pageFilter.empty()) {
        sql << "AND page = '" << escapeString(pageFilter) << "' ";
    }
    if (!elementIdFilter.empty()) {
        sql << "AND element_id = '" << escapeString(elementIdFilter) << "' ";
    }

    sql << "ORDER BY time_bucket DESC LIMIT " << limit << " OFFSET " << offset;

    PGresult* res = PQexec(dbConnection_, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Read clicks failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return result;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        AggregatedClicks agg;
        agg.timeBucket = parseTimestamp(PQgetvalue(res, i, 0));
        agg.projectId = PQgetvalue(res, i, 1);
        agg.page = PQgetvalue(res, i, 2);
        agg.elementId = PQgetvalue(res, i, 3);
        agg.clicksCount = std::stoll(PQgetvalue(res, i, 4));
        agg.uniqueUsers = std::stoll(PQgetvalue(res, i, 5));
        agg.uniqueSessions = std::stoll(PQgetvalue(res, i, 6));
        result.push_back(agg);
    }

    PQclear(res);
    return result;
}

std::vector<AggregatedPerformance> Database::readPerformance(
    const std::string& projectId,
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to,
    const std::string& pageFilter,
    int limit,
    int offset
) {
    std::vector<AggregatedPerformance> result;

    std::ostringstream sql;
    sql << "SELECT time_bucket, project_id, page, samples_count, "
        << "avg_total_load_ms, p95_total_load_ms, avg_ttfb_ms, p95_ttfb_ms, "
        << "avg_fcp_ms, p95_fcp_ms, avg_lcp_ms, p95_lcp_ms "
        << "FROM agg_performance WHERE project_id = '" << escapeString(projectId) << "' "
        << "AND time_bucket >= '" << formatTimestamp(from) << "' "
        << "AND time_bucket < '" << formatTimestamp(to) << "' ";

    if (!pageFilter.empty()) {
        sql << "AND page = '" << escapeString(pageFilter) << "' ";
    }

    sql << "ORDER BY time_bucket DESC LIMIT " << limit << " OFFSET " << offset;

    PGresult* res = PQexec(dbConnection_, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Read performance failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return result;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        AggregatedPerformance agg;
        agg.timeBucket = parseTimestamp(PQgetvalue(res, i, 0));
        agg.projectId = PQgetvalue(res, i, 1);
        agg.page = PQgetvalue(res, i, 2);
        agg.samplesCount = std::stoll(PQgetvalue(res, i, 3));
        agg.avgTotalLoadMs = std::stod(PQgetvalue(res, i, 4));
        agg.p95TotalLoadMs = std::stod(PQgetvalue(res, i, 5));
        agg.avgTtfbMs = std::stod(PQgetvalue(res, i, 6));
        agg.p95TtfbMs = std::stod(PQgetvalue(res, i, 7));
        agg.avgFcpMs = std::stod(PQgetvalue(res, i, 8));
        agg.p95FcpMs = std::stod(PQgetvalue(res, i, 9));
        agg.avgLcpMs = std::stod(PQgetvalue(res, i, 10));
        agg.p95LcpMs = std::stod(PQgetvalue(res, i, 11));
        result.push_back(agg);
    }

    PQclear(res);
    return result;
}

std::vector<AggregatedErrors> Database::readErrors(
    const std::string& projectId,
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to,
    const std::string& pageFilter,
    const std::string& errorTypeFilter,
    int limit,
    int offset
) {
    std::vector<AggregatedErrors> result;

    std::ostringstream sql;
    sql << "SELECT time_bucket, project_id, page, error_type, errors_count, "
        << "warning_count, critical_count, unique_users "
        << "FROM agg_errors WHERE project_id = '" << escapeString(projectId) << "' "
        << "AND time_bucket >= '" << formatTimestamp(from) << "' "
        << "AND time_bucket < '" << formatTimestamp(to) << "' ";

    if (!pageFilter.empty()) {
        sql << "AND page = '" << escapeString(pageFilter) << "' ";
    }
    if (!errorTypeFilter.empty()) {
        sql << "AND error_type = '" << escapeString(errorTypeFilter) << "' ";
    }

    sql << "ORDER BY time_bucket DESC LIMIT " << limit << " OFFSET " << offset;

    PGresult* res = PQexec(dbConnection_, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Read errors failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return result;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        AggregatedErrors agg;
        agg.timeBucket = parseTimestamp(PQgetvalue(res, i, 0));
        agg.projectId = PQgetvalue(res, i, 1);
        agg.page = PQgetvalue(res, i, 2);
        agg.errorType = PQgetvalue(res, i, 3);
        agg.errorsCount = std::stoll(PQgetvalue(res, i, 4));
        agg.warningCount = std::stoll(PQgetvalue(res, i, 5));
        agg.criticalCount = std::stoll(PQgetvalue(res, i, 6));
        agg.uniqueUsers = std::stoll(PQgetvalue(res, i, 7));
        result.push_back(agg);
    }

    PQclear(res);
    return result;
}

std::vector<AggregatedCustomEvents> Database::readCustomEvents(
    const std::string& projectId,
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to,
    const std::string& eventNameFilter,
    const std::string& pageFilter,
    int limit,
    int offset
) {
    std::vector<AggregatedCustomEvents> result;

    std::ostringstream sql;
    sql << "SELECT time_bucket, project_id, event_name, page, events_count, "
        << "unique_users, unique_sessions "
        << "FROM agg_custom_events WHERE project_id = '" << escapeString(projectId) << "' "
        << "AND time_bucket >= '" << formatTimestamp(from) << "' "
        << "AND time_bucket < '" << formatTimestamp(to) << "' ";

    if (!eventNameFilter.empty()) {
        sql << "AND event_name = '" << escapeString(eventNameFilter) << "' ";
    }
    if (!pageFilter.empty()) {
        sql << "AND page = '" << escapeString(pageFilter) << "' ";
    }

    sql << "ORDER BY time_bucket DESC LIMIT " << limit << " OFFSET " << offset;

    PGresult* res = PQexec(dbConnection_, sql.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Read custom events failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return result;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        AggregatedCustomEvents agg;
        agg.timeBucket = parseTimestamp(PQgetvalue(res, i, 0));
        agg.projectId = PQgetvalue(res, i, 1);
        agg.eventName = PQgetvalue(res, i, 2);
        agg.page = PQgetvalue(res, i, 3);
        agg.eventsCount = std::stoll(PQgetvalue(res, i, 4));
        agg.uniqueUsers = std::stoll(PQgetvalue(res, i, 5));
        agg.uniqueSessions = std::stoll(PQgetvalue(res, i, 6));
        result.push_back(agg);
    }

    PQclear(res);
    return result;
}

} // namespace aggregation


