FROM gcc:13 AS builder

WORKDIR /app

RUN apt-get update && apt-get install -y \
    cmake \
    libpq-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    pkg-config \
    libssl-dev \
    nlohmann-json3-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Скачиваем cpp-httplib вручную (header-only)
RUN git clone --depth 1 --branch v0.18.3 https://github.com/yhirose/cpp-httplib.git /opt/httplib || \
    (sleep 5 && git clone --depth 1 --branch v0.18.3 https://github.com/yhirose/cpp-httplib.git /opt/httplib)

# Копируем proto из корня проекта
COPY proto/ proto/

# Копируем файлы aggregation-service
COPY aggregation-service/CMakeLists.txt aggregation-service/
COPY aggregation-service/include/ aggregation-service/include/
COPY aggregation-service/src/ aggregation-service/src/

WORKDIR /app/aggregation-service

# Используем установленные пакеты вместо FetchContent
RUN mkdir -p build && cd build && \
    cmake .. \
    -DUSE_SYSTEM_HTTPLIB=ON \
    -DHTTPLIB_INCLUDE_DIR=/opt/httplib \
    && make -j$(nproc)

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libpq5 \
    libgrpc++1.51 \
    libprotobuf32 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/aggregation-service/build/aggregation-service .

EXPOSE 8081

CMD ["./aggregation-service"]
