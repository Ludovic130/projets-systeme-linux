# Minimal HTTP Server in C

## Description
This program implements a minimal **HTTP/1.1 web server** in C using standard POSIX sockets.  

It listens on port `8080`, accepts one incoming client at a time, serves a fixed HTML page, and immediately drops the connection. This project serves as a foundational exercise for network programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **TCP Server Socket:** Listens persistently on port `8080`.
* **Static Content Delivery:** Serves a hardcoded HTML landing page (titled `LUDOX`).
* **HTTP/1.1 Standard Headers:** Responds with explicit `Content-Type: text/html` and `Content-Length` metadata fields.
* **Isolated Connections:** Enforces a `Connection: close` workflow (single request processing loop per connection).

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **Make**
* **A web browser** or **`curl`** (for end-to-end network testing)

---

## Compilation

To generate the `http_server` executable binary, execute the following command in your terminal:

```bash
make
```

---

## Usage

### 1. Launch the Server
Start the background server process from your command line:
```bash
./http_server
```

### 2. Connect via Web Client
Open a web browser and navigate to:
```text
http://localhost:8080
```
Alternatively, test the endpoint from a secondary terminal using `curl`:
```bash
curl http://localhost:8080
```

### Server-Side Console Output
```text
HTTP server started on http://localhost:8080
Waiting for connections...
Response sent to 127.0.0.1
```

---

## Implementation Details
* **`socket()`:** Spawns an IPv4 network boundary streaming TCP socket.
* **`bind()`:** Binds the socket to port `8080` listening universally across all network adapters (`INADDR_ANY`).
* **`listen()`:** Moves the socket state into passive listen mode with a backlog constraint threshold of `5`.
* **`accept()`:** Blocks the calling thread until an external network client establishes a connection.
* **`recv()`:** Captures raw byte streams from the client request (safely ignored in this static demonstration).
* **`build_http_response()`:** Formats standard HTTP compliance text envelopes utilizing safe `snprintf` actions.
* **`send()`:** Flushes data buffers down the raw socket stream to the browser client.
* **`close()`:** Drops client socket connections cleanly to prevent trailing socket resource consumption.

---

## Problems Encountered & Solutions

Here is a list of technical bugs, architectural limitations, and race conditions identified during development, alongside their resolutions:

### 1. Pointer overwrites via static internal buffers in `inet_ntoa()`
* **Problem:** The legacy `inet_ntoa()` routine returns a pointer referencing a shared, static internal buffer. Invoking it multiple times inside a single `printf` string causes matching components to mirror the final parsed evaluation. Additionally, it is inherently thread-unsafe.
* **Solution:** Transition to the modern `inet_ntop()` API, forcing the system to populate a dedicated thread-safe, user-allocated character buffer instead:
  ```c
  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &address.sin_addr, ip, sizeof(ip));
  printf("Response sent to %s\n", ip);
  ```

### 2. State pollution on shared `address` structures
* **Problem:** Using a single `address` variable structure for both initial `bind()` configurations and downstream `accept()` actions allows incoming data structures to completely overwrite your baseline server configurations. This drops tracking values after a single transaction loop.
* **Solution:** Instantiate an independent client tracking variable container (`client_addr`) specifically allocated for your acceptance filters:
  ```c
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  client_dt = accept(server_dt, (struct sockaddr *)&client_addr, &client_len);
  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
  ```

### 3. Missing error filtering on `recv()` returns
* **Problem:** If a client drops their connection packet early or triggers a socket error, `recv()` flags a `0` or `-1` result code. Ignoring this status forces the execution flow to format and send phantom responses regardless.
* **Solution:** Validate network transaction bounds cleanly. Additionally, do not terminate the entire application (`exit(EXIT_FAILURE)`) over a single bad remote request; use a loop fallback statement instead:
  ```c
  ssize_t n = recv(client_dt, buffer, BUFFER_SIZE - 1, 0);
  if (n < 0) {
      perror("recv");
      close(client_dt);
      continue; // Keep the server active for other incoming clients
  }
  ```

### 4. Memory vulnerabilities via unchecked `snprintf()` bounds
* **Problem:** The `snprintf()` routine returns the full character size it *would* output if it had unconstrained buffer headroom. If your HTML file or payload size matches or surpasses the buffer limits, transmitting based on that unchecked integer size causes out-of-bounds page leakage or segmentation faults.
* **Solution:** Securely sanitize length boundaries before passing the counter to transmission directives:
  ```c
  size_t n = snprintf(response, BUFFER_SIZE, ...);
  *resp_len = (n < BUFFER_SIZE) ? n : BUFFER_SIZE - 1;
  ```

### 5. Address socket blocks caused by `TIME_WAIT` locks
* **Problem:** Terminating and rebooting the server process repeatedly causes `bind()` to fail with an `EADDRINUSE` (Address already in use) error because the operating system locks the port in a defensive `TIME_WAIT` state for ~60 seconds.
* **Solution:** Inject the `SO_REUSEADDR` configuration option flag into the socket right before triggering binding actions:
  ```c
  int opt = 1;
  if (setsockopt(server_dt, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }
  ```

### 6. Resource trapping on unhandled `SIGINT` (Ctrl+C) terminations
* **Problem:** Terminating the running instance abruptly via `Ctrl+C` prevents the program execution loop from closing descriptors, dirtying operating system port allocation frameworks.
* **Solution:** Intercept the signal events via a dedicated signal catcher routine to close down parent sockets cleanly before exiting:
  ```c
  signal(SIGINT, handle_sigint);
  ```

### 7. Thread freezing on single-client blockers
* **Problem:** Because it relies on a single-client blocking execution layout, any client that opens a link and hangs without transmitting data locks the server inside `recv()`, starving all other incoming web traffic.
* **Solution:** For advanced deployments, introduce threading (`pthread`), multi-processing (`fork`), or polling abstractions (`epoll`). While a single-client sequential flow is fine for a minimal test script, this boundary must remain documented.

### 8. Partial HTTP header ingestion limits
* **Problem:** High-volume or fragmented packet streams can spread HTTP header envelopes across several separate data streams. Reading only a single `recv()` block risks capturing incomplete headers.
* **Solution:** For robust parsing operations, implement a reading loop mechanism that continuously pulls slices until the strict double carriage-return newline sequence (`\r\n\r\n`) marker is fully found.

### 9. String formatting hazards from unescaped raw network strings
* **Problem:** Data streams received through `recv()` are not automatically bound with terminal null characters (`\0`), making standard `%s` logging operations susceptible to out-of-bounds buffer bleeding.
* **Solution:** While this program safely bypasses request prints, always append structural boundaries if logging functions are introduced:
  ```c
  buffer[n] = '\0';
  ```

### 10. Rigid payload limits due to fixed buffer sizing
* **Problem:** Relying strictly on a macro configuration boundaries (`BUFFER_SIZE 4096`) will crash or mangle responses if the source code attempts to ingest or serve large multimedia content blocks.
* **Solution:** Keep the current static limits for micro-demos, but use dynamic heap memory resizing scripts if expanding the scope to file servers.

### 11. Missing document text encoding details
* **Problem:** Omitting charset details from `Content-Type: text/html` triggers encoding estimation routines across modern browsers, which can mangle localized or special character outputs.
* **Solution:** Explicitly serve the precise character layout within response formatting structures:
  ```text
  Content-Type: text/html; charset=utf-8
  ```

### 12. Absence of RFC-compliant fallback headers
* **Problem:** The HTTP/1.1 specifications recommend serving standard `Date` and server identifier configurations (`Server`). Missing these can cause rigid automated agents or legacy clients to reject transmissions.
* **Solution:** Enhance your response text framework to incorporate standard environment parameters:
  ```text
  "Date: <RFC 1123 date>\r\n"
  "Server: MiniC/1.0\r\n"
  ```

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on sockets.
