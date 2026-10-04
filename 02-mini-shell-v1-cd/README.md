# Mini-Shell with `cd` and History

## Description
This program implements an advanced **V1 interactive mini-shell** in C. 

Building upon the foundational features of V0, this iteration incorporates modern shell conveniences including dynamic terminal prompt drawing, a persistent execution command history lookup cache, custom built-in path routing mechanics, and basic kernel asynchronous signal masking. This is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **Enhanced Prompt Interface:** Integrates the GNU `readline` library to support command history cache persistence and native arrow-key navigation.
* **Dynamic Working Directory Context:** Computes and redraws the current active system path context directly into the terminal prompt.
* **Built-in Shell Directives:** Handles internal execution state changes directly inside the parent thread environment:
  * `cd [directory]` : Updates the working directory (defaults to `/root` if no path argument is provided).
  * `exit` : Safely terminates the running shell instance.
* **Defensive Signal Isolation:** Masks and ignores asynchronous interruption events (`SIGINT` / `Ctrl+C`) within the parent shell thread while keeping default execution traps intact for child tasks.

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **GNU Readline development library** (install via `sudo apt install libreadline-dev` on Debian/Ubuntu systems)
* **Make**

---

## Compilation

To generate the `mini_shell` executable binary, run the following command in your terminal workspace:

```bash
make
```

---

## Usage

Launch the compiled shell environment from your standard console:
```bash
./mini_shell
```

### Built-In Reference Matrix
* **`cd [directory]`**: Triggers structural workspace movements. Providing no target argument defaults directory routing straight to `/root`.
* **`exit`**: Instantly breaks execution hooks and terminates the active process loop.

### Execution Session Example
```bash
Ludovic:~ /home/user> ls -l
...
Ludovic:~ /home/user> cd Documents
Ludovic:~ /home/user/Documents> pwd
/home/user/Documents
Ludovic:~ /home/user/Documents> exit
```

---

## Problems Encountered & Solutions

Here is an architectural breakdown of edge cases, system truncations, and memory faults identified during development, alongside their resolutions:

### 1. Segmentation faults caused by processing empty string inputs
* **Problem:** If a user submits an empty line by pressing `Enter` with no text payload, the internal parsing token utility (`cut()`) evaluates `argv[0]` directly to `NULL`. Passing that unvalidated `NULL` pointer straight to evaluations like `strcmp(argv[0], "exit")` triggers a core dump.
* **Solution:** Wrap string validation filters within clean structural layout assertions to intercept unpopulated vectors safely:
  ```c
  if (argv[0] != NULL && strcmp(argv[0], "cd") == 0) { ... }
  else if (argv[0] != NULL && strcmp(argv[0], "exit") == 0) { ... }
  else if (argv[0] != NULL) { cmd(argv); }
  ```

### 2. Missing prototype signatures for `readline` and `add_history`
* **Problem:** Omitting `<readline/readline.h>` and `<readline/history.h>` includes forces the C compiler to drop implicit function mapping definitions, assuming it yields standard 32-bit `int` registers rather than explicit `char *` references. On 64-bit target hardware architectures, this truncates high bits, rendering addresses hazardous to dereference.
* **Solution:** Explicitly append the structural dependency headers inside your core `lib.h` build manifest.

### 3. Linker reference failures during compilation
* **Problem:** Build stages fail with an `undefined reference to readline` tracking error flag.
* **Solution:** Inject the required runtime linker flags (`-lreadline`) into your project `Makefile`. *Note: On certain system dependencies, appending companion terminal caps like `-lncurses` or `-ltermcap` may also be required.*

### 4. Broken fallback parameters for standard `cd` invocations
* **Problem:** Executing a bare `cd` directive with no secondary argument input failed to update target system paths, keeping the user context stuck in place.
* **Solution:** Implement a safe bounds validation check. If `argv[1] == NULL`, fallback explicitly to system administrative baselines:
  ```c
  if (chdir("/root") == 0) { ... }
  ```

### 5. `SIGINT` signals breaking the parent application frame
* **Problem:** Striking `Ctrl+C` dispatched an unmitigated interruption signal that killed the parent mini-shell process rather than cleanly stopping the active child application thread.
* **Solution:** Register a signal isolation layout configuration using `signal(SIGINT, SIG_IGN)` within the parent shell module. This leaves child tracking rings untouched so commands can still absorb default interrupts seamlessly.

### 6. Dangling parameters from uninitialized input line pointers
* **Problem:** If an internal directory fetch lookup (`getcwd()`) failed, the `readline()` call sequence skipped entirely, causing the raw `line` variable to hold onto older, recently cleared heap addresses and generating corruption hazards.
* **Solution:** Enforce variable initialization rules by setting `line = NULL;` both at the initial program initialization frame and at the loop recycling phase.

### 7. Stack corruption via buffer overflows within `cut()`
* **Problem:** If an incoming user instruction exceeded 19 distinct argument spaces, the token mapping loop wrote memory data blindly past the allocated margins of `argv[20]`, corrupting the thread stack frame.
* **Solution:** Restrict loop bounds tightly (`pos < 19`) to keep a defensive final slot reserved strictly for the terminal sequence `NULL` element pointer.

### 8. Orphaned data vectors following `exit(0)` instructions
* **Problem:** Triggering an immediate `exit(0)` branch upon parsing the close string statement bypasses subsequent cleanup scripts like `free(line)`.
* **Solution:** While technical code linters highlight this, it does not constitute a critical leak here. The operating system kernel automatically drops and reclaims the full process memory workspace upon termination.

### 9. String truncation via small prompt buffers
* **Problem:** Working within extended nested directory path levels causes deep path characters to match or exceed static prompt array constraints, causing `snprintf` to crop tracking metrics out of view.
* **Solution:** Scale internal prompt character tracking footprints to a safer upper limit (`prompt[1024]`) and always assess return flags.

### 10. Memory clipping caused by mismatched string boundaries
* **Problem:** Using a restricted boundary size allocation (`rep[300]`) for initial `getcwd` actions while running `1024` byte calculations inside localized helper blocks introduces system tracking inconsistencies and quiet string data clipping.
* **Solution:** Normalize and lock down systemic file limits uniformly across the codebase by leveraging the explicit `PATH_MAX` macro macro constant provided by `<limits.h>`.

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercise on the mini-shell.
