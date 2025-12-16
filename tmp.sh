# Health check
echo "1\n"
curl http://localhost:8080/health/ping
# {"status":"ok"}
echo "2\n"
# Отправка page view
curl -X POST http://localhost:8080/page-views \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "timestamp": 1733505600}'
# {"status":"accepted"}
echo "3\n"

# Отправка клика
curl -X POST http://localhost:8080/clicks \
  -H "Content-Type: application/json" \
  -d '{"page": "/home", "element_id": "btn-signup", "timestamp": 1733505600}'
# {"status":"accepted"}
echo "4\n"

# Отправка performance
curl -X POST http://localhost:8080/performance \
  -H "Content-Type: application/json" \
  -d '{"page": "/dashboard", "ttfb_ms": 120, "timestamp": 1733505610}'
# {"status":"accepted"}
echo "5\n"

# Отправка ошибки
curl -X POST http://localhost:8080/errors \
  -H "Content-Type: application/json" \
  -d '{"page": "/dashboard", "error_type": "js_exception", "message": "Oops", "timestamp": 1733505615}'
# {"status":"accepted"}
echo "6\n"

# Отправка кастомного события
curl -X POST http://localhost:8080/custom-events \
  -H "Content-Type: application/json" \
  -d '{"name": "signup_completed", "page": "/signup/success", "timestamp": 1733505620}'
# {"status":"accepted"}
echo "7\n"
# Uptime (все периоды)
curl "http://localhost:8080/uptime?service=api-service"
# {"service":"api-service","period":"all","periods":{"day":{"ok":0,"total":0,"percent":0},"week":{"ok":0,"total":0,"percent":0},"month":{"ok":0,"total":0,"percent":0},"year":{"ok":0,"total":0,"percent":0}}}

# Uptime с периодом
echo "8\n"
curl "http://localhost:8080/uptime?service=api-service&period=day"
# {"service":"api-service","period":"day","periods":{"day":{"ok":0,"total":0,"percent":0}}}

echo "9\n"

# Uptime fixed endpoints
curl "http://localhost:8080/uptime/day?service=api-service"
echo "10\n"

curl "http://localhost:8080/uptime/week?service=metrics-service"
# {"service":"metrics-service","period":"week","periods":{"week":{"ok":0,"total":0,"percent":0}}}
echo "11\n"

# Aggregation watermark
curl http://localhost:8080/aggregation/watermark
# {"last_aggregated_at":"2024-12-06T10:05:00Z"}
echo "12\n"

# Aggregation page-views
curl -X POST http://localhost:8080/aggregation/page-views \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/dashboard", "pagination": {"limit": 100, "offset": 0}}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/dashboard","views_count":2400,"unique_users":350,"unique_sessions":420,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation clicks
echo "13\n"

curl -X POST http://localhost:8080/aggregation/clicks \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/pricing", "element_id": "cta-subscribe-button"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/pricing","element_id":"cta-subscribe-button","clicks_count":180,"unique_users":120,"unique_sessions":140,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation performance
echo "14\n"

curl -X POST http://localhost:8080/aggregation/performance \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/dashboard"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/dashboard","samples_count":500,"avg_total_load_ms":1100,"p95_total_load_ms":1500,"avg_ttfb_ms":120,"p95_ttfb_ms":200,"avg_fcp_ms":450,"p95_fcp_ms":600,"avg_lcp_ms":900,"p95_lcp_ms":1250,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation errors
echo "15\n"
curl -X POST http://localhost:8080/aggregation/errors \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "page": "/dashboard", "error_type": "js_exception"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","page":"/dashboard","error_type":"js_exception","errors_count":45,"warning_count":12,"critical_count":8,"unique_users":30,"created_at":"2024-12-06T10:05:00Z"}]}

# Aggregation custom events
echo "16\n"
curl -X POST http://localhost:8080/aggregation/custom-events \
  -H "Content-Type: application/json" \
  -d '{"project_id": "proj_123", "time_range": {"from": "2024-12-06T09:00:00Z", "to": "2024-12-06T10:00:00Z"}, "event_name": "signup_completed", "page": "/signup/success"}'
# {"rows":[{"time_bucket":"2024-12-06T10:00:00Z","project_id":"proj_123","event_name":"signup_completed","page":"/signup/success","events_count":75,"unique_users":60,"unique_sessions":65,"created_at":"2024-12-06T10:05:00Z"}]}