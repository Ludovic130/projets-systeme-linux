# TCP Socket Project – Daemon and Client

## 1. Overview
This project implements a client-server network communication in C using TCP sockets (`AF_INET`, `SOCK_STREAM`). It consists of two separate programs:

- **daemon:** A TCP server that runs in the background (as a daemon), listens on a port, accepts incoming connections, and sends each client its own IP address and port back.
- **client:** A program that connects to the daemon, reads the messages sent by the server, and displays them on the standard output.

The connection between the two relies on the TCP/IP protocol stack: the client opens a socket, connects to the server's address and port, and exchanges data using `read()` and `write()`.

## 2. Communication Architecture

```text
┌─────────────┐         TCP/IP          ┌─────────────┐
│   CLIENT    │ ◄─────────────────────► │   DAEMON    │
│             │   connect / read /      │             │
│  socket()   │   write / close         │  socket()   │
│  connect()  │                         │  bind()     │
│  read()     │                         │  listen()   │
│  write()    │                         │  accept()   │
└─────────────┘                         └─────────────┘
```

### Steps on the Daemon Side
- `chdir("/")`: Changes the current working directory to the root directory.
- **First `fork()`:** The parent process exits, allowing the child process to continue running.
- `setsid()`: The child process becomes a session leader, detached from the controlling terminal.
- **Second `fork()`:** Ensures the process is no longer a session leader.
- `close(i)`: Closes all file descriptors (including `stdin`, `stdout`, and `stderr`).
- `server_tcp()`: Creates the socket, binds it, puts it into listening mode, and accepts incoming connections.
- For each connection, a `fork()` creates a child process that handles the connection via `traite_connexion()`, while the parent closes the connected socket and waits for the next incoming connection.
- `signal(SIGCHLD, SIG_IGN)` is used to automatically prevent zombie processes.

### Steps on the Client Side
- `lecture_arguments()`: Parses the `-a` (address) and `-p` (port) command-line arguments using `getopt()`.
- `socket()`: Creates a TCP socket.
- `connect()`: Connects to the server's IP address and port.
- `read()`: Reads the incoming messages sent by the server.
- `write()`: Outputs the messages directly to `STDOUT_FILENO`.
- The loop exits when the server closes the connection (causing `read()` to return 0).

## 3. The Link Between Daemon and Client

The communication link is established via three core elements:

| Element | Daemon Side | Client Side |
| :--- | :--- | :--- |
| **IP Address** | `INADDR_ANY` (all interfaces) | `localhost` by default, customizable with `-a` |
| **Port** | `htons(0)` (dynamic port allocation) | `2000` by default, customizable with `-p` |
| **Protocol** | TCP (`SOCK_STREAM`) | TCP (`SOCK_STREAM`) |

The client must know the exact address and port of the server to connect to it. Because the daemon uses a dynamic port configuration (`htons(0)`), the operating system kernel assigns a random available port. The client defaults to looking for port `2000`, but this can be adjusted with `-p`.

> **Important:** For the client to connect successfully, you must find out the real port assigned to the daemon. Since the daemon uses `htons(0)`, the port is picked randomly by the kernel. You would either need to check it in the system logs (`syslog`), or hardcode an explicit port within `creat_socket_stream()`.

## 4. Compilation

```bash
gcc -D_GNU_SOURCE -Wall -Wextra -o daemon daemon.c
gcc -D_GNU_SOURCE -Wall -Wextra -o client client.c
```

## 5. Execution

```bash
# Step 1: Launch the daemon in the background
./daemon

# Step 2: Execute
journalctl -t daemon       

# output :
❯ journalctl -t daemon
Hint: You are currently not seeing messages from other users and the system.
      Users in groups 'adm', 'systemd-journal' can see all messages.
      Pass -q to turn off this notice.
sept. 16 14:01:46 localhost.localdomain daemon[145514]: IP = 0.0.0.0, port = 41855
sept. 16 14:03:53 localhost.localdomain daemon[146482]: IP = 0.0.0.0, port = 36371
sept. 16 14:04:35 localhost.localdomain daemon[147019]: IP = 0.0.0.0, port = 60281
sept. 16 14:13:32 localhost.localdomain daemon[147525]: IP = 0.0.0.0, port = 58849
sept. 16 14:18:58 localhost.localdomain daemon[148387]: IP = 0.0.0.0, port = 47569
sept. 16 14:36:43 localhost.localdomain daemon[150040]: IP = 0.0.0.0, port = 42637
sept. 16 14:37:48 localhost.localdomain daemon[150218]: IP = 127.0.0.1, port = 42637

# Step 3: Launch the client
./client -p (port)

```

## 8. Encountered Issues

### 8.1 Daemon and Standard Output
The daemon closes all file descriptors (`close(i)` for `i` from 0 to `_SC_OPEN_MAX`), including `stdout` (1) and `stderr` (2). Consequently:
- `fprintf(stdout, ...)` operations do not display anything.
- The only visible messages are those sent via `syslog()`.
- After `close(i)`, the first newly opened file or socket might reuse descriptor 1, which represents a potential bug.
- **Solution:** Use `syslog()` everywhere, or only close file descriptors starting from 3 and redirect `stdout`/`stderr` to a log file instead.

### 8.2 Daemon Dynamic Port Allocation
The daemon uses `htons(0)` for its port, meaning the kernel assigns a random available port. As a result, the client cannot know the default port beforehand. You must:
- Either read the assigned port from the system logs (`syslog`).
- Or fix an explicit, hardcoded port inside `creat_socket_stream()`.

### 8.3 Finding the Logs
`/var/log/syslog` might not contain the messages if `rsyslog` is not installed on the system. Use `journalctl` instead:
```bash
journalctl -t serveur_tcp
journalctl SYSLOG_FACILITY=3
journalctl | grep "daemon"
```

### 8.4 Missing `openlog()` call
Without calling `openlog()`, the log identifier defaults to the program's binary name. Add the following to the beginning of `main()`:
```c
openlog("serveur_tcp", LOG_PID | LOG_CONS, LOG_DAEMON);
```

### 8.5 Socket Closure and Client-Side Reception
In `traite_connexion`, the socket is closed immediately after calling `send()`. If the client has not had enough time to read, data loss can occur. Use `shutdown(sock, SHUT_WR)` or introduce a short delay before calling `close()`.

### 8.6 `fork()` and Zombie Processes
`signal(SIGCHLD, SIG_IGN)` is used to prevent zombie processes, but it also prevents the `wait()` system call from working properly. This is acceptable for this implementation but should be documented.

### 8.7 Missing Error Handling for `accept()`
If `accept()` fails, the daemon exits its loop and terminates. It would be safer to use `continue` or handle the error gracefully without killing the entire server.

### 8.8 Fixed Buffer Size
`buffer[256]` and `LG_BUFFER` are fixed sizes. Any incoming string longer than this limit will be truncated. Consider implementing dynamic memory allocation or choosing a significantly larger buffer size.

## 9. Areas for Improvement

- Add `openlog()` to clearly identify the daemon process in the system logs.
- Replace all instances of `fprintf(stdout, ...)` with `syslog()`.
- Enforce an explicit port number or log the randomly selected port so the client can find it.
- Use `shutdown()` before `close()` to ensure all buffered data is reliably transmitted to the client.
- Handle `SIGPIPE` in case the client abruptly disconnects during a write operation.
- Systematically check return values for `socket`, `bind`, `listen`, `accept`, and `fork`.
- Use `poll()` or `select()` to multiplex and handle multiple clients within a single process instead of spawning new forks.
- Implement a `read()` timeout on the client side.
- Document the application-level protocol (which currently relies on raw plaintext strings).

## Auteur

**Ludovic130**

## Références

**Le livre de christphe blaess sur la programmation système en C sous linux**