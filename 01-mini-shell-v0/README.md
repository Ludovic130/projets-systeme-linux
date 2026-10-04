# Mini-Shell V0

## Description
This program implements the foundational version of an interactive **Unix mini-shell**. 

It provides a command-line interface that reads user input, tokenizes strings into isolated argument arrays, and executes the commands in a dedicated child process space using the `fork()` and `execvp()` system architecture. This is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **Interactive Command Loop:** Continuously prompts for and processes user terminal inputs.
* **Dynamic Tokenization:** Parses complex input strings into execution-safe argument vectors (`argv`).
* **Process Isolation:** Spawns distinct execution environments using `fork()`.
* **Execution Path Resolution:** Leverages `execvp()` to automatically locate and execute binaries via the system `PATH`.
* **Synchronized Termination:** Uses `wait()` to force the parent shell to block until the child task completes.

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **Make**

---

## Compilation

To generate the `mini_shell_v0` executable binary, run the following command in your terminal:

```bash
make
```

---

## Usage

Launch the compiled mini-shell directly from your terminal environment:
```bash
./mini_shell_v0
```

### Execution Example
```bash
root@mini-shell:~$ ls -l
total 8
-rw-r--r-- 1 user user 1234 date fichier.c
...
root@mini-shell:~$ 
```

---

## Code Structure
The implementation follows a classic five-stage execution cycle:
1. **Command Reading:** Captures raw input lines from standard input via `fgets()`.
2. **Tokenization:** Breaks down string arrays into isolated tokens utilizing `strtok()`.
3. **Process Spawning:** Forks a child process while incorporating defensive `EAGAIN` fault handling.
4. **Command Execution:** Overlays the child process context with the target program image via `execvp()`.
5. **Process Synchronization:** Blocks the parent shell thread via `wait()` to await the child process return code.

---

## Problems Encountered & Solutions

### The `execvp()` "Bad Address" Error
During early development phases, calls to `execvp()` unexpectedly broke with an EFAULT error (**"Bad Address"**).

* **The Cause:** 
  Splitting input strings into tokens with `strtok()` modifies the base string by injecting trailing null terminators (`\0`). While the `argv` pointer table mapped these correctly, the underlying string captured via `fgets()` retained its trailing newline character (`\n`). Upon tokenization, this trailing artifact generated malformed, empty, or improperly bounded text sequences. 
  
  Additionally, passing an un-terminated execution array to `execvp()` caused the kernel to read out of bounds. The execution vector must strictly terminate with an explicit `NULL` pointer, and the system needed rigorous string delimiter handling (spaces, tabs, newlines) to guarantee memory boundary safety.

* **The Solution:**
  * Configure `strtok()` to use a comprehensive delimiter string filter (`" \t\n"`) to strip spaces, tabs, and newline elements simultaneously.
  * Explicitly enforce that the final element slot of your populated `argv` array is assigned to `NULL`.
  * Protect the system against segmentation faults by validating that `argv[0]` is not `NULL` (empty command) before sending data blocks to `execvp()`.
  * Add conditional constraints to bypass processing workflows entirely if the user submits empty text lines.

*The active codebase successfully incorporates all four architectural patches.*

---

## Author
* **Ludovic130**

## References
* **Christophe Blaess**, *Programmation système sous Linux* – Exercise on the mini-shell.
* **Linux Manual Pages:** `fork(2)`, `execvp(3)`, `strtok(3)`, `wait(2)`.
