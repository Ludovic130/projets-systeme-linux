# Signal Handling with sigaction and sigqueue

## Overview
This project consists of two small programs that demonstrate the use of POSIX real-time signals in Linux:

* **`signals` (Receiver):** Installs a handler for almost every signal using `sigaction()` with the `SA_SIGINFO` flag. It prints the signal number and its corresponding `si_code` when a signal arrives.
* **`sigqueue_demo` (Sender):** Sends a signal with an optional payload (`union sigval`) to one or more process IDs (PIDs) using `sigqueue()`.

Together, these programs demonstrate the practical differences between a plain `kill()` call (no payload, `si_code = SI_USER`) and a `sigqueue()` call (payload included, `si_code = SI_QUEUE`).

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Tools:** GCC compiler, Make utility

---

## Compilation
To compile both tools, run:
```bash
make
```
This produces the `signals` and `sigqueue_demo` executables. To remove them and clean up the directory, run:
```bash
make clean
```

---

## Usage

### 1. Terminal 1 – Start the Receiver
```bash
./signals
# Example output: PID=12345
```

### 2. Terminal 2 – Send Signals
```bash
./sigqueue_demo 10 12345       # Sends SIGUSR1 with payload to PID 12345
./sigqueue_demo 12345          # Sends SIGTERM (default) to PID 12345
kill -USR1 12345               # Plain kill command for comparison
```

### Expected Receiver Output
```text
Received 10
si_code = -1        # from sigqueue (SI_QUEUE)

Received 10
si_code = 0         # from kill (SI_USER)
```
*Note: `SIGKILL`, `SIGSTOP`, and signal `0` cannot be intercepted. The "not intercepted" messages for these signals are expected behavior.*

---

## Syntax
```bash
sigqueue_demo [signal] pid...
```
* If only PIDs are provided, **SIGTERM** is used by default.
* Otherwise, the first argument is interpreted as the **signal number**.

---

## Project Structure

| File | Role |
| :--- | :--- |
| **`lib.h`** | Common header (defines `_GNU_SOURCE`, includes `<signal.h>`, etc.) |
| **`signals.c`** | Receiver implementation |
| **`sigqueue_demo.c`** | Sender implementation |
| **`Makefile`** | Build automation script |

---

## Known Issues & Pitfalls

### Receiver
* **Feature Test Macros:** `_NSIG` requires `#define _GNU_SOURCE` to be declared before any includes.
* **Configuration:** You must use `sa_sigaction` and the `SA_SIGINFO` flag; otherwise, the signal payload is discarded.
* **Signal Safety:** Using `fprintf()` inside a signal handler is **not** async-signal-safe. Use `write()` in production code.
* **Execution Flow:** Do not resume execution after encountering fatal signals like `SIGSEGV`, `SIGFPE`, `SIGBUS`, or `SIGILL`.

### Sender
* **Input Parsing:** `sscanf("%d", ...)` silently accepts malformed strings like `"12abc"`. Use `strtol()` with `endptr` for strict validation.
* **Permissions:** `sigqueue()` can only signal processes sharing the same UID, unless running with `CAP_KILL` privileges.
* **Payload Constraints:** `SIGKILL` and `SIGSTOP` cannot carry a payload.
* **Error Handling:** Errors encountered by `sigqueue()` should be handled and properly reflected in the application's exit code.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Inspired by exercises on `sigaction`, `SA_SIGINFO`, and `sigqueue` from *Programmation système sous Linux* by Christophe Blaess.
