CREATE TABLE logs (
    service_name VARCHAR(255) NOT NULL,
    log_message TEXT NOT NULL,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);
