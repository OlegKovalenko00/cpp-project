# MetricSys

MetricSys is a self-hosted C++ microservice pipeline for collecting, storing, aggregating, and monitoring web analytics metrics.

MetricSys is an early-stage open-source infrastructure project. It is organized as a set of small services that exchange analytics events through REST, RabbitMQ, gRPC, and PostgreSQL. The current codebase focuses on the core data path for page views, clicks, performance metrics, client-side errors, custom events, aggregation, and service health monitoring.

## Features

- REST API for ingesting web analytics events.
- RabbitMQ-backed event delivery between ingestion and storage.
- PostgreSQL storage for raw and aggregated metrics.
- gRPC APIs for reading raw metrics and aggregated 5-minute buckets.
- Health-check endpoints for individual services.
- Docker Compose setup for running the services and their dependencies.
- OpenAPI specifications for the public HTTP API.

## Architecture

MetricSys is split into four C++ services:

| Service | Role | Default ports |
| --- | --- | --- |
| `api-service` | Receives metrics over REST and publishes events to RabbitMQ. It also proxies aggregation and uptime requests. | HTTP `8080` |
| `metrics-service` | Consumes metric events from RabbitMQ and stores raw data in PostgreSQL. It exposes raw metric reads over gRPC and health checks over HTTP. | gRPC `50051`, HTTP `8082` |
| `aggregation-service` | Reads raw metrics from `metrics-service` over gRPC and builds 5-minute metric buckets in PostgreSQL. It exposes aggregated data over gRPC and health checks over HTTP. | gRPC `50052`, HTTP `8081` |
| `monitoring-service` | Checks service health and exposes monitoring information, including uptime data backed by PostgreSQL. | HTTP `8083` |

The main event flow is:

1. Clients send metric events to `api-service` over REST.
2. `api-service` publishes events to RabbitMQ.
3. `metrics-service` consumes events and writes them to PostgreSQL.
4. `aggregation-service` reads raw metrics through gRPC and stores 5-minute aggregates.
5. `monitoring-service` checks service health and records monitoring data.

## Tech stack

- C++23
- REST over HTTP
- RabbitMQ
- gRPC
- PostgreSQL
- Protobuf
- Docker and Docker Compose
- CMake
- OpenAPI

The service implementations also use libraries such as `cpp-httplib`, `nlohmann/json`, `libpqxx`, `rabbitmq-c`, and Google Test.

## Getting started

### Requirements

- Docker with Docker Compose support
- Bash-compatible shell for the root startup script

### Run the full stack

From the repository root:

```bash
bash build.sh
```

The script starts the service Compose files in dependency order:

1. `metrics-service/docker-compose.yml`
2. `aggregation-service/docker-compose.yml`
3. `monitoring-service/docker-compose.yml`
4. `api-service/docker-compose.yml`

The API service should then be available at:

```bash
curl http://localhost:8080/health/ping
```

RabbitMQ management is exposed at `http://localhost:15672` with the default `guest` / `guest` credentials from the Compose configuration.

### Run services individually

Each service has its own Docker Compose file. For example:

```bash
cd metrics-service
docker compose up --build -d
```

When starting services individually, start `metrics-service` first because it creates the shared Docker network used by the other service Compose files.

### Send sample events

The repository includes a small Python client for generating sample analytics traffic:

```bash
pip3 install faker pyyaml requests
python3 client.py --base-url http://localhost:8080 --rate 2 --workers 1
```

## API documentation

The HTTP API is documented in:

- [`openapi.yaml`](openapi.yaml)
- [`api-service/openapi.yaml`](api-service/openapi.yaml)

Service-specific notes are available in each service directory:

- [`api-service/README.md`](api-service/README.md)
- [`metrics-service/README.md`](metrics-service/README.md)
- [`aggregation-service/README.md`](aggregation-service/README.md)
- [`monitoring-service/README.md`](monitoring-service/README.md)

## Repository layout

```text
.
|-- api-service/          REST API and RabbitMQ publisher
|-- metrics-service/      RabbitMQ consumer, raw metric storage, gRPC API
|-- aggregation-service/  5-minute metric aggregation service
|-- monitoring-service/   Health monitoring and uptime service
|-- proto/                Shared Protobuf definitions
|-- openapi.yaml          Root OpenAPI specification
|-- build.sh              Starts the Docker Compose services
|-- client.py             Sample traffic generator
`-- README.md
```

## Project status

MetricSys is early-stage and under active development. The core services, Docker setup, schemas, and APIs are present, but the project should still be treated as evolving infrastructure rather than a production-ready analytics platform.

## License

MetricSys is released under the MIT License. See [`LICENSE`](LICENSE) for details.
