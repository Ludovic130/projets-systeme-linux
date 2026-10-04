# File Locking with `fcntl`

## Description
This program demonstrates how to use the `fcntl()` system call to place an **advisory write lock** on a specific byte range of a file. 

It handles the complete lifecycle of a file lock: creating a file, writing a message, locking a designated portion, reading the contents, unlocking the range, and cleanly closing the file. This is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Prerequisites
To compile and run this project, you need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **Make**

---

## Compilation

To generate the executable named `verrou`, run the following command in your terminal:

```bash
make
```

---

## Usage

Execute the compiled binary from your terminal:

```bash
./verrou
```

### Expected Behavior
The program automatically:
1. Creates a file named `verrou.txt`.
2. Writes a payload message to it.
3. Places a write lock on bytes 12–15.
4. Reads the file back and prints its contents to the screen.

### Example Output
```bash
$ ./verrou
The lock is on file descriptor: 3
The lock is on file descriptor: 3 which contains Ludovic est verrouiller.
```

---

## Problems Encountered & Solutions

Here is a breakdown of common technical pitfalls encountered when working with file descriptor locking in C, along with their solutions:

### 1. String not null-terminated after `read()`
* **Problem:** The buffer `t[25]` is filled with 24 bytes from `read()`, but no null terminator (`\0`) is appended. Calling `printf("%s", t)` reads past the buffer limits, leading to undefined behavior (garbage characters or a segmentation fault).
* **Solution:** Allocate one extra byte for the null terminator and set it manually right after reading:
  ```c
  char t[26];
  ssize_t n = read(fd, t, 24);
  if (n >= 0) t[n] = '\0';
  ```

### 2. Hardcoded lock range calculation
* **Problem:** `lock.l_start = 25 / 2;` evaluates to byte 12, and `lock.l_len = 4` successfully locks bytes 12–15. This works for this specific test case, but hardcoding values makes the code extremely fragile if the file content changes.
* **Solution:** Compute string lengths dynamically using `strlen()`, or use well-defined offsets to adapt the boundaries to variable string sizes.

### 3. Misunderstanding Advisory Locking
* **Problem:** `fcntl` locks are advisory by default. They do not physically block processes from reading or writing if those separate processes ignore the locking API entirely. They only prevent concurrent modification if *all* interacting applications cooperatively check the locks.
* **Solution:** Design application workflows under the assumption that this is cooperative behavior. For mandatory operating system locks, specific runtime mount configurations or alternative flags (like `flock` with `LOCK_MAND`) are required, though they sacrifice portability.

### 4. Blocking (`F_SETLKW`) vs. Non-blocking (`F_SETLK`)
* **Problem:** `F_SETLKW` forces the process to sleep and block until the lock can be safely acquired. If another concurrent process hangs or misbehaves while holding the lock, your program will block indefinitely.
* **Solution:** For non-blocking applications, swap out the command for `F_SETLK`. This returns instantly with a failure flag if the lock is busy, allowing you to check for `EACCES` or `EAGAIN` errors and handle them gracefully.

### 5. Missing error checks on `write()`
* **Problem:** The return value of the `write()` system call is ignored. If the disk fills up or the file descriptor runs into an unhandled I/O exception, the program moves forward silently.
* **Solution:** Always validate that the return value matches the expected bytes written, and handle any mismatch using `perror()`.

### 6. Overly broad file permissions
* **Problem:** Creating the file using a standard `0644` mask may be too permissive if the file handles sensitive system or session data.
* **Solution:** Tighten down security parameters based on your threat model (e.g., use `0600` so only the owner has read/write privileges).

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on file locking.
