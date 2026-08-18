# Testing

Three tiers, split by what each can actually prove.

```
   /\        system     endpoints and contention, against a live board
  /  \       on-target  peripherals, real timing
 /____\      host       pure logic
```

## Host tier — `pio test -e native`

Runs on your machine with gcc. No board, no network, seconds to complete.

```sh
pio test -e native
```

Covers the logic that has been deliberately kept free of Arduino dependencies:

| Test | Covers |
| --- | --- |
| `test_native_debounce` | TC-2.1 — the debounce decision, including the `millis()` rollover |
| `test_native_status` | TC-3.1 — the JSON payload format and buffer sizing |

The rollover case is the argument for this tier existing. `millis()` wraps after ~49.7 days, so reaching that boundary on real hardware would mean running the board for seven weeks; as a pure function it is one assertion.

Only the Arduino-free sources are compiled here — `status_json.cpp`, plus `debounce.h` which is header-only. Anything that includes `Arduino.h` is excluded by `build_src_filter`, which is what keeps this tier buildable on a machine with no embedded toolchain at all.

## On-target tier — `pio test -e target`

PlatformIO builds a test firmware, flashes it, and reads Unity's results back over serial. Needs the board connected.

```sh
pio test -e target
```

| Test | Covers |
| --- | --- |
| `test_embedded_led` | TC-1.1 — on, off and toggle through the real `digitalWrite`/`neopixelWrite` |
| `test_embedded_button` | TC-2.1 hardware half — no phantom presses, pin idles high, counter clears |

Budget about 25 seconds per test file: each one is a full build, flash and run cycle. The first run after changing environments recompiles the Arduino framework and takes considerably longer — it is not stuck.

`main.cpp` is excluded from these builds because its `setup()`/`loop()` would collide with the test runner's.

## System tier — `tools/system_test.py`

Needs a board that is running the normal firmware and joined to the network.

```sh
python tools/system_test.py --host 10.0.0.101
```

Covers TC-3.1 (every endpoint answers with the documented shape), TC-3.2 (requests actually change device state rather than merely returning 200), and an HTTP-only drop check — 100 rapid toggles must advance the counter by exactly 100.

The full **TC-3.3** contention test needs a human pressing the button while HTTP traffic runs, so it is opt-in:

```sh
python tools/system_test.py --host <board-ip> --serial <port> --contention 20
```

It counts button presses from the board's serial log and HTTP toggles from the responses, then asserts that the two account for the counter exactly:

```
HTTP toggles accepted:    200
button presses on serial:  77
expected total:           277
actual counter delta:     277
```

That arithmetic is the whole point — it turns "seems to work" into a number. The board's `toggles` field on `/status` exists to make it possible.

## What is not covered

Route dispatch is only exercised by the system tier, so it needs a live board. Testing it on the host would mean faking `WebServer`, which is more machinery than the coverage is worth.

TC-1.2 (current draw on GPIO4) and TC-1.3 (50 power cycles) remain manual and unrun — see [process.md](process.md#known-gaps).
