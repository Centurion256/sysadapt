#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <syscall.h>
#include <unistd.h>

#include "local_unistd_32.h"
#include "syscall_32.h"

ssize_t syscall32(uint64_t sysno, uint8_t count, ...)
{
    va_list args;
    va_start(args, count);
    ssize_t retval = 0;
    //not very nice, but seems like there's no better way to do this...
    switch (count)
    {
    case 0:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno)
        : "memory");
        break;
    case 1:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno), "b" (va_arg(args, uint64_t))
        : "memory");
        break;
    case 2:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno), "b" (va_arg(args, uint64_t)), "c" (va_arg(args, uint64_t))
        : "memory");
        break;
    case 3:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno), "b" (va_arg(args, uint64_t)), "c" (va_arg(args, uint64_t)), "d" (va_arg(args, uint64_t))
        : "memory");
        break;
    case 4:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno), "b" (va_arg(args, uint64_t)), "c" (va_arg(args, uint64_t)), "d" (va_arg(args, uint64_t)), "S" (va_arg(args, uint64_t))
        : "memory");
        break;
    case 5:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno), "b" (va_arg(args, uint64_t)), "c" (va_arg(args, uint64_t)), "d" (va_arg(args, uint64_t)), "S" (va_arg(args, uint64_t)), "D" (va_arg(args, uint64_t))
        : "memory");
        break;
    case 6:
        asm volatile ("int $0x80"
        : "=a" (retval)
        : "a" (sysno), "b" (va_arg(args, uint64_t)), "c" (va_arg(args, uint64_t)), "d" (va_arg(args, uint64_t)), "S" (va_arg(args, uint64_t)), "D" (va_arg(args, uint64_t)), "R" (va_arg(args, uint64_t))
        : "memory");
        break;
    default:
        return -1;
        break;
    }
    return retval;
}

ssize_t write32(int fd, const void* buf, size_t count)
{
    return syscall32(__NR32_write, 3, fd, buf, count);
}

ssize_t read32(int fd, void* buf, size_t count)
{
    return syscall32(__NR32_read, 3, fd, buf, count);
}