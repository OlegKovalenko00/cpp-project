import json
import sys
import time
import requests

BASE_URL = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8080"


def test_health_ping():
    """GET /health/ping"""
    print("Testing GET /health/ping...")
    resp = requests.get(f"{BASE_URL}/health/ping")
    assert resp.status_code == 200, f"Expected 200, got {resp.status_code}"
    data = resp.json()
    assert data["status"] == "ok", f"Expected status 'ok', got {data}"
    print("  ✅ PASSED\n")


def test_page_view_success():
    """POST /page-views - success"""
    print("Testing POST /page-views (success)...")
    payload = {
        "page": "/home",
        "user_id": "user_123",
        "session_id": "sess_456",
        "referrer": "https://google.com",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/page-views", json=payload)
    assert resp.status_code == 202, f"Expected 202, got {resp.status_code}"
    print("  ✅ PASSED\n")


def test_page_view_empty_page():
    """POST /page-views - empty page field"""
    print("Testing POST /page-views (empty page)...")
    payload = {
        "page": "",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/page-views", json=payload)
    assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"
    data = resp.json()
    assert data["code"] == "INVALID_PAGE_VIEW"
    print("  ✅ PASSED\n")


def test_click_success():
    """POST /clicks - success"""
    print("Testing POST /clicks (success)...")
    payload = {
        "page": "/pricing",
        "element_id": "cta-button",
        "action": "click",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/clicks", json=payload)
    assert resp.status_code == 202, f"Expected 202, got {resp.status_code}"
    print("  ✅ PASSED\n")


def test_click_empty_element_id():
    """POST /clicks - empty element_id"""
    print("Testing POST /clicks (empty element_id)...")
    payload = {
        "page": "/pricing",
        "element_id": "",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/clicks", json=payload)
    assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"
    data = resp.json()
    assert data["code"] == "INVALID_CLICK_EVENT"
    print("  ✅ PASSED\n")


def test_performance_success():
    """POST /performance - success"""
    print("Testing POST /performance (success)...")
    payload = {
        "page": "/dashboard",
        "ttfb_ms": 120,
        "fcp_ms": 450,
        "lcp_ms": 800,
        "total_page_load_ms": 1100,
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/performance", json=payload)
    assert resp.status_code == 202, f"Expected 202, got {resp.status_code}"
    print("  ✅ PASSED\n")


def test_performance_negative_timing():
    """POST /performance - negative timing value"""
    print("Testing POST /performance (negative timing)...")
    payload = {
        "page": "/dashboard",
        "ttfb_ms": -10,
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/performance", json=payload)
    assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"
    data = resp.json()
    assert data["code"] == "INVALID_PERFORMANCE_EVENT"
    print("  ✅ PASSED\n")


def test_error_event_success():
    """POST /errors - success"""
    print("Testing POST /errors (success)...")
    payload = {
        "page": "/dashboard",
        "error_type": "js_exception",
        "message": "Cannot read property 'id' of undefined",
        "stack": "TypeError: ...\n    at Dashboard.tsx:42",
        "severity": "error",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/errors", json=payload)
    assert resp.status_code == 202, f"Expected 202, got {resp.status_code}"
    print("  ✅ PASSED\n")


def test_error_event_empty_message():
    """POST /errors - empty message"""
    print("Testing POST /errors (empty message)...")
    payload = {
        "page": "/dashboard",
        "error_type": "js_exception",
        "message": "",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/errors", json=payload)
    assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"
    data = resp.json()
    assert data["code"] == "INVALID_ERROR_EVENT"
    print("  ✅ PASSED\n")


def test_custom_event_success():
    """POST /custom-events - success"""
    print("Testing POST /custom-events (success)...")
    payload = {
        "name": "signup_completed",
        "page": "/signup/success",
        "user_id": "user_123",
        "properties": {
            "plan": "pro",
            "source": "landing_a"
        },
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/custom-events", json=payload)
    assert resp.status_code == 202, f"Expected 202, got {resp.status_code}"
    print("  ✅ PASSED\n")


def test_custom_event_empty_name():
    """POST /custom-events - empty name"""
    print("Testing POST /custom-events (empty name)...")
    payload = {
        "name": "",
        "timestamp": int(time.time())
    }
    resp = requests.post(f"{BASE_URL}/custom-events", json=payload)
    assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"
    data = resp.json()
    assert data["code"] == "INVALID_CUSTOM_EVENT"
    print("  ✅ PASSED\n")


def test_invalid_json():
    """POST with invalid JSON"""
    print("Testing POST /page-views (invalid JSON)...")
    resp = requests.post(
        f"{BASE_URL}/page-views",
        data="not valid json",
        headers={"Content-Type": "application/json"}
    )
    assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"
    print("  ✅ PASSED\n")


def main():
    print(f"\n{'='*50}")
    print(f"Testing API at {BASE_URL}")
    print(f"{'='*50}\n")

    tests = [
        test_health_ping,
        test_page_view_success,
        test_page_view_empty_page,
        test_click_success,
        test_click_empty_element_id,
        test_performance_success,
        test_performance_negative_timing,
        test_error_event_success,
        test_error_event_empty_message,
        test_custom_event_success,
        test_custom_event_empty_name,
        test_invalid_json,
    ]

    passed = 0
    failed = 0

    for test in tests:
        try:
            test()
            passed += 1
        except AssertionError as e:
            print(f"  ❌ FAILED: {e}\n")
            failed += 1
        except requests.exceptions.ConnectionError:
            print(f"  ❌ FAILED: Could not connect to {BASE_URL}\n")
            failed += 1

    print(f"{'='*50}")
    print(f"Results: {passed} passed, {failed} failed")
    print(f"{'='*50}\n")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
