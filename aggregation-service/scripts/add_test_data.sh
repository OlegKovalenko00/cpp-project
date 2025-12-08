#!/bin/bash
# filepath: /home/arsenchik/programming/cppprojects/CPP_second_year/cpp-project/aggregation-service/scripts/add_test_data.sh
# Скрипт для добавления тестовых данных в metrics-service

echo "Adding test data to metrics-service database..."

docker exec -i metrics-postgres psql -U metrics_user -d metrics_db << 'EOF'
-- Добавляем свежие page_views
INSERT INTO page_views (page, user_id, session_id, referrer, timestamp) VALUES
    ('/home', 'test-user-1', 'test-sess-1', 'https://google.com', NOW()),
    ('/products', 'test-user-1', 'test-sess-1', '/home', NOW() - INTERVAL '5 minutes'),
    ('/checkout', 'test-user-2', 'test-sess-2', '/products', NOW() - INTERVAL '2 minutes'),
    ('/about', 'test-user-3', 'test-sess-3', 'direct', NOW() - INTERVAL '10 minutes');

-- Добавляем свежие clicks
INSERT INTO click_events (page, element_id, action, user_id, session_id, timestamp) VALUES
    ('/home', 'buy-button', 'click', 'test-user-1', 'test-sess-1', NOW()),
    ('/products', 'add-to-cart', 'click', 'test-user-2', 'test-sess-2', NOW() - INTERVAL '3 minutes');

-- Добавляем свежие performance events
INSERT INTO performance_events (page, ttfb_ms, fcp_ms, lcp_ms, total_page_load_ms, user_id, session_id, timestamp) VALUES
    ('/home', 100.5, 250.2, 600.7, 950.3, 'test-user-1', 'test-sess-1', NOW()),
    ('/products', 120.1, 300.8, 750.4, 1100.6, 'test-user-2', 'test-sess-2', NOW() - INTERVAL '4 minutes'),
    ('/checkout', 150.0, 400.0, 900.0, 1400.0, 'test-user-1', 'test-sess-1', NOW() - INTERVAL '1 minute');

-- Добавляем свежие errors
INSERT INTO error_events (page, error_type, message, severity, user_id, session_id, timestamp) VALUES
    ('/checkout', 'NetworkError', 'Failed to fetch payment data', 2, 'test-user-1', 'test-sess-1', NOW()),
    ('/products', 'ValidationError', 'Invalid input', 1, 'test-user-2', 'test-sess-2', NOW() - INTERVAL '5 minutes');

-- Добавляем свежие custom events
INSERT INTO custom_events (name, page, user_id, session_id, timestamp) VALUES
    ('purchase_completed', '/checkout', 'test-user-1', 'test-sess-1', NOW()),
    ('newsletter_signup', '/home', 'test-user-3', 'test-sess-3', NOW() - INTERVAL '8 minutes');

SELECT 'Test data inserted successfully!' as status;
EOF

echo ""
echo "Verifying data counts:"
docker exec -i metrics-postgres psql -U metrics_user -d metrics_db << 'EOF'
SELECT 'page_views' as table_name, COUNT(*) as count FROM page_views
UNION ALL
SELECT 'click_events', COUNT(*) FROM click_events
UNION ALL
SELECT 'performance_events', COUNT(*) FROM performance_events
UNION ALL
SELECT 'error_events', COUNT(*) FROM error_events
UNION ALL
SELECT 'custom_events', COUNT(*) FROM custom_events;
EOF

echo ""
echo "Done! Now run test_grpc_connection to verify data retrieval."

