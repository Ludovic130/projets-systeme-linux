# Directory Scanning

## Description
This project implements a directory traversal utility similar to `ls -l` but with detailed information for each entry. It combines the `opendir()`, `readdir()`, and `closedir()` APIs with `stat()` to display the file type, permissions, and size of every entry in a directory.

## Features
- Recursive or iterative traversal of one or multiple directories passed as arguments
- Display of file types: block device, character device, directory, FIFO/pipe, symbolic link, regular file, or socket
- Detailed permissions breakdown (User, Group, Others: r/w/x)
- Display of file size in bytes
- Comprehensive error handling using `perror()`
- Support for multiple directory arguments (defaults to the current directory if none are provided)

## Configuration / File Formats
The program reads standard Unix/Linux filesystem structures via system headers like `<dirent.h>` and `<sys/stat.h>`.

Example output format:

```
. :
Type : directory
Permissions : u:rwx g:r-x o:r-x
Taille : 4096 octets
Nom :
Makefile
lib.h
opendir_stat
opendir_stat.c
README.md
```

## Compilation

```bash
make
```

Or manually:
```bash
gcc -Wall -Wextra -Werror -o opendir_stat opendir_stat.c
```

## Execution

```bash
# Scan the current directory
./opendir_stat

# Scan a specific directory
./opendir_stat /path/to/directory

# Scan multiple directories
./opendir_stat /tmp /home /var/log
```

## Main API
- `opendir()` - Opens a directory stream matching the provided path string.
- `readdir()` - Iterates through the directory stream to read individual file entries (`struct dirent`).
- `stat()` / `lstat()` - Retrieves metadata (size, type, permissions) for a given file path.
- `closedir()` - Closes the open directory stream and frees associated resources.

## File Structure
- `opendir_stat.c` - Main program containing the directory traversal logic
- `lib.h` - Common header file with standard includes (including `<dirent.h>` and `<sys/stat.h>`)
- `Makefile` - Automation rules for compilation

## Cleanup
```bash
make clean    # Removes object files
make fclean   # Removes the executable and object files
make re       # Forces a complete recompilation
```

## Encountered / Potential Issues

### 1. File Path Resolution Bugs
- **Relative Path Pitfall:** `readdir()` only provides the file name (e.g., `file.txt`), not its absolute path. If you pass this name directly to `stat()` while scanning an external directory, `stat()` will look for it in your *current* working directory and fail.
- **Solution:** Construct the full path dynamically using `snprintf(full_path, sizeof(full_path), "%s/%s", dir_name, entry->d_name)` before passing it to `stat()`.

### 2. Infinite Recursion Loops
- **The `.` and `..` Trap:** Every directory contains the virtual entries `.` (current directory) and `..` (parent directory). If a recursive function scans these without explicitly skipping them, it will trigger an infinite loop, eventually resulting in a stack overflow.
- **Solution:** Explicitly add a conditional check like `if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;`.

### 3. Broken Symbolic Links & `stat()` vs `lstat()`
- **Dangling Links:** If you use `stat()`, the operating system automatically follows symbolic links to read the target file's metadata. If that target file does not exist, `stat()` will return `-1` (failure).
- **Solution:** Use `lstat()` instead of `stat()`. `lstat()` reads the metadata of the symbolic link itself rather than following it to its target destination.

### 4. Memory & Resource Exhaustion
- **File Descriptor Leaks:** If your program encounters a sub-directory it cannot access due to permission restrictions, or if an error causes a function to exit prematurely without calling `closedir()`, you will leak file descriptors. The system can quickly hit its maximum descriptor limit (`RLIMIT_NOFILE`), causing future folder lookups to fail completely.

### 5. Buffer Overflows on Deep Folder Structures
- **Fixed-Size Paths:** If you combine paths inside a fixed-size character array (e.g., `char path[1024]`), scanning deep directory hierarchies will eventually cause data truncation or buffer overflows. Use `PATH_MAX` from `<limits.h>` as a baseline, or allocate your string buffers dynamically.

## Auteur

**Ludovic130**

## Références

**Le livre de christphe blaess sur la programmation système en C sous linux**