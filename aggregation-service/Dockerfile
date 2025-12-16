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
    git \
    && rm -rf /var/lib/apt/lists/*

# Копируем proto из корня проекта
COPY proto/ proto/

# Копируем файлы aggregation-service
COPY aggregation-service/CMakeLists.txt aggregation-service/
COPY aggregation-service/include/ aggregation-service/include/
COPY aggregation-service/src/ aggregation-service/src/
COPY aggregation-service/tests/ aggregation-service/tests/

WORKDIR /app/aggregation-service

# Собираем проект - httplib и json скачиваются через CMake FetchContent
RUN mkdir -p build && cd build && \
    cmake .. && make -j$(nproc)

FROM gcc:13-bookworm

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libpq5 \
    libgrpc++1.51 \
    libprotobuf32 \
    libssl3 \
    libbrotli1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/aggregation-service/build/aggregation-service .

EXPOSE 8081

CMD ["./aggregation-service"]