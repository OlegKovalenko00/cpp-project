FROM gcc:13 AS builder

WORKDIR /app

RUN apt-get update && apt-get install -y \
    cmake \
    git \
    librabbitmq-dev \
    && rm -rf /var/lib/apt/lists/*

# Копируем файлы api-service
COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/

# Собираем проект - httplib и json скачиваются через CMake FetchContent
RUN mkdir -p build && cd build && \
    cmake .. && make -j$(nproc)

FROM gcc:13-bookworm

WORKDIR /app

RUN apt-get update && apt-get install -y \
    librabbitmq4 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/api-service .

EXPOSE 8080

CMD ["./api-service"]
