# Config Parser

## Description
This project implements a simple configuration file parser in the form of key=value pairs. It uses a linked list to store configurations in memory, providing functions to load, modify, save, and display the configuration.

## Features
- Reads a configuration file (`config.txt`) in `key=value` format
- In-memory storage via a linked list (`config_t`)
- Adds/modifies key=value pairs using `set_struct()`
- Saves the configuration back to the file using `set_conffile()`
- Displays the entire configuration using `load()`
- Frees memory using `fr()`
- Handles allocation errors and file opening failures

## Configuration File Format

```
clé=valeur
autre_clé=autre_valeur
```

Example (`config.txt`):
```
name=pawpaw
firstname=marc
game=mario
fun=crazy
test=Ok
```

## Compilation

```bash
make
```

Or manually:
```bash
gcc -Wall -Wextra -Werror -o Config Config.c
```

## Execution

```bash
./Config
```

The program:
1. Loads the configuration from `config.txt`
2. Adds/modifies multiple entries via `set_struct()`
3. Saves the updated configuration to `config.txt`
4. Displays the full configuration
5. Frees allocated memory

## Main API
- `load_conf()` - Loads the `config.txt` file into memory
- `set_struct(key, val)` - Adds or updates a key/value pair
- `set_conffile()` - Writes the in-memory configuration back to `config.txt`
- `load()` - Displays all key=value pairs
- `fr()` - Frees all memory allocated for the linked list

## File Structure
- `Config.c` - Main program with the parser implementation
- `lib.h` - Common header with standard includes
- `config.txt` - Example configuration file
- `Makefile` - Compilation automation

## Cleanup
```bash
make clean    # Removes object files
make fclean   # Removes the executable and object files
make re       # Forces a complete recompilation
```

## Encountered / Potential Issues

### 1. File Parsing & Robustness
- **Handling of Spaces:** If a line contains spaces around the `=` character (e.g., `key = value`), the parser might capture spaces within the key or value names unless explicitly trimmed.
- **Empty Lines and Comments:** The program can crash or corrupt data if it encounters blank lines, trailing spaces, or commented lines (e.g., lines starting with `#`) unless they are explicitly skipped during `load_conf()`.
- **Missing `=` Separator:** If a line does not contain an `=` symbol, `strchr` or parsing tokens could return `NULL`, leading to a segmentation fault if unchecked.

### 2. Memory Management (Linked List)
- **Memory Leaks on Duplicates:** When `set_struct()` updates an existing key, the old value string must be properly freed before assigning the new one; otherwise, a memory leak occurs.
- **String Duplication:** Keys and values read from temporary line buffers must be cloned using `strdup()`. If they point directly to a transient buffer inside `load_conf()`, all nodes will point to corrupted or identical stack memory.
- **Incomplete Freeing (`fr`):** When purging the list, the program must free both the structural node (`config_t*`) AND the dynamically allocated string pointers (`key` and `val`) within each node.

### 3. Buffer Overflows & String Limits
- **Line Length Constraints:** If `fgets` uses a fixed-size buffer to read lines from `config.txt`, exceptionally long values will be split into multiple lines. This breaks the protocol and risks corrupting subsequent pairs.
- **Unbounded Operations:** Functions like `sprintf` or `strcpy` used during file output or assignment risk smashing the stack if keys or values exceed expected lengths. `snprintf` should be used instead.

### 4. Concurrent Access & Data Loss
- **Destructive Overwriting:** `set_conffile()` rewrites the entire file from scratch. If the program crashes midway through writing, or if two instances write simultaneously, the entire `config.txt` file can be completely emptied or corrupted.
- **File Access Rights:** If `config.txt` lacks write permissions or is locked by another process, `set_conffile()` will fail silently unless its return values are rigorously checked.

## Auteur

**Ludovic130**

## Références

**Le livre de christphe blaess sur la programmation système en C sous linux**