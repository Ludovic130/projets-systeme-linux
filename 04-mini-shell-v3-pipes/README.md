# Mini-Shell with Pipes and Redirections (Project 4)

## Description
This program implements an advanced **V3 interactive mini-shell** in C. 

This major iteration scales up the I/O capabilities of the custom shell engine by introducing full **pipeline support** (`|`). It enables multiple distinct commands to be chained together sequentially, allowing the output of one process to feed into the input of the next, all while seamlessly integrating standard stream redirections (`>`, `>>`, and `<`). This is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **Pipeline Chaining (`|`):** Multi-command pipe sequences allow fluent inter-process communication (IPC) via `pipe()`.
* **Isolated Compound Redirections:** Leverages independent stream overrides (`>`, `>>`, `<`) safely bounded inside specific pipeline stages.
* **Core Mini-Shell Engine:** Inherits all previous system capabilities including GNU `readline` terminal features, dynamic shell prompt paths, native `cd` / `exit` environments, and defensive asynchronous signal masks (`SIGINT`).

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **GNU Readline development library** (install via `sudo apt install libreadline-dev` on Debian/Ubuntu systems)
* **Make**

---

## Compilation

To generate the `mini_shell_pipe` executable binary, execute the following command in your terminal workspace:

```bash
make
```

---

## Usage

Launch the compiled shell environment from your standard console:
```bash
./mini_shell_pipe
```

### Examples of Advanced Pipelines & Redirections
```bash
# A simple multi-stage pipeline counting matching source files
Ludovic:~ /home/user> ls -l | grep ".c" | wc -l

# Piping execution results down to a file redirection target
Ludovic:~ /home/user> ls -l | grep ".c" > c_files.txt

# Blending input file streams, linear pipes, and final output creation
Ludovic:~ /home/user> sort < unsorted.txt | uniq > sorted_uniq.txt
```

---

## Code Structure
The shell engine parses, splits, and manages multi-process lifecycles across a coordinated structure:
* **`cut()`:** Splits raw user input data into isolated argument token fragments.
* **`apply_redir_to_cmd()`:** Targets a single sub-command list segment to configure explicit file descriptor rewrites.
* **`exec_pfils()`:** Spawns pipes, forks child processes, establishes inter-stage communication, and executes command redirections.
* **`exec_pipe()`:** Inspects arguments for the pipe symbol (`|`), counts the required sub-commands, and bootstraps `exec_pfils()`.
* **`red()`:** Fallback handler that resolves standard single-command redirections when no pipeline is present.
* **`cmd()`:** Evaluates inputs; pathways containing `|` route to `exec_pipe()`, while standard workloads fork a single child process.
* **`chdr()` & `choose()`:** Manage internal shell movements (`cd`), exit statements (`exit`), and operational distribution.

---

## Problems Encountered & Solutions

Here is an architectural breakdown of edge cases, descriptor traps, and process synchronization fixes implemented to stabilize the engine:

### 1. Segmentation faults when mixing pipelines and redirections
* **Problem:** Executing compound instructions like `ls -l | grep ".c" > c_files.txt` crashed because the redirection parser modified the argument list *before* the token engine could divide the pipe boundaries. This caused string clipping and shell faults.
* **Solution:** Pipelines are now split first into isolated command array components. Redirections are then handled independently for each discrete subcommand using `apply_redir_to_cmd()`, restricting file updates to their proper execution step.

### 2. Truncated or empty output files following pipeline redirects
* **Problem:** The statement `ls -l | grep ".c" > c_files.txt` yielded an empty text file because the global redirection logic was bound to the top-level pipeline loop rather than the terminal command execution node.
* **Solution:** Shifted the `apply_redir_to_cmd()` execution block inside the child process scope so that modifications trigger right before `execvp()`, ensuring data lands in the correct file descriptor.

### 3. Hanging processes caused by file descriptor resource leaks
* **Problem:** Leaving unused pipe descriptors open inside child environments prevented automated EOF (End-of-File) markers from propagating. This caused tracking commands to hang indefinitely waiting for input streams that never closed.
* **Solution:** Enforced strict pipe closure loops (`close(tubes[j][0])` and `close(tubes[j][1])`) across every child instance right before passing argument arrays down to `execvp()`.

### 4. Premature parent exit generating orphaned child processes
* **Problem:** The parent shell loop completed execution early and returned to the prompt before background child pipelines finished writing, causing truncated terminal strings and orphaned tasks.
* **Solution:** Integrated an explicit tracking cleanup loop within the parent process that invokes `wait(NULL)` continuously until all active child threads have fully exited.

### 5. Out-of-bounds array reads on dangling operator tails
* **Problem:** Incomplete instructions like `ls >` forced `red()` to read past allocation margins while looking for a file target at `argv[i+1]`, causing segmentation faults or `EFAULT` (Bad address) states.
* **Solution:** Appended explicit array size boundary validation checks (`i+1 < c`). Missing parameters are now intercepted gracefully, showing a clear warning message instead of crashing.

### 6. Passing operator metadata symbols directly to `execvp()`
* **Problem:** Retaining control symbols (`>`, `<`, `>>`) within the argument array fed to `execvp()` caused the kernel to error out while attempting to parse them as execution flags.
* **Solution:** Truncate the pointer sequence cleanly by overwriting the operator array index position with `NULL` (`argv[i] = NULL`) during preprocessing.

### 7. Core dumps on working directory update failures
* **Problem:** If an environment tracking tool (`getcwd()`) failed, the `readline` layer was bypassed, causing the system to read dangling pointer addresses from the previous iteration.
* **Solution:** Reset pointer addresses consistently by declaring `line = NULL;` at the beginning of the program and re-initializing it at each loop iteration.

### 8. `strcmp` operations breaking on unpopulated inputs
* **Problem:** Pressing `Enter` on a blank line populates a `NULL` slot at index `argv[0]`. Passing this unvalidated pointer into `strcmp(argv[0], "cd")` caused immediate segmentation faults.
* **Solution:** Enforce simple structural verification guards (`argv[0] != NULL`) before triggering string matching algorithms.

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on pipes and redirections.
