# Linux System Programming Projects

A comprehensive collection of **25 C programs** developed during an in-depth study of the reference book *Programmation système sous Linux* by **Christophe Blaess**.

---

## Project Index

| # | Project Directory | Key Themes & System Calls |
| :---: | :--- | :--- |
| **01** | [`01-mini-shell-v0/`](01-mini-shell-v0/) | Multi-processing (`fork`, `execvp`), string tokenization (`strtok`) |
| **02** | [`02-mini-shell-v1-cd/`](02-mini-shell-v1-cd/) | Command history (`readline`), directory navigation (`chdir`), signal trapping (`SIGINT`) |
| **03** | [`03-mini-shell-v2-redirections/`](03-mini-shell-v2-redirections/) | I/O stream duplication (`dup2`), low-level file manipulation (`open`) |
| **04** | [`04-mini-shell-v3-pipes/`](04-mini-shell-v3-pipes/) | Inter-process communication (`pipe`), process branching (`fork`) |
| **05** | [`05-client-server-basic/`](05-client-server-basic/) | Low-level IPC abstractions (`socket`, `bind`, `accept`) |
| **06** | [`06-tcp-client/`](06-tcp-client/) | Network client handshakes (`connect`), CLI argument processing (`getopt`) |
| **07** | [`07-tcp-server/`](07-tcp-server/) | Concurrent network daemons (`listen`, `accept`, `fork`) |
| **08** | [`08-tcp-client-server/`](08-tcp-client-server/) | Integrated full-stack Client + Server architecture |
| **09** | [`09-select-pipes/`](09-select-pipes/) | Multi-client broadcast communication hub |
| **10** | [`10-file-server/`](10-file-server/) | Remote network file system utilities |
| **11** | [`11-file-locking-fcntl/`](11-file-locking-fcntl/) | Synchronous data locking barriers (`fcntl`, `F_SETLKW`) |
| **12** | [`12-directory-scanning/`](12-directory-scanning/) | VFS navigation and directory traversing (`opendir`, `readdir`) |
| **13** | [`13-config-parser/`](13-config-parser/) | Alphanumeric layout parsing mechanics |
| **14** | [`14-daemon-process/`](14-daemon-process/) | Background process decoupling (`fork`, `setsid`) |
| **15** | [`15-inotify-monitor/`](15-inotify-monitor/) | Real-time filesystem event tracking (`inotify`) |
| **16** | [`16-gdbm-database/`](16-gdbm-database/) | Key-value data persistence layers (`gdbm`) |
| **17** | [`17-hash-table-basic/`](17-hash-table-basic/) | Low-level static Hash Table implementations |
| **18** | [`18-dynamic-linking-dlopen/`](18-dynamic-linking-dlopen/) | Runtime shared library linking architectures (`dlopen`, `dlsym`) |
| **19** | [`19-pty-terminal/`](19-pty-terminal/) | Virtual sub-terminal control interfaces (Pseudo-terminals) |
| **20** | [`20-signal-handling-advanced/`](20-signal-handling-advanced/) | Real-time signals (`sigaction`, `SA_SIGINFO`, `sigqueue`) |
| **21** | [`21-performance-measurement/`](21-performance-measurement/) | Resource tracking and profile diagnostics (`getrusage`) |
| **22** | [`22-https-server/`](22-https-server/) | Minimalist compliant HTTP networking layer |
| **23** | [`23-threads-basic/`](23-threads-basic/) | Parallel baseline processing layouts (`pthread_create`, `pthread_join`) |
| **24** | [`24-thread-mutex/`](24-thread-mutex/) | Shared state race shielding barriers (`pthread_mutex`) |
| **25** | [`25-thread-condition-variables/`](25-thread-condition-variables/) | Condition event dispatch networks (`pthread_cond`) |

---

## Prerequisites
* **OS:** Linux (or Windows Subsystem for Linux - WSL)
* **Compiler/Automation:** GCC, Make utility
* **External Core Libraries:** Depending on the project directory, you may require:
  * `libreadline-dev` (for interactive mini-shells)
  * `libgdbm-dev` (for structured storage engines)

---

## Compilation

Each project environment compiles fully independently of the rest of the workspace. Navigate into your target project directory and invoke the automated compiler rules:

```bash
cd 01-mini-shell-v0
make
```

To wipe previous object structures and binary traces inside any folder, clean the workspace using:
```bash
make clean
```

---

## Repository Hygiene (`.gitignore`)

A global `.gitignore` file should be placed at the root of the repository to prevent untracked object configurations, dynamically linked binary files, or temporal runtime storage records from polluting your revision history.

### Recommended Root `.gitignore` Block
```gitignore
# Object files and structural artifacts
*.o
*.a
*.so

# Compiled project executables
01-mini-shell-v0/mini_shell_v0
02-mini-shell-v1-cd/mini_shell
03-mini-shell-v2-redirections/mini_shell_redir
04-mini-shell-v3-pipes/mini_shell_pipe
06-tcp-client/tcp_client
07-tcp-server/tcp_server
08-tcp-client-server/tcp_client
08-tcp-client-server/tcp_server
16-gdbm-database/gdbm_demo
18-dynamic-linking-dlopen/dynload
21-performance-measurement/getrusage_demo
22-https-server/http_server
23-threads-basic/parallel_sum
24-thread-mutex/mutex_threads
25-thread-condition-variables/prod_cons

# Runtime testing data and localized dumps
*.gdbm
verrou.txt
test.gdbm
c_files.txt

# Editor profiles / Operating System files
*.swp
*~
.DS_Store
```
*Golden Rule: Never push compiled binaries or `.o` object allocations to your code repository. The configuration rules above enforce this abstraction automatically.*

---

## Credits
* **Author:** Ludovic130
* **Reference:** Built alongside the standard examples, exercises, and system paradigms structured in *Programmation système sous Linux* by Christophe Blaess.
