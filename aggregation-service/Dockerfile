FROM gcc:13 AS builder

WORKDIR /app

RUN apt-get update && apt-get install -y \
    cmake \
    libpq-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/
COPY ../proto/ ../proto/

RUN mkdir -p build && cd build && cmake .. && make -j$(nproc)

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libpq5 \
    libgrpc++1.51 \
    libprotobuf32 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/aggregation-service .

EXPOSE 50052

CMD ["./aggregation-service"]
