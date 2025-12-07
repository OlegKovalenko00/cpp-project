#include "metrics.h"
#include <pqxx/pqxx>
#include <iostream>
#include <sstream>

MetricsServiceImpl::MetricsServiceImpl(const DatabaseConfig& db_config)
    : db_config_(db_config) {}

std::string MetricsServiceImpl::get_connection_string() const {
    return "host=" + db_config_.host +
           " dbname=" + db_config_.dbname +
           " user=" + db_config_.user +
           " password=" + db_config_.password;
}

grpc::Status MetricsServiceImpl::GetPageViews(
    grpc::ServerContext* context,
    const metricsys::GetPageViewsRequest* request,
    metricsys::GetPageViewsResponse* response) {
    
    try {
        pqxx::connection conn(get_connection_string());
        pqxx::work tx(conn);

        std::stringstream query;
        query << "SELECT id, page, user_id, session_id, referrer, "
              << "EXTRACT(EPOCH FROM timestamp)::bigint as ts "
              << "FROM page_views WHERE 1=1";

        if (request->has_time_range()) {
            query << " AND timestamp >= to_timestamp(" 
                  << request->time_range().start_timestamp() << ")"
                  << " AND timestamp <= to_timestamp(" 
                  << request->time_range().end_timestamp() << ")";
        }

        if (request->has_page_filter() && !request->page_filter().empty()) {
            query << " AND page LIKE " << tx.quote("%" + request->page_filter() + "%");
        }

        if (request->has_user_id_filter() && !request->user_id_filter().empty()) {
            query << " AND user_id = " << tx.quote(request->user_id_filter());
        }

        if (request->has_pagination()) {
            query << " LIMIT " << request->pagination().limit()
                  << " OFFSET " << request->pagination().offset();
        } else {
            query << " LIMIT 100";
        }

        pqxx::result result = tx.exec(query.str());
        tx.commit();

        for (const auto& row : result) {
            auto* event = response->add_events();
            event->set_id(row["id"].as<std::string>());
            event->set_page(row["page"].as<std::string>());
            event->set_user_id(row["user_id"].as<std::string>(""));
            event->set_session_id(row["session_id"].as<std::string>(""));
            event->set_referrer(row["referrer"].as<std::string>(""));
            event->set_timestamp(row["ts"].as<int64_t>(0));
        }

        response->set_total_count(result.size());
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        std::cerr << "GetPageViews error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status MetricsServiceImpl::GetClicks(
    grpc::ServerContext* context,
    const metricsys::GetClicksRequest* request,
    metricsys::GetClicksResponse* response) {
    
    try {
        pqxx::connection conn(get_connection_string());
        pqxx::work tx(conn);

        std::stringstream query;
        query << "SELECT id, page, element_id, action, user_id, session_id, "
              << "EXTRACT(EPOCH FROM timestamp)::bigint as ts "
              << "FROM click_events WHERE 1=1";

        if (request->has_time_range()) {
            query << " AND timestamp >= to_timestamp(" 
                  << request->time_range().start_timestamp() << ")"
                  << " AND timestamp <= to_timestamp(" 
                  << request->time_range().end_timestamp() << ")";
        }

        if (request->has_page_filter() && !request->page_filter().empty()) {
            query << " AND page LIKE " << tx.quote("%" + request->page_filter() + "%");
        }

        if (request->has_element_id_filter() && !request->element_id_filter().empty()) {
            query << " AND element_id LIKE " << tx.quote("%" + request->element_id_filter() + "%");
        }

        if (request->has_user_id_filter() && !request->user_id_filter().empty()) {
            query << " AND user_id = " << tx.quote(request->user_id_filter());
        }

        if (request->has_pagination()) {
            query << " LIMIT " << request->pagination().limit()
                  << " OFFSET " << request->pagination().offset();
        } else {
            query << " LIMIT 100";
        }

        pqxx::result result = tx.exec(query.str());
        tx.commit();

        for (const auto& row : result) {
            auto* event = response->add_events();
            event->set_id(row["id"].as<std::string>());
            event->set_page(row["page"].as<std::string>());
            event->set_element_id(row["element_id"].as<std::string>(""));
            event->set_action(row["action"].as<std::string>(""));
            event->set_user_id(row["user_id"].as<std::string>(""));
            event->set_session_id(row["session_id"].as<std::string>(""));
            event->set_timestamp(row["ts"].as<int64_t>(0));
        }

        response->set_total_count(result.size());
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        std::cerr << "GetClicks error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status MetricsServiceImpl::GetPerformance(
    grpc::ServerContext* context,
    const metricsys::GetPerformanceRequest* request,
    metricsys::GetPerformanceResponse* response) {
    
    try {
        pqxx::connection conn(get_connection_string());
        pqxx::work tx(conn);

        std::stringstream query;
        query << "SELECT id, page, ttfb_ms, fcp_ms, lcp_ms, total_page_load_ms, "
              << "user_id, session_id, "
              << "EXTRACT(EPOCH FROM timestamp)::bigint as ts "
              << "FROM performance_events WHERE 1=1";

        if (request->has_time_range()) {
            query << " AND timestamp >= to_timestamp(" 
                  << request->time_range().start_timestamp() << ")"
                  << " AND timestamp <= to_timestamp(" 
                  << request->time_range().end_timestamp() << ")";
        }

        if (request->has_page_filter() && !request->page_filter().empty()) {
            query << " AND page LIKE " << tx.quote("%" + request->page_filter() + "%");
        }

        if (request->has_user_id_filter() && !request->user_id_filter().empty()) {
            query << " AND user_id = " << tx.quote(request->user_id_filter());
        }

        if (request->has_pagination()) {
            query << " LIMIT " << request->pagination().limit()
                  << " OFFSET " << request->pagination().offset();
        } else {
            query << " LIMIT 100";
        }

        pqxx::result result = tx.exec(query.str());
        tx.commit();

        for (const auto& row : result) {
            auto* event = response->add_events();
            event->set_id(row["id"].as<std::string>());
            event->set_page(row["page"].as<std::string>());
            event->set_ttfb_ms(row["ttfb_ms"].as<double>(0));
            event->set_fcp_ms(row["fcp_ms"].as<double>(0));
            event->set_lcp_ms(row["lcp_ms"].as<double>(0));
            event->set_total_page_load_ms(row["total_page_load_ms"].as<double>(0));
            event->set_user_id(row["user_id"].as<std::string>(""));
            event->set_session_id(row["session_id"].as<std::string>(""));
            event->set_timestamp(row["ts"].as<int64_t>(0));
        }

        response->set_total_count(result.size());
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        std::cerr << "GetPerformance error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status MetricsServiceImpl::GetErrors(
    grpc::ServerContext* context,
    const metricsys::GetErrorsRequest* request,
    metricsys::GetErrorsResponse* response) {
    
    try {
        pqxx::connection conn(get_connection_string());
        pqxx::work tx(conn);

        std::stringstream query;
        query << "SELECT id, page, error_type, message, stack, severity, "
              << "user_id, session_id, "
              << "EXTRACT(EPOCH FROM timestamp)::bigint as ts "
              << "FROM error_events WHERE 1=1";

        if (request->has_time_range()) {
            query << " AND timestamp >= to_timestamp(" 
                  << request->time_range().start_timestamp() << ")"
                  << " AND timestamp <= to_timestamp(" 
                  << request->time_range().end_timestamp() << ")";
        }

        if (request->has_page_filter() && !request->page_filter().empty()) {
            query << " AND page LIKE " << tx.quote("%" + request->page_filter() + "%");
        }

        if (request->has_error_type_filter() && !request->error_type_filter().empty()) {
            query << " AND error_type = " << tx.quote(request->error_type_filter());
        }

        if (request->has_severity_filter() && request->severity_filter() != metricsys::SEVERITY_UNSPECIFIED) {
            query << " AND severity >= " << request->severity_filter();
        }

        if (request->has_user_id_filter() && !request->user_id_filter().empty()) {
            query << " AND user_id = " << tx.quote(request->user_id_filter());
        }

        if (request->has_pagination()) {
            query << " LIMIT " << request->pagination().limit()
                  << " OFFSET " << request->pagination().offset();
        } else {
            query << " LIMIT 100";
        }

        pqxx::result result = tx.exec(query.str());
        tx.commit();

        for (const auto& row : result) {
            auto* event = response->add_events();
            event->set_id(row["id"].as<std::string>());
            event->set_page(row["page"].as<std::string>());
            event->set_error_type(row["error_type"].as<std::string>(""));
            event->set_message(row["message"].as<std::string>(""));
            event->set_stack(row["stack"].as<std::string>(""));
            event->set_severity(static_cast<metricsys::Severity>(row["severity"].as<int>(0)));
            event->set_user_id(row["user_id"].as<std::string>(""));
            event->set_session_id(row["session_id"].as<std::string>(""));
            event->set_timestamp(row["ts"].as<int64_t>(0));
        }

        response->set_total_count(result.size());
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        std::cerr << "GetErrors error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status MetricsServiceImpl::GetCustomEvents(
    grpc::ServerContext* context,
    const metricsys::GetCustomEventsRequest* request,
    metricsys::GetCustomEventsResponse* response) {
    
    try {
        pqxx::connection conn(get_connection_string());
        pqxx::work tx(conn);

        std::stringstream query;
        query << "SELECT id, name, page, user_id, session_id, "
              << "EXTRACT(EPOCH FROM timestamp)::bigint as ts "
              << "FROM custom_events WHERE 1=1";

        if (request->has_time_range()) {
            query << " AND timestamp >= to_timestamp(" 
                  << request->time_range().start_timestamp() << ")"
                  << " AND timestamp <= to_timestamp(" 
                  << request->time_range().end_timestamp() << ")";
        }

        if (request->has_name_filter() && !request->name_filter().empty()) {
            query << " AND name = " << tx.quote(request->name_filter());
        }

        if (request->has_page_filter() && !request->page_filter().empty()) {
            query << " AND page LIKE " << tx.quote("%" + request->page_filter() + "%");
        }

        if (request->has_user_id_filter() && !request->user_id_filter().empty()) {
            query << " AND user_id = " << tx.quote(request->user_id_filter());
        }

        if (request->has_pagination()) {
            query << " LIMIT " << request->pagination().limit()
                  << " OFFSET " << request->pagination().offset();
        } else {
            query << " LIMIT 100";
        }

        pqxx::result result = tx.exec(query.str());
        tx.commit();

        for (const auto& row : result) {
            auto* event = response->add_events();
            event->set_id(row["id"].as<std::string>());
            event->set_name(row["name"].as<std::string>());
            event->set_page(row["page"].as<std::string>(""));
            event->set_user_id(row["user_id"].as<std::string>(""));
            event->set_session_id(row["session_id"].as<std::string>(""));
            event->set_timestamp(row["ts"].as<int64_t>(0));
        }

        response->set_total_count(result.size());
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        std::cerr << "GetCustomEvents error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

void run_grpc_server(const std::string& address, const DatabaseConfig& db_config) {
    MetricsServiceImpl service(db_config);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Metrics gRPC server listening on " << address << std::endl;

    server->Wait();
}
