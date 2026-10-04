# TCP Client with Host and Service Resolution

## Description
This program is a simple **TCP client** designed for Linux systems. It establishes a connection to a remote server specified by either an IP address or a hostname, along with a port number or a registered service name.

The application resolves network details using:
* **Address Resolution:** `inet_aton()` or `gethostbyname()`
* **Port/Service Resolution:** `sscanf()` or `getservbyname()`

Once connected, the client reads all incoming data transmitted by the server and streams it directly to the standard output (`stdout`) until the remote host closes the connection.

This is a learning project for system programming under Linux, inspired by the work of Christophe Blaess.

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler:** GCC
* **Tools:** Make utility
* **Testing:** A local or remote TCP server (e.g., `netcat`)

---

## Compilation
To build the executable, run:
```bash
make
```

---

## Usage

```bash
tcp_client [-a address] [-p port]
```

### Options

| Option | Description | Default Value |
| :--- | :--- | :--- |
| **`-a`** | Server address (IPv4 or hostname) | `localhost` |
| **`-p`** | Server port (numeric value or service name) | `2000` |
| **`-h`** | Displays usage help and exits | — |

### Examples

* **Connect to localhost on the default port (2000):**
  ```bash
  ./tcp_client
  ```

* **Connect to a specific IP address and port number:**
  ```bash
  ./tcp_client -a 192.168.1.10 -p 8080
  ```

* **Connect using a hostname and a standard service name:**
  ```bash
  ./tcp_client -a example.com -p http
  ```

### Quick Local Test with Netcat
1. In the first terminal, spin up a listening server:
   ```bash
   nc -l 2000
   ```
2. In a second terminal, start your client:
   ```bash
   ./tcp_client
   ```
3. Type text into the `nc` terminal; it will immediately stream and display in the client's terminal.

---

## Code Structure
* **`lecture_arguments()`**: Parses command-line options using `getopt()`. It handles host resolution via `inet_aton()` / `gethostbyname()`, and processes the port using `sscanf()` / `getservbyname()`.
* **`main()`**: Allocates the TCP socket, establishes the connection to the targeted server, and enters a processing loop that reads from the network socket and writes to `stdout`.

---

## Problems Encountered & Solutions

### 1. `getopt()`, `gethostbyname()`, `getservbyname()` not declared
* **Problem:** These functions require `_GNU_SOURCE` (or `_DEFAULT_SOURCE`) along with specific headers (`<unistd.h>`, `<netdb.h>`). Missing them triggers implicit-declaration compiler warnings and potential runtime segmentation faults.
* **Solution:** Added `#define _GNU_SOURCE` at the very top of `lib.h`, and verified the inclusion of `<unistd.h>` and `<netdb.h>`.

### 2. `gethostbyname()` is deprecated
* **Problem:** `gethostbyname()` is obsolete, is not thread-safe, and lacks IPv6 support. 
* **Solution:** For production systems, `getaddrinfo()` is preferred. It remains in this educational demo for simplicity but is documented here as deprecated.

### 3. `sscanf()` silently accepts trailing characters
* **Problem:** Parsing a string like `"2000abc"` using `sscanf("2000abc", "%d", &num)` returns `1` and extracts `2000`, silently ignoring the invalid `"abc"` suffix.
* **Solution:** Transition to `strtol()` accompanied by an `endptr` validation check:
  ```c
  char *end;
  long v = strtol(port, &end, 10);
  if (*end == '\0') { /* String is a valid clean integer */ }
  ```

### 4. Unhandled `getservbyname()` failures
* **Problem:** If `getservbyname()` fails and returns `NULL`, the code prints an error message but proceeds to dereference `servent->s_port`, leading to a critical NULL pointer dereference (segmentation fault).
* **Solution:** Inject an immediate error exit path following the error log:
  ```c
  if ((servent = getservbyname(port, 'tcp')) == NULL) {
      fprintf(stderr, "Service %s unknown\n", port);
      return -1;
  }
  ```

### 5. `read()` may return partial data
* **Problem:** The `read()` system call can return fewer bytes than requested even when more data is on the way. 
* **Solution:** While our streaming loop handles this gracefully (it writes exactly `nb_lus` bytes), protocol-aware clients would require continuous looping and stream buffering until a full message delimiter is satisfied.

### 6. Unchecked `write()` return value
* **Problem:** Calling `write(STDOUT_FILENO, buffer, nb_lus)` might write fewer bytes than expected if the output pipe or buffer fills up.
* **Solution:** For a baseline client, the current workflow suffices. For enterprise robustness, implement a loop that guarantees all bytes are written, or use `fwrite()` on `stdout`.

### 7. Overlapping error reporting between `inet_aton()` and `gethostbyname()`
* **Problem:** When an invalid IP format is entered (e.g., `999.999.999.999`), `inet_aton()` fails and falls back to `gethostbyname()`, producing a confusing DNS lookup error.
* **Solution:** Behavior documented. Optional optimization includes catching dotted-quad strings early to reject them before falling back to DNS resolution.

### 8. Structural fragility with `adresse.sin_family` timing
* **Problem:** A `memset()` inside `lecture_arguments()` blanks out the entire structure, and `sin_family = AF_INET` is set later in `main()`. Reordering code could easily break the initialization.
* **Solution:** Safely set `adresse->sin_family = AF_INET;` directly inside `lecture_arguments()` right after the structure is zeroed out.

### 9. Lack of `SIGPIPE` handling
* **Problem:** If a client writes to a socket that has been abruptly closed by the server, the kernel kills the process with a `SIGPIPE` signal by default.
* **Solution:** Our client currently only reads, escaping this issue. If write features are added, register `signal(SIGPIPE, SIG_IGN)` during startup and gracefully catch `EPIPE` errors on `write()`.

### 10. Indefinite connection timeouts
* **Problem:** If a destination server is completely unreachable, `connect()` blocks the runtime environment for up to ~2 minutes while the kernel retries TCP SYN packets.
* **Solution:** Use non-blocking sockets integrated with `select()` or `poll()` loops to configure a customized connection timeout window.

### 11. Complete lack of IPv6 support
* **Problem:** The codebase relies strictly on `AF_INET` and `struct sockaddr_in`, rendering it incompatible with IPv6 architectures.
* **Solution:** Refactor the networking logic to use modern dual-stack APIs like `getaddrinfo()` with `AF_UNSPEC` paired with `struct sockaddr_storage`.

---

## Credits
* **Author:** Ludovic130
* **Reference:** Inspired by the network and TCP socket programming exercises from *Programmation système sous Linux* by Christophe Blaess.
