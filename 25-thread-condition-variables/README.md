# Producer/Consumer with Threads, Mutex, and Condition Variables

## Overview
This project provides a small demonstration of the classic **producer/consumer pattern** implemented via POSIX threads in Linux:
* **Producer Thread (`thread_push`):** Inserts sequential integers into a shared circular buffer.
* **Consumer Thread (`thread_pop`):** Removes integers from the same circular buffer.
* **Synchronization:** Thread orchestration is managed safely using a POSIX **mutex** (`mutex_buffer`) alongside a **condition variable** (`condition_buffer`).

This is a learning project for system programming under Linux, based on the work of Christophe Blaess (*Programmation système sous Linux*).

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler:** GCC
* **Libraries:** `pthread` (typically bundled standard within `glibc`)
* **Tools:** Make utility

---

## Compilation
To compile the application, execute:
```bash
make
```
This builds the `prod_cons` binary. To wipe the executable and clean up intermediate object files, run:
```bash
make clean
```

---

## Usage
Start the program by running:
```bash
./prod_cons
```
*Behavioral Note:* The program runs until the global variable `val` hits `5` within the producer context. After this limit, the producer ceases data insertions, but due to internal synchronization traps, the execution pipeline enters an endless loop (see **Known Issues & Bugs** below). 

*To forcefully kill the program, press **`Ctrl+C`**.*

---

## Project Structure

| File | Role |
| :--- | :--- |
| **`lib.h`** | Common project definitions and standard headers |
| **`prod_cons.c`** | Core thread management and circular buffer logic |
| **`Makefile`** | Compilation scripts and clean directives |

### Functional Blueprint
* **`thread_push()`**: Producer loop. Locks the shared mutex, evaluates if the buffer is saturated (`count == SIZE`), sleeps for a randomized period, inserts a new payload integer, and signals the consumer.
* **`thread_pop()`**: Consumer loop. Locks the shared mutex, evaluates if the buffer is empty (`count == 0`), pulls down a payload integer, and signals the producer.
* **`aleatoire()`**: Mathematical helper returning a random integer within a custom bound: `[0, maximum)`.

---

## Known Issues & Bugs in this Version

### 1. Counter Overwrite in `thread_pop()`
* **Problem:** The expression `count = BUFFER[tail_buffer];` accidentally replaces the tracking counter with the data value stored inside the circular queue buffer. This completely corrupts the thread lifecycle state and breaks all downstream tracking criteria (`count == 0` / `count == SIZE`).
* **Solution:** Assign the internal index value into a distinct localized storage block instead of overwriting the reference count:
  ```c
  int value = BUFFER[tail_buffer];
  tail_buffer = (tail_buffer + 1) % SIZE;
  count--;
  printf("Thread removed %d from the buffer\n", value);
  ```

### 2. Colliding `pthread_t` Storage Allocations
* **Problem:** `pthread_create(&threads, ...)` is called sequentially across both threads using the identical variable reference, meaning the tracking identification token for the initial worker thread is immediately overwritten and lost.
* **Solution:** Define distinct tracking handlers for each worker context:
  ```c
  pthread_t prod, cons;
  pthread_create(&prod, NULL, thread_push, NULL);
  pthread_create(&cons, NULL, thread_pop, NULL);
  ```

### 3. Immediate Exit Interruption via `pthread_exit(NULL)` in Main
* **Problem:** Invoking `pthread_exit()` from within `main()` kills the master process thread while permitting secondary threads to continue running blindly in the background. This prevents clean application destruction and skips program teardown blocks.
* **Solution:** Apply thread execution joins or tie lifecycle gates down smoothly to a tracking state flag before closing:
  ```c
  pthread_join(prod, NULL);
  pthread_join(cons, NULL);
  ```

### 4. `pthread_cond_wait()` Bound within Conditional Statements (`if`)
* **Problem:** Evaluating buffer constraints via `if (count == SIZE) pthread_cond_wait(...)` leaves the thread vulnerable to **spurious wakeups** allowed by POSIX specifications. A thread could wake up without the underlying validation state being true, causing execution corruption.
* **Solution:** Wrap the validation boundary within an explicit looping check:
  ```c
  while (count == SIZE)
      pthread_cond_wait(&condition_buffer, &mutex_buffer);
  ```

### 5. Multi-Purpose Collision on `val` States
* **Problem:** The `val` variable serves contradictory definitions—acting simultaneously as a data value, an upper tracking threshold (`val == SIZE`), and an incremental boundary.
* **Solution:** Split tracking duties away from payload streams by instantiating a localized producer tally variable (e.g., `nb_produced`).

### 6. Indefinite Spinning on Producer Termination
* **Problem:** Once `val` exceeds `5`, the producer stops performing insertions. However, since the state control flag `running` remains active, both threads enter a tight high-CPU spinning pattern, constantly locking variables and sleeping without executing payload work.
* **Solution:** Upon reaching completion limits, set `running = 0` and fire a global condition broadcast notification to safely drain and exit the consumer loops.

### 7. Missing Random Generator Initialization
* **Problem:** Using `rand()` without configuring an explicit seed means the program generates identical pseudo-random outputs across every independent run.
* **Solution:** Seed the random landscape inside the `main()` initialization block using: `srand(time(NULL));`.

### 8. Unchecked API Error Returns
* **Problem:** Return values for critical threading primitives (`pthread_create`, `pthread_mutex_lock`, `pthread_cond_wait`) are ignored, hiding potential failures.
* **Solution:** Wrap API declarations in evaluation blocks and display errors cleanly via `strerror()`.

### 9. Memory Cache Contention (Data Race on `running`)
* **Problem:** Multiple execution units access and evaluate the state variable `running` without acquiring the corresponding mutex boundary lock, meaning thread caches might not see state updates in real time.
* **Solution:** Enforce strict mutex lock protection surrounding any state adjustments, or implement dedicated compiler atomic types.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Derived from scheduling threads, safe critical zones, and event messaging constructs from *Programmation système sous Linux* by Christophe Blaess.
