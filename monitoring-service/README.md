# Monitoring Service

`monitoring-service` checks the health of the MetricSys services and records monitoring data in PostgreSQL. It exposes HTTP endpoints for liveness, readiness, and uptime queries.

## Responsibilities

- Check `api-service`, `metrics-service`, and `aggregation-service` health endpoints.
- Store monitoring records in PostgreSQL.
- Expose uptime information by service and period.
- Serve its own liveness and readiness endpoints.

## Interfaces

| Interface | Port | Description |
| --- | --- | --- |
| HTTP | `8083` | Health and uptime API |
| PostgreSQL | Host port `5435` | Monitoring log storage |

HTTP endpoints:

- `GET /health/ping`
- `GET /health/ready`
- `GET /uptime?service=<name>[&period=day|week|month|year]`
- `GET /uptime/day?service=<name>`
- `GET /uptime/week?service=<name>`
- `GET /uptime/month?service=<name>`
- `GET /uptime/year?service=<name>`

## Run with Docker Compose

```bash
cd monitoring-service
docker compose up --build -d
```

This starts:

- `monitoring-service`
- PostgreSQL for monitoring records

The Compose configuration uses `host.docker.internal` to reach services running through the other Compose files.

## Local build

Requirements:

- CMake 3.14+
- C++23 compiler
- PostgreSQL client libraries
- `libpqxx`
- `pkg-config`

Build:

```bash
cd monitoring-service
mkdir -p build
cd build
cmake ..
cmake --build . --parallel
```

Run:

```bash
./monitoring-service
```

## Configuration

The Docker Compose file sets the main runtime configuration:

| Variable | Purpose |
| --- | --- |
| `HTTP_PORT` | HTTP server port |
| `POSTGRES_HOST`, `POSTGRES_DB`, `POSTGRES_USER`, `POSTGRES_PASSWORD` | PostgreSQL connection |
| `API_SERVICE_HOST`, `API_SERVICE_PORT` | API service health target |
| `METRICS_SERVICE_HOST`, `METRICS_SERVICE_PORT` | Metrics service health target |
| `AGGREGATION_SERVICE_HOST`, `AGGREGATION_SERVICE_PORT` | Aggregation service health target |
