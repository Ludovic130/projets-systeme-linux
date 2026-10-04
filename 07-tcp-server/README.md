# TCP Server with Host and Service Resolution

## Description
This program implements a concurrent **TCP Server** engineered for Linux environments. It binds to a dynamic port or a specific network service, accepts incoming connection requests from clients, and offloads individual tasks to independent processes.

### Key Features
* Creates a streaming listening socket (`socket`, `bind`, `listen`, `accept`).
* Supports multiple parallel clients using concurrent process branching via `fork()`.
* Automatically retrieves network metadata using `getsockname()` and `getpeername()`.
* Performs name, protocol, and service resolution via standard POSIX APIs (`gethostbyname`, `getservbyname`, `getprotobyname`).
* Transmits client connection diagnostics back to the remote peer upon a successful handshake.

This is a learning project for system programming under Linux, inspired by the work of Christophe Blaess.

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler:** GCC
* **Tools:** Make utility

---

## Compilation
To compile the server executable, run:
```bash
make
```
This produces the `tcp_server` executable. To clean up and delete the compiled binary, run:
```bash
make clean
```

---

## Usage

Start the server process in your terminal window:
```bash
./tcp_server
```

Upon initialization, the server will output its local listening attributes:
```text
My address >> IP= 0.0.0.0, Port: 45678
```

Whenever a new client connects, the parent or child process reports connection data on standard output:
```text
Local connection: IP= 127.0.0.1, Port: 45678
Remote: IP = 127.0.0.1, Port = 54321
```

The connected client process will simultaneously receive the following greeting message over the stream:
```text
Your address: IP = 127.0.0.1, Port = 54321
Hello
```

---

## Code Structure
* **`creat_socket_stream()`**: Resolves the host, network protocol, and service definitions, then generates and binds the primary TCP listening socket.
* **`affiche_adresse_socket()`**: Queries and displays the internal local IP address and port mapping using `getsockname()`.
* **`traite_connexion()`**: Handles an isolated client stream—gathers client attributes with `getpeername()`, echoes the data back to the client, and safely shuts down the connection channel.
* **`server_tcp()`**: Orchestrates the infinite master server loop—manages incoming sync events and forks localized worker child processes per peer client.
* **`main()`**: Operational entry point that immediately boots into the `server_tcp()` routine.

---

## Problems Encountered & Solutions

### 1. `_GNU_SOURCE` Requirement
* **Problem:** Internal system functions like `gethostbyname()`, `getservbyname()`, `getprotobyname()`, and `inet_ntoa()` require `_GNU_SOURCE` (or `_DEFAULT_SOURCE`) to expose definitions correctly across modern library headers (`<netdb.h>`, `<arpa/inet.h>`, `<netinet/in.h>`).
* **Solution:** Declared `#define _GNU_SOURCE` at the very top of `lib.h` before including any standard libraries.

### 2. Contradictory Function Names in Error Traces
* **Problem:** Legacy code paths utilized error descriptors like `perror("getprotobynumber")` and `perror("getservbyport")`, despite the code invoking `getprotobyname()` and `getservbyname()`, heavily confusing debugging steps.
* **Solution:** Aligned all `perror()` output strings with the true programmatic runtime functions.

### 3. `gethostbyname()` Obsolescence
* **Problem:** The `gethostbyname()` framework lacks thread safety and completely misses support for IPv6 lookups. It is fundamentally deprecated.
* **Solution:** Documented. For modern designs, migrating to `getaddrinfo()` is highly encouraged, though this educational implementation retains the legacy API.

### 4. Hardcoded `write()` Buffer Lengths
* **Problem:** Evaluating buffer limits manually inside `write(sock, "Votre adresse : ", 16)` introduces maintainability bugs if strings change length down the line.
* **Solution:** Substituted hardcoded constants with explicit dynamic `strlen()` calls:
  ```c
  const char *msg = "Your address: ";
  write(sock, msg, strlen(msg));
  ```

### 5. Premature `close()` Interruptions
* **Problem:** Dropping the socket via `close()` immediately after sending might occasionally drop pending data buffers on specific kernels by throwing a connection reset (RST).
* **Solution:** While standard TCP teardowns deliver short buffers seamlessly, transitioning to a graceful one-way write shutdown before a complete socket close provides predictable behavior:
  ```c
  shutdown(sock, SHUT_WR); // Signal FIN to peer
  close(sock);
  ```

### 6. Non-Portable Process Deadlock Risks (`SIGCHLD = SIG_IGN`)
* **Problem:** Ignoring `SIGCHLD` automatically suppresses zombie generation on modern Linux distributions, but this behavioral pattern breaks standard compliance on strict POSIX systems, occasionally forcing `wait()` blocks or returning `ECHILD`.
* **Solution:** Implemented a standard-compliant asynchronous signal handler routine that reaps children via non-blocking loops:
  ```c
  void reaper(int sig) {
      while (waitpid(-1, NULL, WNOHANG) > 0);
  }
  signal(SIGCHLD, reaper);
  ```

### 7. Transient Errors Terminating the Active Daemon
* **Problem:** Minor temporary failures during `accept()` loops (such as connection aborts like `ECONNABORTED`) forced the entire daemon to return `-1` and exit prematurely.
* **Solution:** Enforced check exceptions within the event queue to gracefully bypass soft signals and sustain server uptime:
  ```c
  if (sock_connectee < 0) {
      if (errno == EINTR || errno == ECONNABORTED) continue;
      perror("accept");
      return -1;
  }
  ```

### 8. Sudden Application Termination via `SIGPIPE`
* **Problem:** If a remote client severs a connection while the server is mid-transit during a write transaction, the OS kernel forcefully shuts down the server using a unhandled `SIGPIPE` signal.
* **Solution:** Overrode default system signal behaviors by declaring `signal(SIGPIPE, SIG_IGN)` during system startup and validating for standard error codes like `EPIPE`.

### 9. Structural Fragmentation between `send()` and `write()` APIs
* **Problem:** The source code haphazardly nested standard `write()` calls alongside formal network socket `send()` calls.
* **Solution:** Uniformly standardized on the network-optimized `send()` API paired with standard safety flags to natively mitigate process interruption side effects:
  ```c
  send(sock, buffer, strlen(buffer), MSG_NOSIGNAL);
  ```

### 10. Abstract Network Limits (No IPv6 Support)
* **Problem:** The programmatic constraints of `AF_INET` alongside `struct sockaddr_in` prevent the server from hosting connections across IPv6 scopes.
* **Solution:** Future architectures can bypass this constraint by adapting socket generation routines around generic `struct sockaddr_storage` contexts alongside `getaddrinfo()`.

### 11. Thread Overlaps with Static Buffer APIs (`inet_ntoa`)
* **Problem:** The `inet_ntoa()` utility passes data back using an internal shared static buffer memory space, causing structural multi-threaded overrides or duplicate outputs within complex formatting routines.
* **Solution:** Upgraded network address transformations to utilize localized reentrant safety APIs like `inet_ntop()`:
  ```c
  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &adresse.sin_addr, ip, sizeof(ip));
  ```

### 12. Unbounded Execution Pipelines via `sprintf()`
* **Problem:** Unbounded calls to `sprintf(buffer, ...)` within `traite_connexion()` introduce a significant buffer overflow vector if network variables unexpectedly outgrow the static `256`-byte buffer layout.
* **Solution:** Refactored memory manipulation flows to leverage length-delimited alternatives:
  ```c
  snprintf(buffer, sizeof(buffer), "IP = %s, Port = %u \n", ip, port);
  ```

---

## Security Warning
This application provides no internal authentication layers and explicitly routes low-level network topology records back to connecting clients. It is developed solely for systemic education and laboratory use cases. **Do not expose this server process on unencrypted public network topologies.**

---

## Credits
* **Author:** Ludovic130
* **Reference:** Derived from concurrent computing, network resolution, and standard TCP stream primitives outlined in *Programmation système sous Linux* by Christophe Blaess.
