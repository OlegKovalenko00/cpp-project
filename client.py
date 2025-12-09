
import argparse
import random
import signal
import sys
import time
import threading
import uuid
import copy
import json

import yaml
import requests
from faker import Faker

STOP = False
fake = Faker()


def signal_handler(sig, frame):
    global STOP
    STOP = True
    print("Stopping...")

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)


def load_openapi(path):
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def collect_examples(openapi):
    examples = []
    paths = openapi.get("paths", {})
    for path, methods in paths.items():
        for method, meta in methods.items():
            if method.lower() != "post":
                continue
            content = meta.get("requestBody", {}).get("content", {})
            app_json = content.get("application/json", {})
            example = app_json.get("example")
            if example:
                examples.append((path, example))
    return examples


def randomize_example(example):
    e = copy.deepcopy(example)
    # common fields we expect from schema in the provided OpenAPI
    # user_id, session_id, timestamp, page, element_id, name, properties, referrer, severity
    if isinstance(e, dict):
        if "user_id" in e:
            e["user_id"] = random.choice([f"user_{random.randint(1,9999)}", fake.user_name()])
        if "session_id" in e:
            e["session_id"] = f"sess-{uuid.uuid4().hex[:8]}"
        if "timestamp" in e:
            now = int(time.time())
            e["timestamp"] = now + random.randint(-300, 300)
        if "page" in e:
            e["page"] = random.choice(["/home", "/products", "/dashboard", "/pricing", "/signup"])
        if "element_id" in e:
            e["element_id"] = random.choice(["btn-signup", "cta-subscribe-button", "product-1"])
        if "referrer" in e:
            e["referrer"] = random.choice([None, "https://google.com", "https://bing.com", "/home"])
        if "properties" in e and isinstance(e["properties"], dict):
            e["properties"]["request_id"] = uuid.uuid4().hex
        if "severity" in e:
            e["severity"] = random.choice(["warning", "error", "critical"])
        if "ttfb_ms" in e:
            e["ttfb_ms"] = max(0, int(random.gauss(120, 30)))
        if "fcp_ms" in e:
            e["fcp_ms"] = max(0, int(random.gauss(400, 100)))
        if "lcp_ms" in e:
            e["lcp_ms"] = max(0, int(random.gauss(900, 200)))
    return e


def worker_thread(base_url, examples, rate, name):
    session = requests.Session()
    interval = 1.0 / rate if rate > 0 else 1.0
    while not STOP:
        path, example = random.choice(examples)
        payload = randomize_example(example)
        url = base_url.rstrip("/") + path
        try:
            resp = session.post(url, json=payload, timeout=5)
            print(f"[{name}] POST {path} -> {resp.status_code} ({resp.reason})")
        except Exception as ex:
            print(f"[{name}] ERROR POST {path} -> {ex}")
        # wait
        if STOP:
            break
        time.sleep(interval)
    print(f"[{name}] exiting")


def main():
    parser = argparse.ArgumentParser(description="OpenAPI-based infinite request generator")
    parser.add_argument("--openapi", "-o", default="openapi.yaml", help="OpenAPI YAML file")
    parser.add_argument("--base-url", "-u", default="http://localhost:8080", help="Server base URL")
    parser.add_argument("--rate", "-r", type=float, default=1.0, help="requests per second per worker")
    parser.add_argument("--workers", "-w", type=int, default=1, help="number of concurrent workers")
    args = parser.parse_args()

    openapi = load_openapi(args.openapi)
    examples = collect_examples(openapi)
    if not examples:
        print("No POST examples found in OpenAPI file.")
        sys.exit(1)
        print(f"Found {len(examples)} POST endpoints with examples. Starting {args.workers} workers at {args.rate} rps each.")
    threads = []
    for i in range(args.workers):
        t = threading.Thread(target=worker_thread, args=(args.base_url, examples, args.rate, f"W{i+1}"), daemon=True)
        t.start()
        threads.append(t)

    try:
        while not STOP:
            time.sleep(0.5)
    except KeyboardInterrupt:
        pass

    for t in threads:
        t.join()


if __name__ == "__main__":
    main()