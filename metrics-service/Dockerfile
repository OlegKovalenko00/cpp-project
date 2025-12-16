FROM gcc:13 AS builder

WORKDIR /app

RUN apt-get update && apt-get install -y \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    libpq-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    librabbitmq-dev \
    && rm -rf /var/lib/apt/lists/*

COPY proto/ proto/
COPY metrics-service/CMakeLists.txt metrics-service/
COPY metrics-service/include/ metrics-service/include/
COPY metrics-service/src/ metrics-service/src/
COPY metrics-service/tests/ metrics-service/tests/

WORKDIR /app/metrics-service

RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)

FROM gcc:13-bookworm

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libpq5 \
    libgrpc++1.51 \
    libprotobuf32 \
    librabbitmq4 \
    libatomic1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/metrics-service/build/metrics-service .

ENV GRPC_PORT=50051
ENV HTTP_PORT=8080
ENV POSTGRES_HOST=postgres
ENV POSTGRES_DB=metrics_db
ENV POSTGRES_USER=metrics_user
ENV POSTGRES_PASSWORD=metrics_password
ENV RABBITMQ_HOST=rabbitmq
ENV RABBITMQ_PORT=5672
ENV RABBITMQ_USER=guest
ENV RABBITMQ_PASSWORD=guest
ENV RABBITMQ_QUEUE=metrics_events

EXPOSE 50051
EXPOSE 8080

CMD ["./metrics-service"]
