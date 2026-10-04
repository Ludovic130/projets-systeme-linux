# Dynamic Library Loading (`dlopen` / `dlsym`)

## Description
This program demonstrates how to load a shared library at runtime using `dlopen()`, retrieve a function address with `dlsym()`, execute it, and then unload the library with `dlclose()`.  

It serves as a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **Make**
* **`libdl`** (usually included with `glibc`; the `-ldl` flag may be required on some systems)

---

## Compilation

To generate the `dynload` executable, simply run the following command in your terminal:

```bash
make
```

> **Note:** If the source file for the shared library (`libmathN.c`) is present, the `Makefile` will automatically compile the `libmathN.so` file as well.

---

## Usage

Ensure that the `libmathN.so` file is located in the **same directory** as the executable, then run:

```bash
./dynload
```

### Example Output
The program loads the library, searches for the `saluer` function, executes it, and then cleanly unloads it from memory.
```bash
$ ./dynload
Hello from the shared library!
```

---

## Problems Encountered & Solutions

Here is a list of common errors encountered when working with dynamic libraries and how to resolve them:

### 1. `dlopen()` fails: "cannot open shared object file"
* **Cause:** The shared library `libmathN.so` is not in the current directory or its path is incorrect.
* **Solution:** Compile the library and place it in the same folder as the executable, or provide a relative/absolute path in `dlopen()`. Use `ldd` or `ls` to verify its presence.

### 2. `dlsym()` returns `NULL`
* **Cause:** The `saluer` function does not exist in the library, or the library was not compiled with the correct symbol visibility.
* **Solution:** Verify that the library exports the symbol (e.g., run `nm -D libmathN.so`). Ensure the function is not declared as `static`.

### 3. Linker error: "undefined reference to dlopen"
* **Cause:** The program was not linked with the system `dl` library.
* **Solution:** Add `-ldl` to your linker flags (already configured in the `Makefile`). On some systems, installing `libc6-dev` or `libdl-dev` is required.

### 4. Segmentation fault when calling the function pointer
* **Cause:** The function pointer is `NULL` because `dlsym()` failed.
* **Solution:** Always clear previous errors with `dlerror()` before calling `dlsym()`, then test the pointer immediately after. *The current codebase already implements this robust validation step.*

### 5. Shared library not compiled with `-fPIC`
* **Cause:** Compiling without the `-fPIC` flag can prevent the library from loading or cause runtime crashes.
* **Solution:** Always generate your shared libraries using Position Independent Code (`-fPIC`):
  ```bash
  gcc -shared -fPIC -o libmathN.so libmathN.c
  ```

### 6. `dlclose()` fails or causes a crash
* **Cause:** The library is still being used, or residual pointers are still referencing its functions.
* **Solution:** Do not call any functions from the library after executing `dlclose()`. Remember to also check the return value of `dlclose()`.

### 7. Error messages are not displayed correctly
* **Cause:** `dlerror()` returns a string that gets overwritten by subsequent system calls.
* **Solution:** Store the output of `dlerror()` in a local variable immediately after a failure to preserve it for later use.

### 8. Library not found at runtime (even though it is present)
* **Cause:** The dynamic linker does not look inside the current directory by default.
* **Solution:** Explicitly use `./libmathN.so` in your source code, or configure your system environment variables:
  ```bash
  export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
  ```

### 9. "undefined symbol" during loading
* **Cause:** The shared library depends on other sub-libraries that are not loaded or are missing from the system.
* **Solution:** Inspect dependencies using `ldd libmathN.so` to identify the missing components.

### 10. "Permission denied" during loading
* **Cause:** The library file does not have the necessary read or execute permissions.
* **Solution:** Fix file access permissions using `chmod +r libmathN.so` (and `+x` if necessary).

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on dynamic libraries.
