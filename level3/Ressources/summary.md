
# Vulnerability Summary

## What is Format String Vulnerability?

The program uses `printf(buffer)` instead of `printf("%s", buffer)`.

This allows us to use special format codes:

| Code | What it does |
|------|--------------|
| `%x` | Reads values from the stack |
| `%n` | Writes to memory |

## The Problem

The code checks if a global variable equals 64:
```c
if (global_variable == 64) {
    system("/bin/sh");
}
```

The variable is at address `0x804988c` and starts at `0`.

## The Solution

We use `%n` to write to memory.

`%n` writes how many characters have been printed so far.

## The Payload
```
[address] + [60 × 'A'] + [%4$n]
 4 bytes     60 bytes
```

- **Address (4 bytes):** Where to write (`0x804988c`)
- **60 A's:** Padding to reach 64 total characters
- **%4$n:** Write 64 to the address at stack position 4

**Total characters printed:** 4 + 60 = **64**

## Result

1. `printf` prints 64 characters
2. `%4$n` writes 64 to address `0x804988c`
3. Variable now equals 64
4. Program executes `system("/bin/sh")`
5. **Shell obtained!**