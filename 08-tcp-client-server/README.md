# TCP Client / Server with Host and Service Resolution

## Description
This project features a pair of networking programs demonstrating a complete **TCP Client/Server architecture** under Linux.

The project contains two main executables:
* **`tcp_server`**: A concurrent server that listens on a dynamic port, accepts incoming client connections, and sends back the client's network address details.
* **`tcp_client`**: A client that connects to a server specified by hostname/IP and port/service name, then prints any payload transmitted by the server.

### Key Concepts Demonstrated
* Socket lifecycle operations: `socket()`, `bind()`, `listen()`, `accept()`, and `connect()`.
* Name and network resolution: `gethostbyname()`, `getservbyname()`, and `getprotobyname()`.
* Command-line argument processing using `getopt()`.
* Concurrent client handling via process cloning with `fork()`.

This is a learning project for system programming under Linux, inspired by the work of Christophe Blaess.

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler:** GCC
* **Tools:** Make utility

---

## Compilation
To compile both the client and server programs, run:
```bash
make
```
This produces the `tcp_server` and `tcp_client` executables. To clean up the build artifacts, run:
```bash
make clean
```

---

## Usage

### 1. Start the Server
Run the server executable in your first terminal window:
```bash
./tcp_server
```
The server will bind to a port and display its listening address, for example:
```text
My address >> IP= 0.0.0.0, Port: 45678
```

### 2. Connect with the Client
In a second terminal window, connect to the server using the designated port number:
```bash
./tcp_client -a localhost -p 45678
```

### Client Options

```bash
tcp_client [-a address] [-p port]
```

| Option | Description | Default Value |
| :--- | :--- | :--- |
| **`-a`** | Server address (IPv4 or hostname) | `localhost` |
| **`-p`** | Server port (numeric value or service name) | `2000` |
| **`-h`** | Displays usage help and exits | — |

*Example using a standard service name instead of a port number:*
```bash
./tcp_client -a localhost -p http
```

### Expected Client Output
```text
Your address: IP = 127.0.0.1, Port = 54321
Hello
```

---

## Problems Encountered & Solutions

### 1. `_GNU_SOURCE` Requirement
* **Problem:** System functions such as `getopt()`, `gethostbyname()`, `getservbyname()`, and `getprotobyname()` require the definition of `_GNU_SOURCE` (or `_DEFAULT_SOURCE`) alongside their correct standard headers (`<unistd.h>`, `<netdb.h>`).
* **Solution:** Added `#define _GNU_SOURCE` at the very top of `lib.h` before any standard headers are included.

### 2. Invalid Resolution Functions in `tcp_server.c`
* **Problem:** The original implementation invoked `getprotobynumber()` and `getservbyport()`, which were incorrect for this context. The intended logic required resolution by alphanumeric string names.
* **Solution:** Replaced the incorrect calls with `getprotobyname()` and `getservbyname()` inside the server code.

### 3. `getservbyname()` Failures Causing Segmentation Faults
* **Problem:** In `tcp_client.c`, if `getservbyname()` returned `NULL`, the application printed an error message but proceeded to dereference `servent->s_port`, throwing a critical NULL pointer dereference.
* **Solution:** Inserted an immediate `return -1;` safe-exit sequence right after the error diagnostic print.

### 4. Fragmented `sin_family` Configuration
* **Problem:** Inside the client, `memset()` clears the `sockaddr_in` struct, and `adresse.sin_family = AF_INET` is set downstream in `main()` after `lecture_arguments()`. This ordering functions correctly but introduces code fragility.
* **Solution:** The family assignment could be shifted directly inside `lecture_arguments()` right after the zeroing block. Though not strictly broken, it improves code maintainability.

### 5. `gethostbyname()` Is Deprecated
* **Problem:** The `gethostbyname()` API is obsolete, lacks thread-safety, and does not support IPv6 configurations.
* **Solution:** Modern network applications should transition to `getaddrinfo()`. For this educational demonstration, `gethostbyname()` is retained but explicitly documented as legacy.

### 6. Loose Input Validation with `sscanf()`
* **Problem:** Running a string like `"2000abc"` through `sscanf("2000abc", "%d", &num)` returns a valid token count of `1` and parses `2000`, silently swallowing the invalid trailing characters.
* **Solution:** Transition to a robust `strtol()` check combined with an `endptr` validation block for rigid type parsing.

### 7. Portability Issues with `fork()` + `SIGCHLD = SIG_IGN`
* **Problem:** While setting `SIGCHLD` to `SIG_IGN` automatically reaps zombie child processes on Linux, this behavior is non-portable. On certain POSIX platforms, ignoring the signal can cause subsequent `wait()` operations to block indefinitely.
* **Solution:** For enterprise-grade cross-compatibility, register an explicit `SIGCHLD` signal handler that clean-reaps children in a loop via `waitpid(-1, NULL, WNOHANG)`.

### 8. Indefinite Network Block on `connect()`
* **Problem:** If a target host is dead or unreachable, the `connect()` system call blocks the active thread for up to ~2 minutes while the kernel retries TCP SYN packets.
* **Solution:** Configure non-blocking sockets bound to a `select()` or `poll()` monitoring loop to enforce a customized timeout threshold.

### 9. Complete Lack of IPv6 Capabilities
* **Problem:** The codebase depends entirely on `AF_INET` and `struct sockaddr_in`, rendering it incompatible with IPv6 address scopes.
* **Solution:** Modernize the codebase architecture by migrating to modern dual-stack interfaces like `getaddrinfo()` utilizing `AF_UNSPEC` paired with a generic `struct sockaddr_storage`.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Inspired by the network, concurrent processing, and TCP socket exercises from *Programmation système sous Linux* by Christophe Blaess.
