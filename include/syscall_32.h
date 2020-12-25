#ifndef SYSCALL_32_GUARD
#define SYSCALL_32_GUARD

#include <stdint.h>


ssize_t syscall32(uint64_t sysno, uint8_t count, ...);

ssize_t write32(int fd, const void* buf, size_t count);

ssize_t read32(int fd, void* buf, size_t count);

#endif
