# Build stage
FROM gcc:13 AS build

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    cmake make pkg-config git \
    libpqxx-dev libpq-dev && \
    rm -rf /var/lib/apt/lists/*

COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release -- -j"$(nproc)"

# Runtime stage
FROM gcc:13-bookworm

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libpqxx-dev libpq5 && \
    rm -rf /var/lib/apt/lists/*

COPY --from=build /app/build/monitoring-service .

# Expose service port
EXPOSE 8083


CMD ["./monitoring-service"]
