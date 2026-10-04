# getrusage Demo – Process Statistics

## Description
This program demonstrates the use of the `getrusage()` system call to retrieve resource usage statistics for a process under Linux.  

It can display statistics for:
- **The calling process itself** (`RUSAGE_SELF`).
- **Its terminated child processes** (`RUSAGE_CHILDREN`), when a command is passed as an argument.

This is a learning project for Linux system programming based on the work of **Christophe Blaess**.

## Prerequisites
* **OS:** Linux or Windows Subsystem for Linux (WSL)
* **Compiler:** GCC
* **Build Tool:** Make

## Compilation
To compile the project, run:
```bash
make
```

## Usage

### 1. Statistics for the Calling Process
To view the resource usage of the demo program itself, run it without arguments:
```bash
./getrusage_demo
```

### 2. Statistics for a Child Process
To view the statistics of a child process, pass the command as a string argument. The command will be executed using `system()`, and its statistics will display after it terminates:
```bash
./getrusage_demo "ls -R /"
```

## Example Output
```bash
$ ./getrusage_demo
Time in user mode: 0 s. and 1 ms
Time in kernel mode: 0 s. and 2 ms

Number of minor page faults: 120
Number of major page faults: 0
Number of process swaps: 0
```

## Code Structure

### Core Implementation
* `getrusage(RUSAGE_SELF, ...)`: Fetches statistics for the current running process.
* `getrusage(RUSAGE_CHILDREN, ...)`: Fetches statistics for terminated and waited-for child processes (executed via `system()`).

### Captured `struct rusage` Fields
* `ru_utime`: User CPU time.
* `ru_stime`: System (kernel) CPU time.
* `ru_minflt`: Minor page faults (reclaiming a page without I/O).
* `ru_majflt`: Major page faults (requiring disk I/O).
* `ru_nswap`: Number of times the process was swapped out of main memory.

## Problems Encountered and Solutions

### 1. Duplicate `getrusage()` Call
* **Problem:** `getrusage()` was called twice in a row on the exact same structure. The second call completely overwrote the data from the first, making it redundant and confusing.
* **Solution:** Removed the second call. Only one execution is required:
  ```c
  if (getrusage(lesquelles, &statistiques) != 0) {
      fprintf(stderr, "Unable to get statistics\n");
      exit(EXIT_FAILURE);
  }
  ```

### 2. Ignoring the `system()` Return Value
* **Problem:** The return value of `system()` was unchecked. If the target command failed (e.g., command not found), the program continued silently and printed misleading or zeroed statistics.
* **Solution:** Added a safety check for the `system()` return value:
  ```c
  int ret = system(argv[1]);
  if (ret == -1) {
      perror("system");
      exit(EXIT_FAILURE);
  }
  ```

### 3. `RUSAGE_CHILDREN` Captures Terminated Children Only
* **Problem:** `RUSAGE_CHILDREN` only returns data for child processes that have already terminated and been waited for. If a child is still running, its resource data is excluded. It also accumulates data across *all* terminated children, not just the most recent one.
* **Solution:** This is the native behavior of Linux. It has been documented accordingly. For tracking a specific, individual child process, `wait4()` should be used instead of `system()`.

### 4. `system()` Executes via `/bin/sh -c`
* **Problem:** The `system()` function interprets its argument as a shell command by executing `/bin/sh -c "command"`. This means shell metacharacters (pipes, redirections, globs) are automatically parsed, which can lead to unexpected side effects if a raw execution is expected.
* **Solution:** For a strict, raw binary execution, a combination of `fork()` + `execvp()` is preferred. For this demonstration, `system()` is left intact because allowing shell features is beneficial.

### 5. Confusion Between `RUSAGE_SELF` and `RUSAGE_CHILDREN`
* **Problem:** It is easy to mistake how the scope of these flags works.
* **Solution:** Clarified the scope in the documentation. `RUSAGE_SELF` evaluates only the calling process, while `RUSAGE_CHILDREN` strictly gathers data from dead, reaped child processes. The logic dynamically switches between them based on `argc`.

### 6. Truncation in `ru_utime.tv_usec / 1000` Integer Division
* **Problem:** Dividing microseconds by 1000 using standard integer division truncates the remainder. While acceptable for a rough estimate, it discards precision.
* **Solution:** If microsecond precision is needed, print `tv_usec` directly. For high-precision sub-millisecond displays, use floating-point arithmetic.

### 7. Massive `ru_minflt` / `ru_majflt` Value Spikes
* **Problem:** The fields `ru_minflt` and `ru_majflt` use the `long` data type. On long-running processes, these values can overflow standard capacities on certain architectures.
* **Solution:** Printed them using `%ld`. For maximum cross-platform safety, cast them to `intmax_t` and format using `PRIiMAX` from `<inttypes.h>`.

### 8. Blocking Limitations with `system()`
* **Problem:** The execution flow blocks entirely at `system()` until the child process completes, preventing the main program from executing concurrent background tasks.
* **Solution:** This is a structural limitation. For asynchronous behaviors, implement non-blocking flows using `fork()` + `execvp()` + `waitpid()` (with the `WNOHANG` flag) or set up dedicated signal handlers.

### 9. Cumulative Statistics Reset Behavior
* **Problem:** Under Linux, calling `getrusage(RUSAGE_CHILDREN, ...)` returns the combined total of all terminated children up to that moment. Repeating the call will return identical values unless a new child process terminates in the interim.
* **Solution:** Documented this cumulative tracking logic. If clean, isolated per-child metrics are needed, utilize `wait4()`, which populates an isolated `struct rusage` for the specific reaped child.

### 10. Missing `<sys/time.h>` Header
* **Problem:** The `struct rusage` relies internally on `struct timeval`, which is defined in `<sys/time.h>`. On certain Linux toolchains, `<sys/resource.h>` does not implicitly include it.
* **Solution:** Explicitly included both required headers inside the shared project header file (`lib.h`).

## Author
* **Ludovic130**

## Reference
* Based on system programming exercises by **Christophe Blaess** (*Programmation système sous Linux*).
