"""
Simulates a sequence of IoT alarm events by sending POST requests to the webserver.
Run with: python test_events.py [--host HOST] [--port PORT]
"""

import argparse
import time
import urllib.request
import urllib.error
import json

EVENTS = [
    ("alarm_on",          "Alarm armed"),
    ("motion_detected",   "Motion detected"),
    ("alarm_triggered",   "Alarm triggered"),
    ("alarm_off",         "Alarm disarmed"),
    ("alarm_on",          "Alarm armed again"),
    ("alarm_off",         "Alarm disarmed again"),
]


def post_event(url, event_type):
    body = json.dumps({"type": event_type}).encode()
    req = urllib.request.Request(
        url,
        data=body,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            return resp.status
    except urllib.error.HTTPError as e:
        return e.code
    except urllib.error.URLError as e:
        print(f"  Connection error: {e.reason}")
        return None


def main():
    parser = argparse.ArgumentParser(description="Simulate alarm events")
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", default=5000, type=int)
    parser.add_argument("--delay", default=0.5, type=float, help="Seconds between events")
    args = parser.parse_args()

    url = f"http://{args.host}:{args.port}/event"
    print(f"Sending events to {url}\n")

    for event_type, label in EVENTS:
        status = post_event(url, event_type)
        marker = "OK" if status == 200 else f"FAILED ({status})"
        print(f"  [{marker}] {label} -> {event_type}")
        time.sleep(args.delay)

    print(f"\nDone. View log at http://{args.host}:{args.port}/")


if __name__ == "__main__":
    main()
