# Threads with Mutex – Shared Counter

## Overview
This project provides a straightforward demonstration of **POSIX threads** and mutual exclusion primitives under Linux. It illustrates how to protect a shared resource from execution race conditions using a synchronization barrier:
* Multiple concurrent worker threads (`NB_THREADS`, default is 2) run an identical routine loop.
* Each independent thread increments a global counter variable (`compteur`) until it reaches a targeted limit of `40`.
* The critical processing section is strictly guarded by a globally defined POSIX **mutex** (`PTHREAD_MUTEX_INITIALIZER`).
* Unique tracking indices are passed into each thread using safe `void *` pointers cast via `intptr_t`.

This is a learning project for system programming under Linux, based on the work of Christophe Blaess (*Programmation système sous Linux*).

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler:** GCC
* **Libraries:** `pthread` (integrated natively inside `glibc`)
* **Tools:** Make utility

---

## Compilation
To build the executable file, run:
```bash
make
```
This produces the `mutex_threads` binary. To clean up object code files and delete the binary, run:
```bash
make clean
```

---

## Usage
Run the compiled tracking program from your console:
```bash
./mutex_threads
```

Each active thread will stream diagnostic traces to `stdout` upon successfully acquiring and releasing the synchronization mutex:
```text
Thread 0 of Ludovic acquired the mutex
Thread 0 : compteur = 1
Thread 0 of Ludovic released the mutex
Thread 1 of Ludovic acquired the mutex
Thread 1 : compteur = 2
Thread 1 of Ludovic released the mutex
...
```
The routine automatically halts and completes as soon as `compteur` hits `40`.

---

## Project Structure

| File | Role |
| :--- | :--- |
| **`lib.h`** | Global project header (includes `<pthread.h>`, `<stdint.h>`, etc.) |
| **`mutex_threads.c`** | Core thread management and shared state implementation |
| **`Makefile`** | Code compilation blueprints and workspace cleanup rules |

### Code Blueprint
* **`fn_thread()`**: Routine executed by each spawned thread. It evaluates the loop, claims the mutex lock, reads and alters the critical counter state, and releases control.
* **`aleatoire()`**: Random generation utility returning an integer value bounded by `[0, maximum)`.
* **`main()`**: Instantiates thread arrays, handles standard numeric-to-pointer index conversions (`(void *)(intptr_t)i`), and joins execution threads safely on termination.

---

## Known Issues & Pitfalls

### 1. Header Constraints with `intptr_t`
* **Problem:** Utilizing the pointer-sized integer casting mechanism `intptr_t` requires specific declaration environments. Missing dependencies throw explicit compilation failures.
* **Solution:** Ensured that `<stdint.h>` is fully included within the standard `lib.h` definition sheet.

### 2. Strict Signature Matching for Thread Runners
* **Problem:** Thread callbacks must exactly match the POSIX standard signature: `void *fn_thread(void *arg)`. Declaring a blank signature list (`void *fn_thread()`) represents "unspecified arguments" in standard C, causing interface collisions with `pthread_create`.
* **Solution:** Explicitly enforce a formal `void *` input parameter field inside both the signature definition and function implementation.

### 3. Out-of-Bounds Evaluations outside Mutex Zones
* **Problem:** Evaluating loop conditions via `while (compteur < 40)` outside a critical lock zone creates check-then-act vulnerabilities. Two threads could read `compteur == 39` simultaneously, enter the loop together, and push the counter past the threshold (e.g., to `41`).
* **Solution:** Reframe processing conditions to guarantee state evaluation takes place strictly while holding the lock:
  ```c
  while (1) {
      sleep(aleatoire(3));
      pthread_mutex_lock(&mutex);
      if (compteur >= 40) { 
          pthread_mutex_unlock(&mutex); 
          break; 
      }
      compteur++;
      ...
      pthread_mutex_unlock(&mutex);
  }
  ```

### 4. Memory Visibility Glitches (Data Race on Counter)
* **Problem:** Accessing `compteur` within an unshielded `while` expression while other units are modifying it constitutes a classic data race (yielding undefined behavior under modern C11+ rules).
* **Solution:** Completely enclose every counter read and write statement within structural mutex brackets.

### 5. Invalid Error Trace Processing with `perror` on Joins
* **Problem:** The `pthread_join()` interface passes error flags directly back through its functional return value instead of writing to the system-wide global `errno` variable. Calling `perror()` on failure reads random garbage values from unrelated events.
* **Solution:** Evaluate function output explicitly and leverage thread-safe converters like `strerror()` to decode the diagnostic code:
  ```c
  if ((ret = pthread_join(threads[i], NULL)) != 0) {
      fprintf(stderr, "%s\n", strerror(ret));
      exit(EXIT_FAILURE);
  }
  ```

### 6. Predictable Random Iterations
* **Problem:** Relying on `rand()` functions without declaring a dynamic initialization seed forces identical sequence loops across separate runtime invocations.
* **Solution:** Inject a seed modifier like `srand(time(NULL));` at the definitive start of the `main()` function execution path.

### 7. Thread Starvation with Zero-Delay Sleeps
* **Problem:** If `aleatoire(3)` returns `0`, calling `sleep(0)` drops out instantly. This is valid code but can cause a single thread to consume the lock repeatedly, keeping other threads from running.
* **Solution:** Offset random intervals to guarantee a minimum rest step if necessary: `1 + aleatoire(3)`.

### 8. Absent Explicit Pointer Return Statements
* **Problem:** Thread tracking functions expect a `void *` return value. Omitting an explicit value when terminating can trigger standard warnings across picky compilation layouts.
* **Solution:** Utilizing `pthread_exit(NULL);` handles this safely. Alternatively, append an explicit `return NULL;` at the absolute foot of the execution branch.

### 9. Incomplete Clean-up of Orphan Units during Startup Failures
* **Problem:** If a initialization loop fails midway through `pthread_create()`, calling an abrupt global `exit()` kills the process without waiting for already active background threads to cleanly terminate.
* **Solution:** While a plain process exit is acceptable for simple labs, robust code should cleanly unravel and join previously instantiated threads before dropping offline.

### 10. Misconceptions regarding `static` Variable Security
* **Problem:** Marking global components as `static` limits their visibility to the current translation unit, but it does **not** provide any thread isolation or memory concurrency security.
* **Solution:** Keep in mind that thread synchronization is driven solely by the mutex framework, independently of memory linkage types.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Inspired by the multi-threaded synchronization, mutual exclusion domains, and thread tracking structures outlined in *Programmation système sous Linux* by Christophe Blaess.
