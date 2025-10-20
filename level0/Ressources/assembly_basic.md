
---

## Assembly Instructions Reference

### Instructions Used in This Level

| Assembly | Description | C Equivalent | Example |
|----------|-------------|--------------|---------|
| **`mov A, B`** | Copy value from A to B | `B = A;` | `mov %eax, %ebx` → ebx = eax |
| **`add A, B`** | Add A to B, store in B | `B += A;` | `add $4, %eax` → eax = eax + 4 |
| **`cmp A, B`** | Compare A with B (sets flags) | (prepares if condition) | `cmp $423, %eax` → compare eax with 423 |
| **`jne ADDR`** | Jump if Not Equal | `if (A != B) goto ADDR;` | `jne 0x8048f58` → jump if not equal |
| **`call FUNC`** | Call function | `FUNC();` | `call atoi` → execute atoi() |

### Complete x86 Instruction Set Reference

#### Data Movement
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `mov A, B` | Copy A to B | `B = A;` | Most common instruction |
| `lea A, B` | Load Effective Address | `B = &A;` | Gets memory address, not value |
| `push A` | Push A onto stack | (implicit) | Saves value for later |
| `pop B` | Pop from stack to B | (implicit) | Retrieves saved value |
| `xchg A, B` | Exchange A and B | `temp=A; A=B; B=temp;` | Swaps values |

#### Arithmetic Operations
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `add A, B` | Add A to B | `B += A;` | B = B + A |
| `sub A, B` | Subtract A from B | `B -= A;` | B = B - A |
| `inc A` | Increment A | `A++;` | A = A + 1 |
| `dec A` | Decrement A | `A--;` | A = A - 1 |
| `imul A, B` | Multiply (signed) | `B *= A;` | B = B * A |
| `idiv A` | Divide (signed) | `result = eax / A;` | Result in eax, remainder in edx |
| `neg A` | Negate A | `A = -A;` | Two's complement |

#### Logical Operations
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `and A, B` | Bitwise AND | `B &= A;` | B = B & A |
| `or A, B` | Bitwise OR | `B \|= A;` | B = B \| A |
| `xor A, B` | Bitwise XOR | `B ^= A;` | B = B ^ A |
| `not A` | Bitwise NOT | `A = ~A;` | Inverts all bits |
| `shl A, B` | Shift Left | `B <<= A;` | Multiply by 2^A |
| `shr A, B` | Shift Right | `B >>= A;` | Divide by 2^A |

#### Comparison and Testing
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `cmp A, B` | Compare A with B | (prepares condition) | Sets flags, doesn't store result |
| `test A, B` | Bitwise test (AND without storing) | `if (A & B)` | Often used to check if zero |

#### Unconditional Jumps
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `jmp ADDR` | Jump to address | `goto ADDR;` | Always jumps |
| `call FUNC` | Call function | `FUNC();` | Pushes return address, then jumps |
| `ret` | Return from function | `return;` | Pops return address and jumps |

#### Conditional Jumps (after `cmp` or `test`)
| Assembly | Description | C Equivalent | When to use |
|----------|-------------|--------------|-------------|
| `je ADDR` | Jump if Equal | `if (A == B) goto ADDR;` | After cmp: values are equal |
| `jne ADDR` | Jump if Not Equal | `if (A != B) goto ADDR;` | After cmp: values are different |
| `jg ADDR` | Jump if Greater | `if (A > B) goto ADDR;` | Signed comparison |
| `jge ADDR` | Jump if Greater or Equal | `if (A >= B) goto ADDR;` | Signed comparison |
| `jl ADDR` | Jump if Less | `if (A < B) goto ADDR;` | Signed comparison |
| `jle ADDR` | Jump if Less or Equal | `if (A <= B) goto ADDR;` | Signed comparison |
| `ja ADDR` | Jump if Above | `if (A > B) goto ADDR;` | Unsigned comparison |
| `jae ADDR` | Jump if Above or Equal | `if (A >= B) goto ADDR;` | Unsigned comparison |
| `jb ADDR` | Jump if Below | `if (A < B) goto ADDR;` | Unsigned comparison |
| `jbe ADDR` | Jump if Below or Equal | `if (A <= B) goto ADDR;` | Unsigned comparison |
| `jz ADDR` | Jump if Zero | `if (result == 0) goto ADDR;` | Same as je |
| `jnz ADDR` | Jump if Not Zero | `if (result != 0) goto ADDR;` | Same as jne |

#### String Operations
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `movs` | Move string | `strcpy()` | Copies memory blocks |
| `cmps` | Compare strings | `strcmp()` | Compares memory blocks |
| `scas` | Scan string | `strchr()` | Searches in memory |
| `lods` | Load string | Read byte/word | Loads from memory to register |
| `stos` | Store string | Write byte/word | Stores from register to memory |

#### Stack Operations
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `push A` | Push onto stack | (function call setup) | Decrements esp, stores A |
| `pop B` | Pop from stack | (function return cleanup) | Loads from stack, increments esp |
| `leave` | Restore stack frame | (function epilogue) | Equivalent to: mov ebp,esp; pop ebp |
| `enter` | Create stack frame | (function prologue) | Rarely used, push ebp; mov esp,ebp |

#### Special Instructions
| Assembly | Description | C Equivalent | Notes |
|----------|-------------|--------------|-------|
| `nop` | No operation | (empty statement) | Does nothing, used for padding |
| `int 0x80` | System call (Linux) | `syscall()` | Kernel interrupt |
| `syscall` | System call (x64) | `syscall()` | Modern 64-bit version |

### Common Registers (x86 32-bit)

| Register | Purpose | Preserved across calls? |
|----------|---------|------------------------|
| `%eax` | Accumulator, return value | No |
| `%ebx` | Base register | Yes |
| `%ecx` | Counter | No |
| `%edx` | Data register | No |
| `%esi` | Source index | Yes |
| `%edi` | Destination index | Yes |
| `%ebp` | Base pointer (stack frame) | Yes |
| `%esp` | Stack pointer | Yes (by definition) |
| `%eip` | Instruction pointer | (not directly accessible) |

### Addressing Modes

| Syntax | Description | Example | C Equivalent |
|--------|-------------|---------|--------------|
| `$value` | Immediate (constant) | `mov $5, %eax` | `eax = 5;` |
| `%reg` | Register | `mov %eax, %ebx` | `ebx = eax;` |
| `addr` | Direct memory | `mov 0x8048000, %eax` | `eax = *((int*)0x8048000);` |
| `(%reg)` | Indirect | `mov (%eax), %ebx` | `ebx = *eax;` |
| `offset(%reg)` | Indirect with offset | `mov 4(%eax), %ebx` | `ebx = *(eax + 4);` |
| `offset(%base,%index,scale)` | Complex | `mov 8(%ebp,%ecx,4), %eax` | `eax = *(ebp + ecx*4 + 8);` |

---

