# Aggregation Service

`aggregation-service` reads raw events from `metrics-service` over gRPC, builds 5-minute metric buckets, and stores the aggregated results in PostgreSQL. It exposes aggregated data over gRPC and serves HTTP health endpoints.

## Responsibilities

- Fetch raw page-view, click, performance, error, and custom-event data from `metrics-service`.
- Aggregate metrics into 5-minute buckets.
- Store aggregated results and aggregation watermark data in PostgreSQL.
- Expose aggregated metric reads through `metricsys.aggregation.AggregationService`.
- Serve HTTP health checks.

## Interfaces

| Interface | Port | Description |
| --- | --- | --- |
| gRPC | `50052` inside the Compose network | Aggregated metric API |
| HTTP | `8081` | Health endpoints |
| PostgreSQL | Host port `5434` | Aggregated metric storage |

HTTP endpoints:

- `GET /health/ping`
- `GET /health`
- `GET /ping`

gRPC methods are defined in [`../proto/aggregation.proto`](../proto/aggregation.proto).

## Run with Docker Compose

Start `metrics-service` first so the shared Docker network exists, or use the root `build.sh` script to start the full stack.

```bash
cd aggregation-service
docker compose up --build -d
```

This starts:

- `aggregation-service`
- PostgreSQL database `aggregation_db`

## Local build

Requirements:

- CMake 3.16+
- C++23 compiler
- Protobuf and gRPC development packages
- PostgreSQL client libraries
- OpenSSL

Build:

```bash
cd aggregation-service
mkdir -p build
cd build
cmake ..
cmake --build . --parallel
```

Run:

```bash
./aggregation-service
```

## Tests

```bash
cd aggregation-service
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
| `AGG_DB_HOST`, `AGG_DB_PORT`, `AGG_DB_NAME`, `AGG_DB_USER`, `AGG_DB_PASSWORD` | PostgreSQL connection |
| `AGG_HTTP_HOST`, `AGG_HTTP_PORT` | HTTP health server bind address and port |
| `METRICS_GRPC_HOST`, `METRICS_GRPC_PORT` | Raw metric gRPC source |
