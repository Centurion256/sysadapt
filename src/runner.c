#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <syscall.h>
#include <err.h>
#include <errno.h>
#include <stdarg.h>

#include "syscall_32.h"
#include "local_unistd_32.h"

#define SIZE 14

//FOR TESTING PURPOSES ONLY
ssize_t write64(unsigned int fd, const char* buf, size_t count)
{
    uint8_t nr = SYS_write;
    ssize_t retval = 0;
    asm volatile ("syscall"
        : "=a" (retval)
        : "0" (nr), "D" (fd),  "S" (buf), "d" (count)
        : "memory", "rcx", "r11");
    return retval;
}

//Make sure to write Hello, World!\n or a similar 14-character string to stdin. in.txt already exists for your convenience
//Example: in project root dir: ./build/call < in.txt
//With sysadapt: ./build/sysadapt build/call < in.txt
int main(int argc, char const *argv[])
{
 
    char buf[SIZE] = {0}; //local array
    //Other options include:
    // char* buf = malloc(sizeof(char)*14);
    // static char buf[14] = {0};
    if (read(STDIN_FILENO, buf, SIZE) == -1)
    {
        printf("Failed to read\n");
        exit(1);
    }
    //By itself, THIS IS UNDEFINED BEHAVIOUR, since EIP can only access 32-bit address space.
    //However, try calling this under sysadapt, it should work as intended!
    ssize_t retval = write32(STDOUT_FILENO, buf, SIZE);
    if (retval < 0)
    {
        printf("32-bit write failed: %s\n", strerror(-retval));
    }
    else
    {
        printf("32-bit write succeded\n");
    }
    retval = write(STDOUT_FILENO, buf, 14); //or write64(STDOUT_FILENO, buf, 14);
    if (retval < 0)
    {
        perror("64-bit write failed\n");
    }
    else
    {
        printf("64-bit write succeded\n");
    }
    
    exit(0);
}
