# Multithreaded Locked-File Server

## Description
This program implements a concurrent TCP server that enables multiple clients to connect and perform operations on files simultaneously. 

Clients can open files, acquire exclusive write locks, modify content, and release locks. Locks are managed safely using the `fcntl()` API and are automatically cleaned up if a client unexpectedly disconnects. This project serves as a practical exercise for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **Concurrent TCP Architecture:** Runs on port `8081` using a thread-per-client model (`pthread`).
* **Interactive Command Protocol:** 
  * `OPEN <file>` : Opens (or creates) a target file.
  * `LOCK` : Places an exclusive write lock on the entire file.
  * `WRITE <text>` : Appends string data to the active file.
  * `UNLOCK` : Manually releases the lock.
* **Automatic Resource Cleanup:** Automatically releases system locks and closes file descriptors if a client abruptly disconnects.
* **Detached Thread Management:** Threads are fully isolated using `pthread_detach` to maximize resource efficiency.

---

## Prerequisites
To compile and run this project, you need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **`pthread` library** (usually bundled with `glibc`)
* **Make**
* **`telnet` or `netcat`** (for manual connection testing)

---

## Compilation

To compile and build the server binary, run the following command in your terminal:

```bash
make
```

---

## Usage

### 1. Launch the Server
Start the background server process from your main terminal:
```bash
./file_server
```

### 2. Connect via Client Terminal
Open a secondary terminal session and connect using `nc` (Netcat) or `telnet`:
```bash
nc localhost 8081
# OR
telnet localhost 8081
```

### Example Session Flow
```text
OPEN test.txt
OK Fichier ouvert

LOCK
Fichier Verrouiller

WRITE Hello world
OK Donnees ecrites

UNLOCK
Fichier Derrouiller
```

---

## Implementation Details
* **`client_context_t`:** A structure allocated per client to store isolated session states (client socket, active file descriptor, and filename).
* **`verrouiller_fichier()`:** Uses `F_SETLKW` (blocking wait) to guarantee an exclusive write lock across the file boundaries.
* **`deverrouiller_fichier()`:** Uses `F_SETLK` to clear existing records cleanly.
* **Main Loop:** The primary execution thread continuously listens and accepts connections, immediately offloading tracking logic to standalone client threads.

---

## Problems Encountered & Solutions

Here is a breakdown of the design challenges encountered during development and how they were resolved:

### 1. File descriptor leaks on client disconnect
* **Problem:** If a client abruptly disconnected without explicitly running `UNLOCK` or closing their session, the open file descriptor and lock structures lingered indefinitely on the server.
* **Solution:** Within `gerer_client()`, right after the `recv` loop terminates, an explicit evaluation checks if `ctx.file_fd != -1`. If true, the server safely triggers `deverrouiller_fichier()` and shuts down the file descriptor.

### 2. Client context corruption from stack-passing (Pass-by-Value)
* **Problem:** Passing the context structure by value to the handler thread isolated modifications locally, making them invisible to parent context lookups. Conversely, passing direct main-stack pointers introduced hazardous dangling pointer bugs when the loop recycled.
* **Solution:** Dynamically allocate `client_context_t` inside `main()` using `malloc()` right before initializing the thread, then hand off the heap address. *Note: Remember to explicitly run `free(ctx)` at the close of `gerer_client()` to prevent heap memory exhaustion.*

### 3. Thread-blocking via `F_SETLKW`
* **Problem:** Utilizing a blocking lock directive (`F_SETLKW`) means a client thread freezes entirely if another client holds an active lock on the same resource. While isolated to that thread, it makes the interface appear non-responsive to that client.
* **Solution:** For non-blocking interactive behavior, swap the directive to `F_SETLK`. This immediately drops a failure state back to the connection, allowing you to intercept `EACCES` or `EAGAIN` and notify the client that the file is currently busy.

### 4. Raw non-null-terminated network streams
* **Problem:** Streams read via `recv()` do not append null terminators (`\0`), exposing downstream functions like `strncmp()` or `strlen()` to out-of-bounds memory vulnerabilities.
* **Solution:** The codebase addresses this by explicitly enforcing `buffer[bytes_read] = '\0'` directly after pulling the segment, ensuring safe buffer constraints (where `recv` captures a maximum of `BUFFER_SIZE - 1`).

### 5. `strncpy` missing null terminators
* **Problem:** `strncpy(dst, src, n)` intentionally omits appending a trailing `\0` if the source length equals or exceeds the threshold `n`.
* **Solution:** The codebase patches this safely by manually overwriting the boundary offset right after copying:
  ```c
  ctx->filename[sizeof(ctx->filename) - 1] = '\0';
  ```

### 6. Memory leaks from detached thread tracking omissions
* **Problem:** If threads execute and exit without joining or being detached, their execution frame allocations remain bound to system memory.
* **Solution:** Applied `pthread_detach(thread_id)` right after invoking `pthread_create()` to declare fire-and-forget loops that clean up immediately upon termination.

### 7. Linker failures due to missing runtime libraries
* **Problem:** Compilation breaks with an `undefined reference to pthread_create` error.
* **Solution:** Explicitly append the `-lpthread` compilation flag to the linker recipe inside the workspace `Makefile`.

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on sockets, threads, and file locking.
