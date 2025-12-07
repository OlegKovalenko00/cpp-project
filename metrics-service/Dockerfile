FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# gRPC/protobuf системные + минимум для сборки libpqxx через FetchContent
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    git \
    pkg-config \
    libssl-dev \
    libpq-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем proto файл
COPY proto/metrics.proto /app/proto/

# Копируем исходники metrics-service
COPY metrics-service/ /app/

# Генерируем C++ код из proto с правильной версией protobuf
RUN protoc --proto_path=/app/proto \
           --cpp_out=/app/include \
           --grpc_out=/app/include \
           --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
           /app/proto/metrics.proto && \
    mv /app/include/*.cc /app/src/

# Сборка (FetchContent скачает только libpqxx)
RUN rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

ENV GRPC_PORT=50051
ENV POSTGRES_HOST=postgres
ENV POSTGRES_DB=metrics_db
ENV POSTGRES_USER=metrics_user
ENV POSTGRES_PASSWORD=metrics_password

EXPOSE 50051

CMD ["./build/metrics-service"]
