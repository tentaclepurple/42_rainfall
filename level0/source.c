#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
    int numero;
    char *args[2];
    
    // Convert argument to integer
    numero = atoi(argv[1]);
    
    // Compare with 423 (0x1a7 in hexadecimal)
    if (numero == 423) {
        // If correct, execute a shell with level1 privileges
        args[0] = strdup("/bin/sh");
        args[1] = NULL;
        
        // Change to level1's UID and GID
        gid_t gid = getegid();
        uid_t uid = geteuid();
        
        setresgid(gid, gid, gid);
        setresuid(uid, uid, uid);

        // Execute shell
        execv("/bin/sh", args);
    } else {
        // If incorrect, print "No !"
        fwrite("No !", 1, 5, stderr);
    }
    
    return 0;
}