set(TEST_SOURCES
    main.cpp
    test_database.cpp
    test_metrics_service.cpp
    test_message_processor.cpp
    test_http_handler.cpp
)

add_executable(metrics_service_tests ${TEST_SOURCES})
target_link_libraries(metrics_service_tests
    PRIVATE
        metrics_service_core
        httplib::httplib
        nlohmann_json::nlohmann_json
        gtest_main
)
target_include_directories(metrics_service_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME metrics_service_tests COMMAND metrics_service_tests)

set(TEST_ENV_VARS
    "POSTGRES_HOST=localhost"
    "POSTGRES_DB=metrics_db"
    "POSTGRES_USER=metrics_user"
    "POSTGRES_PASSWORD=metrics_password"
)
set_tests_properties(metrics_service_tests PROPERTIES ENVIRONMENT "${TEST_ENV_VARS}")
