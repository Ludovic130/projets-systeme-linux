# Mini-Shell with Redirections (Project 3)

## Description
This program implements an advanced **V2 interactive mini-shell** in C. 

Building upon the history and directory features of the previous version, this iteration introduces native handling for standard I/O streams, allowing users to leverage **file descriptors redirection** (`>`, `>>`, and `<`) directly from the command line. This is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **Standard Input Redirection (`<`):** Routes file data streams directly into the standard input (`stdin`) of a command.
* **Standard Output Redirection (`>`):** Diverts standard output (`stdout`) to a file, completely overwriting existing contents.
* **Append Output Redirection (`>>`):** Redirects standard output (`stdout`) to a target file, seamlessly appending contents to the end of the file.
* **Core Mini-Shell Engine:** Retains all features from V1, including GNU `readline` arrow navigation, history persistence, active directory workspace prompts, built-in `cd` / `exit` statements, and parent thread `SIGINT` signal masking.

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **GNU Readline development library** (install via `sudo apt install libreadline-dev` on Debian/Ubuntu systems)
* **Make**

---

## Compilation

To generate the `mini_shell_redir` executable binary, run the following command in your terminal workspace:

```bash
make
```

---

## Usage

Launch the compiled shell environment from your standard console:
```bash
./mini_shell_redir
```

### Redirection Examples
```bash
# Redirect command output to create or overwrite a text listing
Ludovic:~ /home/user> ls -l > listing.txt

# Stream data from the file back into a command utility
Ludovic:~ /home/user> cat < listing.txt

# Append fresh string segments to the end of the file
Ludovic:~ /home/user> echo "new line" >> listing.txt
```

---

## Code Structure
The implementation splits string preprocessing and system execution operations into isolated modular blocks:
* **`cut()`:** Splits raw user input data into unique tokens to populate the target `argv` string vector array.
* **`red()`:** Traverses `argv` looking for redirection operators (`>`, `<`, `>>`), updates I/O tracking tables via `dup2()`, and truncates the string sequence at the operator node so downstream execution engines ignore the file targets.
* **`cmd()`:** Spawns a child process frame via `fork()`, invokes `red()` inside the child scope, and passes clean vectors to `execvp()`.
* **`chdr()`:** Controls internal directory changes and handles default fallback arguments.
* **`choose()`:** Acts as an execution router, directing parsing arrays to `cd`, `exit`, or default `cmd()` operations.

---

## Problems Encountered & Solutions

Here is an architectural breakdown of edge cases, system leaks, and parsing errors identified during development, alongside their resolutions:

### 1. Segmentation faults caused by trailing redirection symbols
* **Problem:** If a user inputted an incomplete command structure like `ls >` with no trailing destination file, the processing loop `red()` attempted to read the out-of-bounds array element `argv[i+1]`, resulting in core dumps or an `EFAULT` (Bad address) error.
* **Solution:** Introduced a defensive length safety assertion (`i+1 < c`) before inspecting elements. If the tracking engine detects a missing filename target, it outputs a clean, meaningful error log and aborts the task gracefully.

### 2. Execution failures due to passing redirection arguments to `execvp()`
* **Problem:** Originally, the raw `argv` array retained the operators (`>`, `<`, `>>`) and their accompanying filenames. Passing this un-parsed array to `execvp()` caused the kernel to look for files or arguments that broke the executable.
* **Solution:** Inside the `red()` utility, right after initializing the file descriptor mapping logic, the code overrides the operator slot by setting `argv[i] = NULL`. This cleanly breaks the vector array, ensuring `execvp()` only intercepts the executable binary and its valid programmatic flags.

### 3. File descriptor resource exhaustion leaks
* **Problem:** If a `dup2()` mapping action threw an execution exception, error handlers aborted processes without shutting down open stream contexts, causing system tracking leaks.
* **Solution:** Added structured exit cleanups, guaranteeing that `close(f)` explicitly handles open file descriptors before error boundaries exit.

### 4. Shared state pollution via parent shell redirection
* **Problem:** The redirection logic initially executed across the main parent thread workspace, causing redirection settings to pollute subsequent inputs and completely break the shell's own native interactive I/O streams.
* **Solution:** Isolated the `red()` utility execution loop, forcing it to run exclusively within the localized scope of the child process right after the `fork()` boundary triggers.

### 5. Compiler warnings and implicit signatures for `open` and `dup2`
* **Problem:** Compiling code modules without explicitly importing `<fcntl.h>` and `<unistd.h>` caused the compiler to map `open()` blindly onto fallback 32-bit `int` declarations, spawning silent memory corruption issues or unstable runtime quirks.
* **Solution:** Appended missing POSIX and system environment definitions directly into the global project header file `lib.h`.

### 6. Core dumps caused by running `strcmp` checks on `NULL` spaces
* **Problem:** Pressing `Enter` on an unpopulated input line yields an empty parsing sequence where `argv[0]` evaluates to `NULL`. Sending this pointer directly into `strcmp(argv[0], "cd")` triggered an automatic segmentation fault.
* **Solution:** Implemented clean validation boundaries to verify pointer validity before passing string structures to tracking arguments:
  ```c
  if (argv[0] != NULL) {
      // Execute matching logic safely
  }
  ```

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on redirections.
