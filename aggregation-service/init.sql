-- Aggregation Service Database Schema
-- This file initializes the database schema for aggregation-service

-- Table for aggregated page view events
CREATE TABLE IF NOT EXISTS agg_page_views (
    id SERIAL PRIMARY KEY,
    time_bucket TIMESTAMPTZ NOT NULL,
    project_id TEXT NOT NULL,
    page TEXT NOT NULL,
    views_count BIGINT NOT NULL DEFAULT 0,
    unique_users BIGINT DEFAULT 0,
    unique_sessions BIGINT DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(time_bucket, project_id, page)
);

CREATE INDEX IF NOT EXISTS idx_agg_page_views_time ON agg_page_views(time_bucket);
CREATE INDEX IF NOT EXISTS idx_agg_page_views_project ON agg_page_views(project_id, page);

-- Table for aggregated click events
CREATE TABLE IF NOT EXISTS agg_clicks (
    id SERIAL PRIMARY KEY,
    time_bucket TIMESTAMPTZ NOT NULL,
    project_id TEXT NOT NULL,
    page TEXT NOT NULL,
    element_id TEXT,
    clicks_count BIGINT NOT NULL DEFAULT 0,
    unique_users BIGINT DEFAULT 0,
    unique_sessions BIGINT DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(time_bucket, project_id, page, element_id)
);

CREATE INDEX IF NOT EXISTS idx_agg_clicks_time ON agg_clicks(time_bucket);
CREATE INDEX IF NOT EXISTS idx_agg_clicks_project ON agg_clicks(project_id, page);

-- Table for aggregated performance metrics
CREATE TABLE IF NOT EXISTS agg_performance (
    id SERIAL PRIMARY KEY,
    time_bucket TIMESTAMPTZ NOT NULL,
    project_id TEXT NOT NULL,
    page TEXT NOT NULL,
    samples_count BIGINT NOT NULL DEFAULT 0,
    avg_total_load_ms DOUBLE PRECISION,
    p95_total_load_ms DOUBLE PRECISION,
    avg_ttfb_ms DOUBLE PRECISION,
    p95_ttfb_ms DOUBLE PRECISION,
    avg_fcp_ms DOUBLE PRECISION,
    p95_fcp_ms DOUBLE PRECISION,
    avg_lcp_ms DOUBLE PRECISION,
    p95_lcp_ms DOUBLE PRECISION,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(time_bucket, project_id, page)
);

CREATE INDEX IF NOT EXISTS idx_agg_performance_time ON agg_performance(time_bucket);
CREATE INDEX IF NOT EXISTS idx_agg_performance_project ON agg_performance(project_id, page);

-- Table for aggregated error events
CREATE TABLE IF NOT EXISTS agg_errors (
    id SERIAL PRIMARY KEY,
    time_bucket TIMESTAMPTZ NOT NULL,
    project_id TEXT NOT NULL,
    page TEXT NOT NULL,
    error_type TEXT,
    errors_count BIGINT NOT NULL DEFAULT 0,
    warning_count BIGINT DEFAULT 0,
    critical_count BIGINT DEFAULT 0,
    unique_users BIGINT DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(time_bucket, project_id, page, error_type)
);

CREATE INDEX IF NOT EXISTS idx_agg_errors_time ON agg_errors(time_bucket);
CREATE INDEX IF NOT EXISTS idx_agg_errors_project ON agg_errors(project_id, page);

-- Table for aggregated custom events
CREATE TABLE IF NOT EXISTS agg_custom_events (
    id SERIAL PRIMARY KEY,
    time_bucket TIMESTAMPTZ NOT NULL,
    project_id TEXT NOT NULL,
    event_name TEXT NOT NULL,
    page TEXT,
    events_count BIGINT NOT NULL DEFAULT 0,
    unique_users BIGINT DEFAULT 0,
    unique_sessions BIGINT DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(time_bucket, project_id, event_name, page)
);

CREATE INDEX IF NOT EXISTS idx_agg_custom_events_time ON agg_custom_events(time_bucket);
CREATE INDEX IF NOT EXISTS idx_agg_custom_events_project ON agg_custom_events(project_id, event_name);

-- Watermark table for tracking aggregation progress
CREATE TABLE IF NOT EXISTS aggregation_watermark (
    id INTEGER PRIMARY KEY,
    last_aggregated_at TIMESTAMPTZ NOT NULL
);

-- Initialize watermark to epoch start
INSERT INTO aggregation_watermark (id, last_aggregated_at)
VALUES (1, '1970-01-01T00:00:00Z')
ON CONFLICT (id) DO NOTHING;

