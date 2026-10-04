# Parallel Sum of an Array with Threads

## Overview
This program demonstrates data partitioning using POSIX threads under Linux. It splits a large single-dimensional integer array into `NB_THREADS` contiguous segments and computes the arithmetic sum of each segment concurrently in a separate thread.

### Key Concepts Demonstrated
* Data segmentation and distribution strategies across parallel threads.
* Passing multi-variable arguments to threads using localized struct pointers.
* Lifecycle management of workers via `pthread_create()`, `pthread_join()`, and `pthread_exit()`.
* Handling non-uniform workloads (distributing remainders when the array size is not perfectly divisible by the number of threads).

This is a learning project for system programming under Linux, based on the work of Christophe Blaess (*Programmation système sous Linux*).

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler:** GCC
* **Libraries:** `pthread` (bundled natively within standard `glibc`)
* **Tools:** Make utility

---

## Compilation
To compile the parallel summation tool, run:
```bash
make
```
This yields the `parallel_sum` executable. To remove the binary and intermediate object code, run:
```bash
make clean
```

---

## Usage
Execute the program from your console:
```bash
./parallel_sum
```

### Expected Output
Each thread prints its bounded index scope, along with its calculated subtotal:
```text
Thread 0 processes indices from 0 to 2:
the sum of thread 0 is 6

Thread 1 processes indices from 3 to 4:
the sum of thread 1 is 9

Thread 2 processes indices from 5 to 6:
the sum of thread 2 is 13

Thread 3 processes indices from 7 to 8:
the sum of thread 3 is 17

Thread 4 processes indices from 9 to 10:
the sum of thread 4 is 21
```

---

## Project Structure

| File | Role |
| :--- | :--- |
| **`lib.h`** | Global project header (includes `<pthread.h>`, array boundaries, etc.) |
| **`parallel_sum.c`** | Main logic, array segmentation, and thread runners |
| **`Makefile`** | Automation scripts for clean and release compilation |

### Code Architecture
* **`struct t_`**: Data structure tracking unique thread metadata, specifically `threads_id`, `debut` (start index), and `fin` (end index).
* **`tab()`**: Thread callback function. Computes the arithmetic subtotal of `table[debut..fin]` and prints the resulting sum.
* **`main()`**: 
  1. Computes baseline segment size (`nb_chacun`) and the remainder (`reste`).
  2. Evenly distributes the remaining indices across the initial threads.
  3. Spawns threads, joins them to aggregate state, and gracefully cleans up resources.

---

## Known Issues & Solutions

### 1. Critical Precedence Defect in `pthread_create` Call
* **Problem:** The original expression was written as:
  ```c
  if ((ret = pthread_create(&threads[i], NULL, tab, (void *)&inter[i]) != 0))
  ```
  Because the inequality operator (`!=`) takes structural precedence over assignment (`=`), the expression was evaluated as `ret = (pthread_create(...) != 0)`. Consequently, `ret` received a flag of `0` or `1` instead of tracking the actual return code, rendering error assertions broken.
* **Solution:** Adjust the parentheses formatting to accurately isolate the assignment boundary prior to the validation statement:
  ```c
  if ((ret = pthread_create(&threads[i], NULL, tab, (void *)&inter[i])) != 0)
  ```

### 2. Edge-case Division Faults on Small Arrays
* **Problem:** Given an array size of `11` and `5` worker threads, the math tracks cleanly (`nb_chacun = 2`, `reste = 1`). However, if the array size drops below `NB_THREADS`, `nb_chacun` evaluates to `0`, creating threads with empty workloads where `debut > fin`.
* **Solution:** Behavior documented. For robust engineering, programmatically clamp or reduce `NB_THREADS` to match the array size if it falls below the minimum thread limit.

### 3. Redundant Logic inside Index Balance Blocks
* **Problem:** The secondary execution path `else if (i == reste)` merely resets `index_supple = 0;`, reproducing the default initialization state unnecessarily.
* **Solution:** Streamline the boundary distribution assignment using a compact ternary condition:
  ```c
  int index_supple = (i < reste) ? 1 : 0;
  ```

### 4. Index Variable Clashes in Status Logs
* **Problem:** The log message `"the sum of thread %d is %d"` incorrectly passed the loop modifier `i`, which at that scope matched the terminal array index variable (`inter->fin`) rather than the absolute worker thread identification code.
* **Solution:** Read the tracker identifier parameter explicitly from the struct allocation:
  ```c
  printf("the sum of thread %d is %d \n", inter->threads_id, somme);
  ```

### 5. Unchecked `pthread_join` Operational Status
* **Problem:** Invoking `pthread_join(threads[i], NULL)` without asserting its return code risks swallowing critical tracking failures silently (such as bad thread handlers).
* **Solution:** Wrap the teardown routines within explicit error validation traps using thread-safe string converters:
  ```c
  if ((ret = pthread_join(threads[i], NULL)) != 0) {
      fprintf(stderr, "%s\n", strerror(ret));
      exit(EXIT_FAILURE);
  }
  ```

### 6. Stack-Overflow Risks via Variable-Length Arrays (VLAs)
* **Problem:** Defining arrays like `pthread_t threads[NB_THREADS]` using runtime configurations transforms allocations into Variable-Length Arrays (VLAs), which can rapidly deplete stack space under high dynamic thread requests.
* **Solution:** When using a static `#define NB_THREADS 5` macro, this remains clean. If you transition to dynamic user-input threads later, refactor the application to use dynamic memory mapping via `malloc()`.

### 7. Mutable Global Access Identifiers
* **Problem:** The target dataset `table` is declared as a plain global array, exposing it to unintended mutations despite never being edited.
* **Solution:** Explicitly prepend the immutable type definition to safeguard data and assist compiler optimizations:
  ```c
  const int table[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  ```

### 8. Global State Pollution
* **Problem:** Keeping `taille_tab` and `table` inside the global scope works fine for low-level labs, but degrades code encapsulation in scaling applications.
* **Solution:** Shift the dataset structures cleanly into the localized context of `main()` and pass matching pointers to threads through initialization structures.

### 9. Arithmetic Overflow Risks on Large Data Matrices
* **Problem:** Summing massive elements or processing extremely deep integer ranges can cause standard signed `int somme` storage pipelines to wrap around and overflow.
* **Solution:** Upgrade target collection and aggregation parameters to wide-bit formats such as `long` or `long long`.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Derived from array partitioning methodologies, structural argument casting, and synchronous runtime forks described in *Programmation système sous Linux* by Christophe Blaess.
