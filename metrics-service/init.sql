CREATE TABLE IF NOT EXISTS page_views (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    page VARCHAR(512) NOT NULL,
    user_id VARCHAR(128),
    session_id VARCHAR(128),
    referrer VARCHAR(512),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS click_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    page VARCHAR(512) NOT NULL,
    element_id VARCHAR(256),
    action VARCHAR(256),
    user_id VARCHAR(128),
    session_id VARCHAR(128),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS performance_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    page VARCHAR(512) NOT NULL,
    ttfb_ms DOUBLE PRECISION,
    fcp_ms DOUBLE PRECISION,
    lcp_ms DOUBLE PRECISION,
    total_page_load_ms DOUBLE PRECISION,
    user_id VARCHAR(128),
    session_id VARCHAR(128),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS error_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    page VARCHAR(512) NOT NULL,
    error_type VARCHAR(128),
    message TEXT,
    stack TEXT,
    severity INTEGER DEFAULT 0,
    user_id VARCHAR(128),
    session_id VARCHAR(128),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS custom_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(256) NOT NULL,
    page VARCHAR(512),
    user_id VARCHAR(128),
    session_id VARCHAR(128),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_page_views_timestamp ON page_views(timestamp);
CREATE INDEX IF NOT EXISTS idx_page_views_page ON page_views(page);
CREATE INDEX IF NOT EXISTS idx_page_views_user_id ON page_views(user_id);

CREATE INDEX IF NOT EXISTS idx_click_events_timestamp ON click_events(timestamp);
CREATE INDEX IF NOT EXISTS idx_click_events_page ON click_events(page);

CREATE INDEX IF NOT EXISTS idx_performance_events_timestamp ON performance_events(timestamp);
CREATE INDEX IF NOT EXISTS idx_performance_events_page ON performance_events(page);

CREATE INDEX IF NOT EXISTS idx_error_events_timestamp ON error_events(timestamp);
CREATE INDEX IF NOT EXISTS idx_error_events_severity ON error_events(severity);

CREATE INDEX IF NOT EXISTS idx_custom_events_timestamp ON custom_events(timestamp);
CREATE INDEX IF NOT EXISTS idx_custom_events_name ON custom_events(name);

INSERT INTO page_views (page, user_id, session_id, referrer) VALUES
    ('/home', 'user-001', 'sess-001', 'https://google.com'),
    ('/products', 'user-001', 'sess-001', '/home'),
    ('/about', 'user-002', 'sess-002', 'https://bing.com');

INSERT INTO click_events (page, element_id, action, user_id, session_id) VALUES
    ('/home', 'btn-signup', 'click', 'user-001', 'sess-001'),
    ('/products', 'product-1', 'view', 'user-001', 'sess-001');

INSERT INTO performance_events (page, ttfb_ms, fcp_ms, lcp_ms, total_page_load_ms, user_id, session_id) VALUES
    ('/home', 120.5, 350.2, 800.7, 1200.3, 'user-001', 'sess-001'),
    ('/products', 150.1, 400.8, 950.4, 1500.6, 'user-002', 'sess-002');

INSERT INTO error_events (page, error_type, message, severity, user_id, session_id) VALUES
    ('/checkout', 'TypeError', 'Cannot read property of undefined', 2, 'user-001', 'sess-001');

INSERT INTO custom_events (name, page, user_id, session_id) VALUES
    ('purchase', '/checkout', 'user-001', 'sess-001');
