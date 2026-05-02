# Time Module Reference

Documentation automatically generated from `time.prm`.

## `now`
**Signature:** `func int now()`  

Returns current Unix timestamp in seconds

---

## `sleep`
**Signature:** `func void sleep(int ms)`  

Pauses execution for X milliseconds

---

## `clock`
**Signature:** `func double clock()`  

High-resolution steady clock for profiling (returns double seconds)

---

## `format_time`
**Signature:** `func str format_time(int timestamp, str format_string)`  

format_time(timestamp, format_string) -> str

---

## `format_iso8601`
**Signature:** `func str format_iso8601()`  

iso8601() -> str (current time in YYYY-MM-DDTHH:MM:SSZ)

---

## `ticks`
**Signature:** `func double ticks()`  

ticks() -> double (High-resolution monotonic clock for start/end measurements)

---
