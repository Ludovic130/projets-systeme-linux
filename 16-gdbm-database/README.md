# GDBM Demo – Key/Value Database

## Description
This program demonstrates the use of **GDBM** (GNU Database Manager), a simple key/value database library.  

It creates a database file named `test.gdbm`, inserts a few records (storing the name and age of family members), reopens it in read-only mode, and iterates over all keys to display their values. It is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **GDBM development library** (install via `sudo apt install libgdbm-dev` on Debian/Ubuntu)
* **Make**

---

## Compilation

To generate the `gdbm_demo` executable, run the following command in your terminal:

```bash
make
```

---

## Usage

Run the compiled binary from your terminal by passing the target database file name as an argument:

```bash
./gdbm_demo test.gdbm
```

### Expected Behavior
The program first calls `create_gdbm()`, which creates/overwrites `test.gdbm` with sample data. It then reopens the database in read-only mode and prints all key/value pairs found.

### Example Output
```text
Name-grandsoeur : Diana
Age-grandsoeur : 23
Name-frère : David
Age-frère : 19
```

---

## Implementation Details
* **`create_gdbm()`:** Opens the database with the `GDBM_NEWDB` flag (which creates a new database, overwriting any existing file) and stores four records using `gdbm_store()`.
* **`main()`:** Reopens the database with `GDBM_READER`, iterates through the items using `gdbm_firstkey()` / `gdbm_nextkey()`, and fetches each corresponding value using `gdbm_fetch()`.

---

## Problems Encountered & Solutions

Here is a list of common errors encountered when working with GDBM and how to resolve them:

### 1. `datum.dptr` is not a null-terminated string
* **Problem:** A `datum` in GDBM is structured as `{char *dptr; int dsize;}`. The `dptr` field points to a raw byte buffer of length `dsize` and is not guaranteed to be null-terminated. Printing it directly using `%s` can read past the buffer, printing garbage or triggering a segmentation fault.
* **Solution:** Either copy the data into a buffer of size `dsize + 1` and manually append `\0`:
  ```c
  char *key = malloc(cle.dsize + 1);
  memcpy(key, cle.dptr, cle.dsize);
  key[cle.dsize] = '\0';
  printf("%s\n", key);
  free(key);
  ```
  Or use `fwrite()` to print the exact number of bytes directly to standard output:
  ```c
  fwrite(cle.dptr, 1, cle.dsize, stdout);
  ```

### 2. Wrong `dsize` calculation for accented characters
* **Problem:** Storing `"Name-frère"` with a hardcoded length like `cle.dsize = 9` fails because, in UTF-8 encoding, the accented character `è` occupies 2 bytes. The actual byte length is 10. This mismatch truncates the key or includes trailing corruption.
* **Solution:** Never hardcode sizes for multi-byte or non-ASCII text strings. Always use `strlen()` to compute the exact byte length dynamically:
  ```c
  cle.dptr = "Name-frère";
  cle.dsize = strlen(cle.dptr);
  ```

### 3. Memory leaks inside the iteration loop
* **Problem:** Both `gdbm_firstkey()` and `gdbm_nextkey()` dynamically allocate memory for the key buffers they return. Overwriting the `cle` structure on each iteration step without calling `free()` causes a memory leak.
* **Solution:** Cache the next key entry into a temporary variable, and free the current key buffer at the end of the loop iteration:
  ```c
  datum next;
  for (cle = gdbm_firstkey(base); cle.dptr != NULL; cle = next) {
      next = gdbm_nextkey(base, cle);
      donnee = gdbm_fetch(base, cle);
      if (donnee.dptr != NULL) {
          fwrite(cle.dptr, 1, cle.dsize, stdout);
          printf(" : ");
          fwrite(donnee.dptr, 1, donnee.dsize, stdout);
          printf("\n");
          free(donnee.dptr);
      }
      free(cle.dptr);
  }
  ```

### 4. Linker error: "undefined reference to gdbm_open"
* **Problem:** The program was compiled without explicitly linking the GDBM library dependency.
* **Solution:** Add `-lgdbm` to your linker flags in your project `Makefile`. On certain specific system environments, `-lgdbm_compat` might also be needed.

### 5. `gdbm_open()` fails: "No such file or directory"
* **Problem:** When opening a file with the `GDBM_READER` flag, the database must already exist. If `create_gdbm()` was skipped or failed to write the file, `gdbm_open()` returns `NULL`.
* **Solution:** Always validate the return value of `gdbm_open()` and parse the error code using `gdbm_strerror(gdbm_errno)` to print a human-readable message.

### 6. `GDBM_NEWDB` flag overwrites the data file on every run
* **Problem:** `create_gdbm()` uses `GDBM_NEWDB`, which completely clears out and recreates the database from scratch on every run, destroying data persistence.
* **Solution:** To preserve and append data to an existing file, switch the flag to `GDBM_WRCREAT` (write/create mode):
  ```c
  gdbm_open("test.gdbm", 512, GDBM_WRCREAT, 0644, NULL);
  ```

### 7. `gdbm_store()` errors are not checked
* **Problem:** Ignoring the return status of `gdbm_store()` hides write failures caused by full storage disks or mismatched access permissions.
* **Solution:** Track the return status explicitly and log failure codes:
  ```c
  if (gdbm_store(base, cle, donnee, GDBM_REPLACE) != 0) {
      fprintf(stderr, "gdbm_store error: %s\n", gdbm_strerror(gdbm_errno));
  }
  ```

### 8. Suboptimal file block size parameters
* **Problem:** Passing an explicit parameter like `512` to `gdbm_open()` sets the internal file system block size. Choosing an incorrect static block size manually can lead to performance degradation on larger databases.
* **Solution:** While `512` is fine for small test applications, pass `0` to instruct GDBM to automatically select the operating system's default optimal block size.

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on GDBM.
