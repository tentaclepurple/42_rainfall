
# Vulnerability Summary

## How the program works

```c
// Allocate memory
block1 = malloc(8);
block2 = malloc(8);    // Stores pointer for argv[1]
block3 = malloc(8);
block4 = malloc(8);    // Stores pointer for argv[2]

// Read password file
file = fopen("/home/user/level8/.pass");
fgets(global_var, 68, file);

// Copy arguments
strcpy(block2_ptr, argv[1]);  // VULNERABLE
strcpy(block4_ptr, argv[2]);

puts("~~");  // Prints ~~ instead of password
```

## The Problem

- Password is read into `0x8049960`
- Function `m` prints `0x8049960` but is never called
- `puts` only prints `~~`

## The Vulnerability

`strcpy` doesn't check size. First `strcpy` can overflow and overwrite the pointer used by second `strcpy`.

## Memory Layout

```
[Block2: 8][metadata: 8][Block3: 8][metadata][Block4 pointer]
                                              ↑
                                    Overflow reaches here
```

**Offset:** 20 bytes to reach Block4 pointer

## The Attack

1. **argv[1]:** 20 bytes junk + GOT address of `puts`
2. **argv[2]:** Address of function `m`

## The Payload

**Argument 1:**
```
"A"*20 + "\x28\x99\x04\x08"
         └─ GOT of puts
```

**Argument 2:**
```
"\xf4\x84\x04\x08"
 └─ Address of m
```

## Result

1. First `strcpy` overwrites Block4 pointer with GOT of `puts`
2. Second `strcpy` writes address of `m` into GOT of `puts`
3. Program calls `puts()`
4. GOT redirects to `m`
5. `m` prints the password
6. **Password obtained!**