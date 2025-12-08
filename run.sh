cd api-service
docker compose up --build -d

cd ..
cd aggregation-service
docker compose up --build -d

cd ..
cd metrics-service
docker compose up --build -d

cd ..
cd monitoring-service
docker compose up --build -d