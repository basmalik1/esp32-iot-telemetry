#!/usr/bin/env python3
"""System tests against a running board (TC-3.1, TC-3.2, TC-3.3).

These need a real socket and a real device, so they live outside the Unity
suites. Standard library only, apart from pyserial for the contention test.

    python tools/system_test.py --host 10.0.0.101
    python tools/system_test.py --host 10.0.0.101 --serial COM3 --contention 20

Exits non-zero if any check fails.
"""

import argparse
import json
import sys
import time
import urllib.error
import urllib.request

TIMEOUT = 5.0

passed = 0
failed = 0


def check(label, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print("  PASS  %s" % label)
    else:
        failed += 1
        print("  FAIL  %s %s" % (label, detail))


def get(host, path):
    """GET a path, returning (status_code, parsed_json_or_text)."""
    url = "http://%s%s" % (host, path)
    try:
        with urllib.request.urlopen(url, timeout=TIMEOUT) as r:
            body = r.read().decode()
            try:
                return r.status, json.loads(body)
            except json.JSONDecodeError:
                return r.status, body
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode()


def tc_3_1_endpoints_are_valid(host):
    """Every endpoint answers, and the payload carries the documented fields."""
    print("\nTC-3.1  endpoints valid and return expected outputs")

    code, body = get(host, "/status")
    check("/status returns 200", code == 200, "(got %s)" % code)
    check("/status returns JSON", isinstance(body, dict), "(got %r)" % (body,))

    if isinstance(body, dict):
        for field in ("led", "toggles", "rssi", "uptime_ms"):
            check("/status has %s" % field, field in body)
        check("led is a boolean", isinstance(body.get("led"), bool))
        check("toggles is an integer", isinstance(body.get("toggles"), int))

    code, _ = get(host, "/")
    check("/ returns 200", code == 200, "(got %s)" % code)

    code, _ = get(host, "/no-such-endpoint")
    check("unknown path returns 404", code == 404, "(got %s)" % code)


def tc_3_2_http_reaches_the_led(host):
    """A request must actually change device state, not just return 200."""
    print("\nTC-3.2  HTTP requests reach the LED functions")

    _, on = get(host, "/on")
    check("/on reports led true", on.get("led") is True, "(got %r)" % (on,))

    _, off = get(host, "/off")
    check("/off reports led false", off.get("led") is False, "(got %r)" % (off,))

    _, first = get(host, "/toggle")
    _, second = get(host, "/toggle")
    check("/toggle alternates", first.get("led") != second.get("led"),
          "(%r then %r)" % (first.get("led"), second.get("led")))

    # Mutating endpoints must report the resulting state, so a caller never
    # needs a follow-up request.
    _, status = get(host, "/status")
    check("/status agrees with last toggle",
          status.get("led") == second.get("led"))


def toggles_now(host):
    _, body = get(host, "/status")
    return body["toggles"]


def http_only_contention(host, count):
    """No request may be lost even with no button involved."""
    print("\nTC-3.3a  %d rapid HTTP toggles, none dropped" % count)

    before = toggles_now(host)
    ok = 0
    for _ in range(count):
        code, _ = get(host, "/toggle")
        if code == 200:
            ok += 1
    after = toggles_now(host)

    check("all requests returned 200", ok == count, "(%d/%d)" % (ok, count))
    check("counter advanced by exactly %d" % count, after - before == count,
          "(advanced %d)" % (after - before))


def tc_3_3_contention(host, port, seconds):
    """Both inputs at once: nothing may be forgotten.

    Needs a human pressing the button while this runs. Button presses are
    counted from the board's own serial log, HTTP toggles from the responses,
    and the two must account for the counter exactly.
    """
    try:
        import serial
    except ImportError:
        print("\nTC-3.3  SKIPPED - pyserial not installed")
        return

    print("\nTC-3.3  button and HTTP contending for %d seconds" % seconds)
    print("  >>> PRESS THE BUTTON REPEATEDLY NOW <<<")

    lines = []
    stop = False

    def reader(s):
        buf = b""
        while not stop:
            chunk = s.read(256)
            if chunk:
                buf += chunk
                while b"\n" in buf:
                    line, buf = buf.split(b"\n", 1)
                    lines.append(line.decode("utf-8", "replace"))

    import threading

    s = serial.Serial()
    s.port = port
    s.baudrate = 115200
    s.timeout = 0.2
    s.dtr = False  # do not reset the board on open
    s.rts = False
    s.open()
    threading.Thread(target=reader, args=(s,), daemon=True).start()
    time.sleep(0.2)

    before = toggles_now(host)
    http_ok = 0
    deadline = time.time() + seconds
    while time.time() < deadline:
        code, _ = get(host, "/toggle")
        if code == 200:
            http_ok += 1
        time.sleep(0.08)
    after = toggles_now(host)

    time.sleep(0.3)
    stop = True
    time.sleep(0.4)
    s.close()

    presses = sum(1 for l in lines if "button: led now" in l)
    delta = after - before

    print("  HTTP toggles accepted:    %d" % http_ok)
    print("  button presses on serial: %d" % presses)
    print("  expected total:           %d" % (http_ok + presses))
    print("  actual counter delta:     %d" % delta)

    check("no request was forgotten", delta == http_ok + presses,
          "(off by %d)" % (delta - http_ok - presses))
    if presses == 0:
        print("  NOTE: no button presses seen - the contention half did not "
              "actually run")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--host", required=True, help="board IP or hostname")
    p.add_argument("--serial", help="serial port for TC-3.3, e.g. COM3")
    p.add_argument("--contention", type=int, default=20,
                   help="seconds to run the contention test")
    p.add_argument("--toggles", type=int, default=100,
                   help="requests for the HTTP-only drop test")
    args = p.parse_args()

    print("system tests against %s" % args.host)
    try:
        get(args.host, "/status")
    except Exception as e:
        print("cannot reach the board: %s" % e)
        return 2

    tc_3_1_endpoints_are_valid(args.host)
    tc_3_2_http_reaches_the_led(args.host)
    http_only_contention(args.host, args.toggles)
    if args.serial:
        tc_3_3_contention(args.host, args.serial, args.contention)
    else:
        print("\nTC-3.3  SKIPPED - pass --serial to run the contention test")

    print("\n%d passed, %d failed" % (passed, failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
