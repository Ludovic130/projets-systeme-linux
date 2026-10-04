# Anagram Project – Encountered Issues

This project uses FIFO pipes to enable communication between a client and a server.
The objective is to scramble the letters of a string (anagram) via a server.

## Compilation

```bash
make

# Or

gcc -D_GNU_SOURCE -Wall -Wextra -o serveur serveur.c
gcc -D_GNU_SOURCE -Wall -Wextra -o client client.c
```

### Execution

```bash
# Step 1: Launch the server 
./serveur

# Step 2: Launch the client
./client

# Step 3: In the client's input field, type
bonjour
```

## Issues Related to FIFO Pipes

### 1. Blocking on Open
- `open(fifo, O_RDONLY)` blocks until a writer has opened the pipe.
- `open(fifo, O_WRONLY)` blocks until a reader has opened the pipe.
- Consequence: if the server is not running, the client remains blocked on `open("anagrammeLuc.fifo", O_WRONLY)`.
- Solution: use `O_NONBLOCK` and handle `ENXIO` to detect the absence of a reader/writer.

### 2. Pipe Ends and EOF
- When all writers close a FIFO, the reader receives `EOF` (`fgets` returns `NULL`).
- The server then closes its pipe and reopens it to continue accepting new clients.
- Warning: if a client writes partially and then disappears, the server can remain blocked.

### 3. FIFO Creation
- `mkfifo` fails with `EEXIST` if the file already exists.
- The server exits immediately in this case.
- It should `unlink` beforehand, or handle `EEXIST`.
- FIFOs are not automatically deleted if the program crashes.

### 4. Potential Deadlock
- The server opens the client's FIFO in write mode (`O_WRONLY`), which blocks until the client opens it in read mode.
- If the client fails to do so (crash, omission), the server remains blocked indefinitely.

### 5. Concurrency
- Multiple clients can write to the server FIFO.
- Here, the client's write is atomic because `fprintf` sends less than `PIPE_BUF` (4096 bytes) in a single operation.
- However, the server reads two successive lines; if another client interjects, the protocol breaks.

## Issues Related to `fgets`

### 1. Fixed Buffer Size
- `fgets` reads at most `n-1` characters or up to `\n`.
- If the line exceeds 127 characters, the remainder stays in the pipe and will be read by the next `fgets`.
- This shifts the entire protocol (the client FIFO name might be truncated, etc.).

### 2. Handling of `\n`
- `fgets` retains the `\n` if there is enough space.
- The code removes it manually by checking the last character.
- If the line is too long, there is no `\n` and the removal has no effect (but the code checks the last character, so there is no corruption).

### 3. Unchecked Return Values
- In the client, if `fgets(stdin)` returns `NULL`, `chaine` is not initialized but the program continues.
- In the server, the second `fgets(chaine, ...)` is not checked: if the client only sends one line, the server blocks.

### 4. No Support for Binary Strings
- `fgets` stops at `\n` and does not handle `\0`. This is not a blocker here since we are manipulating text.

## Other Bugs / Limitations

- In `repondre`, the condition `if ((fd = open(...)) >= 0)` followed by `if (fd < 0)` is inconsistent: the second `if` will never execute. Therefore, errors when opening the client FIFO are not handled.
- `strfry` is a GNU extension, not standard. `strdup` is POSIX but not standard C.
- No `SIGPIPE` handling if the client disappears during a write operation.
- `sprintf` is used without buffer overflow checking.
- The client FIFO name uses the PID; a collision is possible if the PID is reused and the file was not deleted.
- The server does not clean up the FIFO if it is abruptly interrupted.

## Areas for Improvement

- Use `O_NONBLOCK` and `poll`/`select` to avoid blocking.
- Replace `fgets` with a read operation using `read` and a protocol based on size or a robust delimiter.
- Systematically check the return values of `open`, `mkfifo`, `fgets`, and `fdopen`.
- Handle `EEXIST` during FIFO creation.
- Use unique FIFO names (for example, with `mkstemp` or a UUID).
- Add timeouts and proper signal handling.

## Auteur

**Ludovic130**

## Références

**Le livre de christphe blaess sur la programmation système en C sous linux**
