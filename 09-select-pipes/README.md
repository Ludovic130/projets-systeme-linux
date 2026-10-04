# Select on Multiple Pipes (Parent/Children)

## Overview
This program provides a practical demonstration of **synchronous I/O multiplexing** using `select()` to monitor multiple communication streams simultaneously. 
* **Worker Forks:** The master parent process instantiates `NB_FILS` (default is 10) independent unidirectional communication channels (`pipe`) and forks an isolated worker child process for each channel.
* **Staggered Transmission:** Each child loops infinitely, streaming a single byte payload down its assigned pipe at a staggered, index-dependent interval of `(i + 1)` seconds.
* **Multiplexed Monitoring:** Rather than blocking sequentially on individual pipes, the parent process leverages `select()` to sleep until *any* descriptor transitions to a readable state. It then captures the event and prints the integer index of the signaling child.

The console output produces a progressive cascade pattern (e.g., `0 1 0 2 0 1 3...`), cleanly proving how `select()` resolves competitive network/IPC events concurrently.

This is a learning project for system programming under Linux, based on the work of Christophe Blaess (*Programmation système sous Linux*).

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler/Automation:** GCC compiler, Make utility

---

## Compilation
To compile the multiplexing utility, execute:
```bash
make
```
This builds the `select_pipes` executable. To remove the binary and clean up object files, run:
```bash
make clean
```

---

## Usage
Launch the multiplexed stream tracker from your terminal window:
```bash
./select_pipes
```
*Behavioral Note: The application forks children that loop indefinitely. Press **`Ctrl+C`** at any point to forcefully kill the parent process and stop the demonstration.*

---

## Project Structure

| File | Role |
| :--- | :--- |
| **`lib.h`** | Shared header file (includes `<sys/select.h>`, `<unistd.h>`, macros, etc.) |
| **`select_pipes.c`** | Main logic containing parent multiplexer and child pipeline loops |
| **`Makefile`** | Compilation scripts and clean directives |

---

## Known Issues & Pitfalls

### 1. Assignment Typo within Error Isolation Checks
* **Problem:** The commented-out fallback helper function `attente_reception()` contains a critical logical defect where the condition check is written as `(errno = EINTR)`. Because it uses a single equals sign (`=`), it assigns `EINTR` to `errno` and always evaluates to true, breaking signal recovery logic.
* **Solution:** Replace the assignment character with a strict equality assertion evaluator:
  ```c
  if (errno == EINTR) { /* Gracefully handle signal interrupt and resume */ }
  ```

### 2. Sub-optimal Descriptor Boundaries inside `select()`
* **Problem:** The application passes the global system constant `FD_SETSIZE` (typically 1024) as the initial boundary size argument (`nfds`) to `select()`. While this functions safely because our dynamic pipe handles stay well below 1024, it forces the Linux kernel to blindly scan hundreds of unused, unallocated bits on every loop iteration, degrading efficiency.
* **Solution:** Track the highest numerical file descriptor allocated during process initialization and supply a targeted, high-performance runtime ceiling:
  ```c
  int max_fd = -1;
  // During pipe allocation tracking:
  if (paires_pipes[i] > max_fd) {
      max_fd = paires_pipes[i];
  }
  // When calling select:
  select(max_fd + 1, &lecture_set, NULL, NULL, NULL);
  ```

### 3. Missing `SIGCHLD` Interception (Zombie Generation)
* **Problem:** When the master application process receives an external interruption event (like `Ctrl+C`) or completes its flow, it drops offline without verifying child status. Because there is no asynchronous handler tracking `SIGCHLD`, detached workers turn into system zombie processes until reaped by `init`/`systemd`.
* **Solution:** Register an explicit process reaping signal loop at startup, or explicitly loop through the known child pid array to forcefully issue `kill(child_pids[i], SIGTERM)` shutdowns during master exit sequences.

### 4. Unbounded Resource Consumption
* **Problem:** The worker loops are missing conditional loop breakers or structural lifetime indicators. Left unmanaged, they run indefinitely and can consume continuous operational headroom.
* **Solution:** Implement an external termination flag or state boundaries to cleanly close descriptor endpoints and trigger safe thread exits after a specific number of successful cycles.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Derived from I/O multiplexing routines, multi-stream pipelines, and concurrent system fork boundaries outlined in *Programmation système sous Linux* by Christophe Blaess.
