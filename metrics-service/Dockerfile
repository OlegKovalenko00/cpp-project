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
    libevent-dev \
    && rm -rf /var/lib/apt/lists/*

COPY proto/metrics.proto /app/proto/
COPY metrics-service/CMakeLists.txt .
COPY metrics-service/include/ include/
COPY metrics-service/src/ src/

RUN protoc --proto_path=/app/proto \
           --experimental_allow_proto3_optional \
           --cpp_out=/app/include \
           --grpc_out=/app/include \
           --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
           /app/proto/metrics.proto && \
    mv /app/include/*.cc /app/src/

RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)

FROM gcc:13-bookworm

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libpq5 \
    libgrpc++1.51 \
    libprotobuf32 \
    libevent-2.1-7 \
    libatomic1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/metrics-service .

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