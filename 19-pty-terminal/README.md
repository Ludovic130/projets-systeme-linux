# Telnet-like Shell Server with Pseudo-Terminal (PTY)

## Description
This program implements a minimal **telnet-like remote shell server** utilizing a **pseudo-terminal (PTY)** under Linux.  
Each client connecting to the server is granted an isolated `/bin/sh -i` shell session, with input and output dynamically relayed through the PTY.  

This is a learning project for Linux system programming based on the work of **Christophe Blaess**.

## Features
* **Dynamic Ephemeral Port:** The TCP server automatically binds to a random free port and displays it at startup.
* **PTY Management:** Allocates an independent PTY (master + slave pairings) for every incoming client via `getpt()`, `grantpt()`, `unlockpt()`, and `ptsname()`.
* **Isolated Execution Environment:** Forks a dedicated child process per client that:
  * Detaches safely from the parent controlling terminal via `setsid()`.
  * Binds the slave PTY side to standard I/O pipelines (`stdin`, `stdout`, `stderr`) using `dup2()`.
  * Executes an interactive shell (`/bin/sh -i`).
* **Bidirectional I/O Multiplexing:** The parent process uses `select()` to manage concurrent, non-blocking data forwarding between the network socket and the PTY master.
* **Zombie Process Protection:** Explicitly configures `SIGCHLD` handling to automatically reap completed child sessions without system blockages.

## Prerequisites
* **OS:** Linux or Windows Subsystem for Linux (WSL) with a PTY-supported environment
* **Compiler:** GCC
* **Build Tool:** Make
* **Libraries:** `libutil` (Typically bundled; if missing, install via `sudo apt install libutil-linux-dev`)
* **Testing Utilities:** `telnet` or `netcat` (`nc`)

## Compilation
Compile the project using the tracking configuration:
```bash
make
```

*Note: If compilation fails due to an `"undefined reference to getpt"` error, manually append the `-lutil` flag to link the utility libraries:*
```bash
gcc -Wall -Wextra -g telshell.c -o telshell -lutil
```

## Usage

### 1. Launching the Server
Execute the binary on the hosting terminal:
```bash
./telshell
```
The program will display its listener coordinates:
```text
My address: IP = 0.0.0.0, Port = 45678
```

### 2. Client Connection
Open a separate terminal window and establish a remote network session using either **Telnet** or **Netcat**:

```bash
# Using Telnet
telnet localhost 45678

# Using Netcat
nc localhost 45678
```
Upon a successful handshake, you will be dropped directly into an interactive shell prompt where commands map straight to the server host environment.

## Implementation Details
* `getpt()`: Requests and opens a brand-new master PTY interface descriptor.
* `grantpt()`: Corrects security permissions and ownership parameters on the companion slave device.
* `unlockpt()`: Configures the state of the slave interface so it can successfully be opened by client sessions.
* `ptsname()`: Retrieves the string path layout targeting the target slave file location system descriptor (e.g., `/dev/pts/N`).
* `setsid()`: Completely severs the sub-process container away from the original calling terminal context structure.
* `cfmakeraw()`: Configures the master PTY descriptor to raw processing streams, turning off standard echo loops and character formatting intercepts.
* `copie_entrees_sorties()`: Coordinates synchronous network traffic and terminal input-output pipelines using asynchronous `select()` tracking loops.

## Problems Encountered and Solutions

### 1. Implicit Declaration Errors on Core PTY Helpers
* **Problem:** Functions like `getpt()`, `grantpt()`, `unlockpt()`, and `ptsname()` are POSIX/UNIX98 specific extensions. Lacking target definition macros, compilers fallback to assuming primitive return integers, causing memory corruption (`char*` truncations) and segmentation faults.
* **Solution:** Added `#define _GNU_SOURCE` at the very top of the shared layout dependencies (`lib.h`) before invoking system-wide include arrays.

### 2. Linker Issues ("undefined reference to getpt")
* **Problem:** Selected PTY automation wrappers reside directly within the `libutil` binaries rather than base standard allocations (`libc`) depending on target kernel variations.
* **Solution:** Attached `-lutil` instructions onto the build commands mapped within the `Makefile`.

### 3. Connection Deadlocks on Remote Exits
* **Problem:** Without parsing End-of-File (EOF) bounds reliably, reading iterations could stall indefinitely whenever clients force-closed active connections.
* **Solution:** Refactored bounds auditing inside the tracking loop `copie_entrees_sorties()`. Whenever a `read()` function outputs values indicating zero or less, the system aggressively tears down the socket layer. 
  * *Correction applied:* Updated evaluating conditions from `>= 0` down to highly targeted `> 0` logic statements to cleanly bypass passing unallocated data payloads (`write(fd, buffer, 0)`).

### 4. Slave File Descriptor Leakage inside Child Processes
* **Problem:** After remapping pipelines with `dup2(fd_esclave, ...)`, the raw underlying file descriptor handle remained unclosed inside the shell fork. While safe during typical `execv` handoffs, it generated structural leaks.
* **Solution:** Inserted cleanup closures on standard descriptor footprints directly behind duplication overrides:
  ```c
  if (fd_esclave > 2) close(fd_esclave);
  ```

### 5. Silent Runtime Core Shell Failures
* **Problem:** If a call to launch `/bin/sh` with `execv` encountered failures, execution leaked straight out into random fallback structures, producing unstable behaviors.
* **Solution:** Wrapped execution points with immediate failure checks:
  ```c
  perror("execv"); 
  exit(EXIT_FAILURE);
  ```

### 6. Invalid Address Sizing Limits in `bind()` Calls
* **Problem:** Passing standard `sizeof(struct sockaddr)` arrays forces generalized 16-byte boundaries. While functional on specific Linux instances out of sheer luck, it destroys safe architecture porting models.
* **Solution:** Explicitly scaled structures to map explicit configurations via `sizeof(adresse)` or `sizeof(struct sockaddr_in)`.

### 7. Binding Metadata Loss via `getsockname()` Tracking
* **Problem:** Passing variables straight through `getsockname()` safely captures tracking ports, but wipes initial configuration limits (`INADDR_ANY`).
* **Solution:** Deemed perfectly fine for this targeted educational demo. For multi-tier workflows, use distinct parameters to track separate properties.

### 8. Bad Handler Validations on `signal()` Setup Routines
* **Problem:** Evaluating returns using `signal(...) != 0` yields false errors because active configurations typically return `SIG_DFL` (which equals `0`).
* **Solution:** Corrected bounds validation to evaluate directly against macro targets:
  ```c
  if (signal(SIGINT, gestionnaire) == SIG_ERR) {
      perror("signal");
      exit(EXIT_FAILURE);
  }
  ```

### 9. Portability Gaps with `SIGCHLD` Ignoring Directives
* **Problem:** Setting `signal(SIGCHLD, SIG_IGN)` tells the Linux kernel to clean up dead forks on its own. While valid on Linux, POSIX considers this behavior unspecified, which can cause code to break on other UNIX-like systems.
* **Solution:** A cross-platform alternative is installing a dedicated tracker running non-blocking cleanup tracking (`waitpid(-1, NULL, WNOHANG)`) in a tight evaluation loop. The native Linux setup remains unchanged for this lab.

### 10. Lack of Encryption or Access Authorization Frameworks
* **Problem:** This tool acts as an unprotected server pipeline. Anyone who discovers the open port gets unauthenticated root-level capability hooks over raw target infrastructure.
* **Solution:** This codebase is restricted exclusively to local testing sandboxes. Never expose this app over open networks. Bind solely to `localhost` interfaces or run within virtualized environments.

### 11. System Resource Drain (PTY Starvation)
* **Problem:** High volume concurrent connection cycles risk hitting hard limits on operating system PTY allocations (`/dev/pts/N`).
* **Solution:** Program flow ensures immediate disposal of master socket bindings the moment client tracking routes return, which handles base scale requirements.

### 12. Undeclared Dependency Flags on `cfmakeraw()`
* **Problem:** Under glibc toolchains, raw mode modification remains hidden behind specific BSD expansion rules.
* **Solution:** Enforced globally using the `_GNU_SOURCE` macro layout declaration inside the header file.

## Security Warning
This software allows unauthenticated remote shell access to anyone who can connect to the port. **Do not run this application on an untrusted or public network.** Use it strictly within isolated development sandboxes (e.g., local virtual machines, containers, or restricted `localhost` networks).

## Author
* **Ludovic130**

## Reference
* Project specifications inspired by system programming exercises from **Christophe Blaess** (*Programmation système sous Linux*).
