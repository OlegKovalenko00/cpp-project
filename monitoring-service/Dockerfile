# syntax=docker/dockerfile:1

# Build stage
FROM gcc:13 AS build

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        cmake make pkg-config git \
        libpqxx-dev libpq-dev && \
    rm -rf /var/lib/apt/lists/*

# Copy minimal sources
COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/

# Configure and build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release -- -j"$(nproc)"

# Runtime stage
FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libpqxx-7 libpq5 && \
    rm -rf /var/lib/apt/lists/*

COPY --from=build /app/build/monitoring-service /usr/local/bin/monitoring-service

# Expose service port
EXPOSE 8083

# Configure DB connection with env vars if needed
ENV PGHOST=localhost \
    PGUSER=postgres \
    PGPASSWORD=postgres \
    PGDATABASE=postgres

CMD ["/usr/local/bin/monitoring-service"]
