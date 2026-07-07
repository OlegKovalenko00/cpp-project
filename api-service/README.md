# API Service

`api-service` is the public HTTP entry point for MetricSys. It receives web analytics events over REST, validates request payloads, and publishes accepted events to RabbitMQ for asynchronous processing by `metrics-service`.

The service also exposes proxy endpoints for uptime data from `monitoring-service` and aggregated metric reads from `aggregation-service`.

## Endpoints

| Method | Path | Description |
| --- | --- | --- |
| `GET` | `/health/ping` | Liveness check |
| `POST` | `/page-views` | Submit a page-view event |
| `POST` | `/clicks` | Submit a click event |
| `POST` | `/performance` | Submit performance timing metrics |
| `POST` | `/errors` | Submit a client-side error event |
| `POST` | `/custom-events` | Submit a custom analytics event |
| `GET` | `/uptime` | Proxy uptime data from `monitoring-service` |
| `GET` | `/uptime/day` | Proxy daily uptime data |
| `GET` | `/uptime/week` | Proxy weekly uptime data |
| `GET` | `/uptime/month` | Proxy monthly uptime data |
| `GET` | `/uptime/year` | Proxy yearly uptime data |
| `GET` | `/aggregation/watermark` | Read the latest aggregation watermark |
| `POST` | `/aggregation/page-views` | Read aggregated page-view buckets |
| `POST` | `/aggregation/clicks` | Read aggregated click buckets |
| `POST` | `/aggregation/performance` | Read aggregated performance buckets |
| `POST` | `/aggregation/errors` | Read aggregated error buckets |
| `POST` | `/aggregation/custom-events` | Read aggregated custom-event buckets |

The OpenAPI specification is available in [`openapi.yaml`](openapi.yaml).

## Run with Docker Compose

Start `metrics-service` first so the shared Docker network exists, or use the root `build.sh` script to start the full stack.

```bash
cd api-service
docker compose up --build -d
```

The service listens on `http://localhost:8080`.

## Local build

Requirements:

- CMake 3.15+
- C++23 compiler
- Protobuf and gRPC development packages
- `librabbitmq`
- PostgreSQL client libraries

Build:

```bash
cd api-service
mkdir -p build
cd build
cmake ..
cmake --build . --parallel
```

Run:

```bash
./api-service
```

## Tests

```bash
cd api-service
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
| `API_HOST`, `API_PORT` | HTTP bind address and port |
| `RABBITMQ_HOST`, `RABBITMQ_PORT` | RabbitMQ connection |
| `RABBITMQ_USERNAME`, `RABBITMQ_PASSWORD` | RabbitMQ credentials |
| `AGGREGATION_GRPC_HOST`, `AGGREGATION_GRPC_PORT` | Aggregation gRPC target |
| `MONITORING_HTTP_HOST`, `MONITORING_HTTP_PORT` | Monitoring HTTP target |
