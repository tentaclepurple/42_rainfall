
# Vulnerability Summary

## What is Heap Overflow?

Similar to stack overflow, but happens in the **heap** (memory allocated by `malloc`).

## How the program works

```c
char *buf = malloc(64);    // Block 1: 64 bytes for our input
char *func = malloc(4);    // Block 2: 4 bytes for function pointer
*func = &m;                // Stores address of m()
strcpy(buf, argv[1]);      // Copies input (VULNERABLE!)
(*func)();                 // Calls the function
```

## The Vulnerability

`strcpy` copies without checking size. If input > 64 bytes, it overflows into the next block.

## Memory Layout

```
[  Block 1: 64 bytes  ][ metadata: 8 bytes ][  Block 2: 4 bytes  ]
         ↑                                          ↑
    Our input                              Function pointer (m)
```

## The Attack

Write 72 bytes (64 + 8 metadata) to reach Block 2, then overwrite the function pointer.

## The Payload

```
[72 bytes junk] + [address of n]
```

| Part | Purpose |
|------|---------|
| `'a'*72` | Fill Block 1 + metadata |
| `'\x54\x84\x04\x08'` | Address of `n` (0x08048454) |

## Result

1. `strcpy` copies 76 bytes
2. Function pointer overwritten with address of `n`
3. Program calls `(*func)()`
4. Instead of `m`, it calls `n`
5. `n` executes `system("/bin/cat /home/user/level7/.pass")`
6. **Password printed!**