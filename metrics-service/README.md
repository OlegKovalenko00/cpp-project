# Metrics Service

`metrics-service` consumes analytics events from RabbitMQ, stores raw metric data in PostgreSQL, and exposes raw metric reads over gRPC. It also provides HTTP health endpoints for service monitoring.

## Responsibilities

- Consume page-view, click, performance, error, and custom-event messages from RabbitMQ.
- Persist raw events in PostgreSQL.
- Expose raw metric query methods through `metricsys.MetricsService`.
- Serve liveness and readiness checks over HTTP.

## Interfaces

| Interface | Port | Description |
| --- | --- | --- |
| gRPC | `50051` | Raw metric query API |
| HTTP | `8082` | Health endpoints |
| PostgreSQL | `5432` on the host Compose setup | Raw metric storage |
| RabbitMQ | `5672`, management UI `15672` | Event queue dependency |

HTTP endpoints:

- `GET /health/ping`
- `GET /health/ready`
- `GET /health`
- `GET /ping`

gRPC methods are defined in [`../proto/metrics.proto`](../proto/metrics.proto).

## Run with Docker Compose

```bash
cd metrics-service
docker compose up --build -d
```

This starts:

- `metrics-service`
- PostgreSQL database `metrics_db`
- RabbitMQ with the management UI on `http://localhost:15672`

## Local build

Requirements:

- CMake 3.15+
- C++23 compiler
- Protobuf and gRPC development packages
- `librabbitmq`
- PostgreSQL client libraries

Build:

```bash
cd metrics-service
mkdir -p build
cd build
cmake ..
cmake --build . --parallel
```

Run:

```bash
./metrics-service
```

## Tests

```bash
cd metrics-service
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
ctest --output-on-failure
```

## Configuration

The Docker Compose file sets the main runtime configuration:

| Variable | Purpose |
| --- | --- |
| `GRPC_PORT` | gRPC server port |
| `HTTP_PORT` | HTTP health server port |
| `POSTGRES_HOST`, `POSTGRES_DB`, `POSTGRES_USER`, `POSTGRES_PASSWORD` | PostgreSQL connection |
| `RABBITMQ_HOST`, `RABBITMQ_PORT`, `RABBITMQ_USER`, `RABBITMQ_PASSWORD` | RabbitMQ connection |
| `RABBITMQ_QUEUE` | Queue used for metric events |
