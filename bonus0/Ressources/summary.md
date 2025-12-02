
# PART 2: Vulnerability Summary

## What is EIP?

**EIP** (Extended Instruction Pointer) is a CPU register that contains the address of the next instruction to execute.

When a function returns:
1. The `ret` instruction loads the **Return Address** from the stack into EIP
2. CPU jumps to that address and continues execution

**In exploitation:**
- We overwrite the Return Address on the stack
- When `ret` executes, EIP gets our malicious address
- Program jumps to our shellcode

## The Vulnerability

The program uses two vulnerable functions:

### 1. strncpy without null terminator
```c
strncpy(dest, src, 20);  // Copies 20 bytes, no \0 if src is >= 20
```

If we fill all 20 bytes, there's no null terminator at the end.

### 2. strcat without size check
```c
strcat(dest, src);  // Concatenates without checking size
```

Since first buffer has no `\0`, `strcat` treats both buffers as one continuous string and overflows.

## Memory Layout

```
[First buffer: 20 bytes][Second buffer: 20 bytes]
                       ↑ No \0 here!
```

When `strcat` runs, it doesn't know where first buffer ends, allowing overflow into the second buffer and beyond.

## Why Shellcode in Environment?

The buffers are too small (20 bytes each) to hold shellcode (50+ bytes).

**Solution:** Store shellcode in an environment variable where there's plenty of space.

## What is a NOP Sled?

**NOP** = No Operation (`\x90` in x86)

A NOP sled is a sequence of NOP instructions before the shellcode:

```
[\x90][\x90][\x90]...[\x90][shellcode]
  ↑                         ↑
  NOPs do nothing      Actual code
```

### Why use a NOP sled?

**Problem:** The exact address of shellcode changes slightly due to:
- Environment variables
- Program path length  
- Stack alignment

**Solution:** Put 500 NOPs before shellcode. Now we don't need the EXACT address:

```
Memory:
0xbffff6db: [NOP][NOP][NOP]...[NOP][shellcode]
            ↑              ↑         ↑
        Start point    We can jump  Ends here
                       ANYWHERE in
                       this range!
```

If we jump to ANY address in the NOP sled:
1. CPU executes NOPs (does nothing)
2. Slides down through all NOPs
3. Eventually reaches shellcode
4. Executes shellcode → Shell!

**Example:**
```
Shellcode at: 0xbffff86b (unknown exact address)
NOP sled: 0xbffff6db to 0xbffff86b (500 bytes)
We jump to: 0xbffff750 (middle of NOPs)
Result: Slides through NOPs → Hits shellcode → Success!
```

## The Payload Structure

```
[4095 B's] + [\n] + [9 A's] + [address] + [30 B's]
```

| Part | Bytes | Purpose |
|------|-------|---------|
| `"B"*4095` | 4095 | Fill first read() buffer (4096 - 1 for \n) |
| `\n` | 1 | Separator between two inputs |
| `"A"*9` | 9 | Padding to reach EIP |
| `\x50\xf7\xff\xbf` | 4 | Address in NOP sled (0xbffff750) |
| `"B"*30` | 30 | Extra padding for stability |

### Why 4095?

The `read()` call reads up to 4096 bytes. We use 4095 to leave room for `\n` which separates the two inputs.

### Why fill with B's?

To ensure `strncpy` copies exactly 20 bytes with NO null terminator, forcing `strcat` to overflow.

## Execution Flow

1. First `read()`: 4095 B's fill the buffer (no `\0` at position 20)
2. `strncpy`: Copies 20 B's to first buffer (no `\0`)
3. Second `read()`: Reads 9 A's + address + 30 B's
4. `strncpy`: Copies 20 bytes to second buffer
5. `strcpy`: Copies first buffer (20 B's, no `\0`!)
6. `strcat`: Tries to append second buffer
   - Doesn't find `\0` in first buffer
   - Treats everything as one string
   - Overflows and overwrites EIP
7. Function returns → EIP loaded with our address
8. Jumps to NOP sled → Slides to shellcode
9. Shellcode executes `/bin/sh`
10. **Shell as bonus1!**