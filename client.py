import argparse
import copy
import json
import random
import signal
import threading
import time
import uuid

import yaml
import requests
from faker import Faker

STOP = False
fake = Faker()
RPC_ID = 0
RPC_ID_LOCK = threading.Lock()


def signal_handler(sig, frame):
    global STOP
    STOP = True
    print("stopping...")


signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)


def load_openapi(path):
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def collect_post_examples(openapi):
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
    if not isinstance(e, dict):
        return e
    if "user_id" in e:
        e["user_id"] = random.choice([f"user_{random.randint(1,9999)}", fake.user_name()])
    if "session_id" in e:
        e["session_id"] = f"sess-{uuid.uuid4().hex[:8]}"
    if "timestamp" in e:
        e["timestamp"] = int(time.time()) + random.randint(-60, 60)
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


def event_worker(api_base, examples, rate, name):
    session = requests.Session()
    interval = 1.0 / rate if rate > 0 else 1.0
    while not STOP:
        path, example = random.choice(examples)
        payload = randomize_example(example)
        url = api_base.rstrip("/") + path
        try:
            r = session.post(url, json=payload, timeout=5)
            print(f"[{name}] POST {path} -> {r.status_code}")
        except Exception as ex:
            print(f"[{name}] ERROR POST {path} -> {ex}")
        if STOP:
            break
        time.sleep(interval)
    print(f"[{name}] exit")


def next_rpc_id():
    global RPC_ID
    with RPC_ID_LOCK:
        RPC_ID += 1
        return RPC_ID


def rpc_worker(agg_base, methods, rate, name):
    session = requests.Session()
    url = agg_base.rstrip("/") + "/jsonrpc"
    interval = 1.0 / rate if rate > 0 else 1.0
    while not STOP:
        method = random.choice(methods)
        params = {}
        if method.lower().find("page") >= 0 or method.lower().find("click") >= 0:
            params = {"user_id": random.choice([None, f"user_{random.randint(1,500)}"]), "limit": random.choice([5, 10, 20])}
        else:
            params = {"limit": random.choice([5, 10, 20])}
        payload = {"jsonrpc": "2.0", "method": method, "params": params, "id": next_rpc_id()}
        try:
            r = session.post(url, json=payload, timeout=5)
            status = r.status_code
            body_preview = r.text[:200].replace("\n", " ")
            print(f"[{name}] RPC {method} -> {status} : {body_preview}")
        except Exception as ex:
            print(f"[{name}] ERROR RPC {method} -> {ex}")
        if STOP:
            break
        time.sleep(interval)
    print(f"[{name}] exit")


def main():
    parser = argparse.ArgumentParser(description="Event + aggregation client")
    parser.add_argument("--openapi", "-o", default="api-service/openapi.yaml", help="OpenAPI yaml path")
    parser.add_argument("--api-base", "-a", default="http://localhost:8080", help="api-service base URL")
    parser.add_argument("--agg-base", "-g", default="http://localhost:8081", help="aggregation-service base URL (JSON-RPC endpoint at /jsonrpc)")
    parser.add_argument("--rate", "-r", type=float, default=1.0, help="events per second per worker")
    parser.add_argument("--workers", "-w", type=int, default=1, help="number of event workers")
    parser.add_argument("--rpc-rate", type=float, default=0.5, help="rpc calls per second per rpc-worker")
    parser.add_argument("--rpc-workers", type=int, default=1, help="number of rpc workers")
    args = parser.parse_args()

    openapi = load_openapi(args.openapi)
    examples = collect_post_examples(openapi)
    if not examples:
        print("no POST examples found in OpenAPI")
        return

    rpc_methods = [
        "GetPageViews", "GetClicks", "GetPerformance", "GetErrors", "GetCustomEvents"
    ]

    print(f"starting: {args.workers} event workers @ {args.rate} rps each, {args.rpc_workers} rpc workers @ {args.rpc_rate} rps each")

    threads = []
    for i in range(args.workers):
        t = threading.Thread(target=event_worker, args=(args.api_base, examples, args.rate, f"E{i+1}"), daemon=True)
        t.start()
        threads.append(t)

    for i in range(args.rpc_workers):
        t = threading.Thread(target=rpc_worker, args=(args.agg_base, rpc_methods, args.rpc_rate, f"R{i+1}"), daemon=True)
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
