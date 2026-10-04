# Hash Table with Separate Chaining

## Description
This program implements a simple **hash table** with **separate chaining** to handle collisions.  

Each bucket is a singly linked list of `person` structures, each holding a text name. It demonstrates the complete core layout of a custom dictionary: a polynomial rolling hash function, efficient insertions, precise lookup routing, and visual reporting.

This is a learning project for system programming under Linux, based on the work of **Christophe Blaess**.

---

## Features
* **Custom Rolling Hash:** Computes target indices using a polynomial hash routine with a standard multiplier (`31`).
* **Dynamic Collision Resolution:** Utilizes a separate chaining layout via structured singly linked lists.
* **O(1) Insert Handling:** Offloads items rapidly by forcing head insertion chains.
* **Traceable Content Iteration:** Full bucket mapping outputs showing linked elements.

---

## Prerequisites
To compile and run this project, you will need:
* **Linux** (or WSL under Windows)
* **GCC** (GNU Compiler Collection)
* **Make**

---

## Compilation

To generate the `hash_table` executable, execute the following command in your terminal:

```bash
make
```

---

## Usage

Execute the compiled binary from your terminal workspace:

```bash
./hash_table
```

### Expected Behavior
The program inserts 9 predefined sample names into the hash table, runs a targeted search verification for one of them (`marcelin`), and prints out the resulting structure mapping.

### Example Output
```text
the name marcelin was found at position 4
	0	----
	1	----
	2	diana	 -> 	max	 -> 	sara	
	3	----
	4	marcelin	 -> 	albert	 -> 	ludovic	
	...
```

---

## Implementation Details
* **`hash()`:** Computes the runtime bucket boundary index utilizing a rolling polynomial multiplier formula bound by the `HASH_SIZE` limit.
* **`init_table()`:** Wipes and resets every base entry cell array reference to `NULL` to prepare for storage allocations.
* **`insert_table()`:** Prefixes fresh incoming data contexts onto the head position layout of the corresponding target chain.
* **`search_table()`:** Steps linearly across chain links evaluating string equivalence with standard `strcmp`.
* **`print_table()`:** Sequentially traverses the top level buckets, iterating linked nodes to build human-readable chain diagrams.

---

## Problems Encountered & Solutions

Here is a comprehensive overview of engineering problems identified during implementation and how they are addressed:

### 1. Segmentation faults caused by premature iteration
* **Problem:** Running table traversals before running a baseline initialization routine leaves the main array populated with arbitrary garbage data pointers. Downstream lookups try dereferencing invalid zones and crash instantly.
* **Solution:** Explicitly invoke `init_table()` at the entry point of your execution flow to systematically map every bucket link to `NULL`.

### 2. Header implicit declarations for `strnlen()`
* **Problem:** The `strnlen()` routine belongs to the POSIX extension matrix rather than vanilla standard ISO C. Specific build environments drop implicit declaration alerts or fail entirely if compilation anchors are missing.
* **Solution:** Enforce `#define _GNU_SOURCE` (or `_POSIX_C_SOURCE`) configurations at the topmost region of your `lib.h` source module before linking `<string.h>`.

### 3. Signed vs. unsigned data mismatching in the hash loop
* **Problem:** Declaring an internal tracker using a signed signed variable (`int hash_val`) while targeting an unsigned return definition (`unsigned int`) is fragile and can spark silent casting exceptions or unexpected platform behavior.
* **Solution:** Standardize data storage definitions inside your computation code blocks cleanly:
  ```c
  unsigned int hash_val = 0;
  ```

### 4. Ambiguous phrasing on lookup failures
* **Problem:** Falling out of loop search bounds prints generic tracking targets via reference variables, raising interface ambiguity during failure events.
* **Solution:** Intentionally structured the diagnostic logic to output the original search criterion value, explicitly clarifying what search item failed to index.

### 5. Missing trailing newlines on fallback notices
* **Problem:** Omiting a line break (`\n`) within the lookup warning function forces subsequent printing blocks (like `print_table()`) to run on the same line, dirtying interface layouts.
* **Solution:** Append explicit trailing escape indicators to prevent message blending:
  ```c
  printf("The name %s was not found\n", p->name);
  ```

### 6. Uninitialized forward link pointers (`next`)
* **Problem:** Local assignments like `person ludovic = {.name = "ludovic"};` leave the trailing node link pointer variable undefined when instantiated on the stack, which introduces security risks if dereferenced prematurely.
* **Solution:** While runtime assignments overwrite this area safely during insertion updates anyway, initializing structure attributes safely at birth prevents hazardous leaks:
  ```c
  person ludovic = {.name = "ludovic", .next = NULL};
  ```

### 7. Non-uniform bucket distribution clusters
* **Problem:** Constraining a classic rolling hash configuration onto a narrow bucket domain (`HASH_SIZE = 10`) guarantees frequent collision overlaps when dealing with shorter strings.
* **Solution:** Expand macro sizes or deploy large prime constraints (like 101 or 1009) to guarantee uniform distribution and smooth lookup performance curves.

### 8. Stack allocation lifecycles
* **Problem:** Initial allocations operate directly on the current function thread stack frame. Transitioning storage allocation modules to heap setups via `malloc()` without companion tracking structures will drop orphaned data traces.
* **Solution:** Maintain current stack scoping models for basic scripts. If refactoring to dynamic runtime heaps, build companion garbage collection sequences to safely walk and free every node chain.

### 9. Index congestion on empty string definitions
* **Problem:** Passing an empty literal block (`""`) causes `strnlen` to evaluate to `0`. Consequently, every empty insertion piles onto index `0` and degrades efficiency.
* **Solution:** Enforce filter validation safeguards inside your ingestion functions to filter out empty strings before calculation routines trigger.

### 10. Weak macro typing limits
* **Problem:** Relying purely on raw macros (`#define HASH_SIZE 10`) offers zero type safety checking mechanics from your standard compilation checks.
* **Solution:** Leverage typed structure expressions or numerical enumeration types to force system compiler validation:
  ```c
  enum { HASH_SIZE = 10 };
  ```

---

## Author
* **Ludovic130**

## Reference
* **Christophe Blaess**, *Programmation système sous Linux* – Exercises on hash tables.
